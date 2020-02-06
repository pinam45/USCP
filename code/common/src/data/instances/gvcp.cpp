//
// Copyright (c) 2020 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include "common/data/instances/gvcp.hpp"
#include "common/utils/logger.hpp"
#include "common/utils/timer.hpp"

#include <fstream>
#include <cassert>
#include <deque>
#include <sstream>

namespace
{
	std::optional<std::stringstream> get_content_line(std::ifstream& instance_stream) noexcept
	{
		std::stringstream line_stream;
		char c = static_cast<char>(instance_stream.get());
		while(instance_stream.good())
		{
			switch(c)
			{
				case '\n':
				case '\t':
				case ' ':
					// ignore
					break;
				case '#':
				{
					// ignore line
					do
					{
						c = static_cast<char>(instance_stream.get());
					} while(instance_stream.good() && c != '\n');
					break;
				}
				default:
				{
					// read line
					do
					{
						line_stream << c;
						c = static_cast<char>(instance_stream.get());
					} while(instance_stream.good() && c != '\n');
					return line_stream;
				}
			}
			c = static_cast<char>(instance_stream.get());
		}
		return {};
	}

	bool process_file(const std::filesystem::path& path, uscp::problem::instance& instance)
	{
		std::ifstream instance_stream(path);
		if(!instance_stream)
		{
			SPDLOG_LOGGER_DEBUG(LOGGER, "std::ifstream constructor failed");
			LOGGER->warn("Failed to read file {}", path);
			return false;
		}

		std::optional<std::stringstream> content_line = get_content_line(instance_stream);
		size_t processed_subsets = instance.subsets_number;
		while(content_line)
		{
			std::stringstream& content_line_stream = *content_line;
			int current_subset_local;
			content_line_stream >> current_subset_local;
			if(current_subset_local < 0)
			{
				LOGGER->warn("Invalid subset number: {}", current_subset_local);
				return false;
			}
			size_t current_subset_global =
			  processed_subsets + static_cast<size_t>(current_subset_local);
			size_t current_point = 0;
			while(content_line_stream.good())
			{
				if(current_point >= instance.points_number)
				{
					instance.points_number = current_point + 1;
					for(dynamic_bitset<>& subset_points: instance.subsets_points)
					{
						subset_points.resize(instance.points_number);
					}
				}
				if(current_subset_global >= instance.subsets_number)
				{
					instance.subsets_number = current_subset_global + 1;
					instance.subsets_points.resize(instance.subsets_number,
					                               dynamic_bitset<>(instance.points_number));
				}
				instance.subsets_points[current_subset_global].set(current_point);

				content_line_stream >> current_subset_local;
				if(current_subset_local < 0)
				{
					LOGGER->warn("Invalid subset number: {}", current_subset_local);
					return false;
				}
				current_subset_global =
				  processed_subsets + static_cast<size_t>(current_subset_local);
				++current_point;
			}
			processed_subsets = instance.subsets_number;

			content_line = get_content_line(instance_stream);
		}

		LOGGER->info("Processed file {}", path);
		return true;
	}
} // namespace

bool uscp::problem::gvcp::read(const std::filesystem::path& base_path,
                               uscp::problem::instance& instance_out) noexcept
{
	instance_out.subsets_number = 0;
	instance_out.points_number = 0;
	instance_out.subsets_points.clear();
	const timer timer;

	std::error_code error;
	if(!std::filesystem::exists(base_path, error))
	{
		if(error)
		{
			SPDLOG_LOGGER_DEBUG(LOGGER, "std::filesystem::exists failed: {}", error.message());
			LOGGER->warn("Check if file/folder exist failed for {}", base_path);
		}
		else
		{
			LOGGER->warn("Tried to read problem instance from non-existing file/folder {}",
			             base_path);
		}
		return false;
	}

	uscp::problem::instance instance = instance_out;
	if(std::filesystem::is_regular_file(base_path, error))
	{
		if(!process_file(base_path, instance))
		{
			LOGGER->error("Failed to process file {}", base_path);
			return false;
		}
	}
	else
	{
		if(error)
		{
			SPDLOG_LOGGER_DEBUG(
			  LOGGER, "std::filesystem::is_regular_file failed: {}", error.message());
			LOGGER->warn("Check if path is a regular file failed for: {}", base_path);
			return false;
		}
		if(!std::filesystem::is_directory(base_path, error))
		{
			if(error)
			{
				SPDLOG_LOGGER_DEBUG(
				  LOGGER, "std::filesystem::is_directory failed: {}", error.message());
				LOGGER->warn("Check if path is a regular file failed for: {}", base_path);
				return false;
			}
			LOGGER->warn("Invalid file/folder type: {}", base_path);
			return false;
		}

		LOGGER->info("Started to read instance from {}", base_path);
		std::deque<std::filesystem::path> paths;
		paths.push_back(base_path);
		while(!paths.empty())
		{
			const std::filesystem::path path = paths.front();
			paths.pop_front();

			if(!std::filesystem::exists(path, error))
			{
				if(error)
				{
					SPDLOG_LOGGER_DEBUG(
					  LOGGER, "std::filesystem::exists failed: {}", error.message());
					LOGGER->error("Check if file/folder exist failed for {}", path);
				}
				else
				{
					LOGGER->error("Non-existing file/folder {}", path);
				}
				return false;
			}

			if(std::filesystem::is_directory(path, error))
			{
				std::filesystem::directory_iterator directory_iterator(path, error);
				if(error)
				{
					SPDLOG_LOGGER_DEBUG(
					  LOGGER, "Directory iterator creation failed: {}", error.message());
					LOGGER->error("Failed to get content of folder {}", path);
					continue;
				}
				for(const std::filesystem::directory_entry& entry: directory_iterator)
				{
					paths.push_back(entry.path());
				}
				continue;
			}
			if(error)
			{
				SPDLOG_LOGGER_DEBUG(
				  LOGGER, "std::filesystem::is_directory failed: {}", error.message());
				LOGGER->error("Check if path is a directory failed for {}", path);
			}

			if(std::filesystem::is_regular_file(path, error))
			{
				if(!process_file(path, instance))
				{
					LOGGER->error("Failed to process file {}", path);
					return false;
				}
				continue;
			}
			if(error)
			{
				SPDLOG_LOGGER_DEBUG(
				  LOGGER, "std::filesystem::is_regular_file failed: {}", error.message());
				LOGGER->error("Check if path is a regular file failed for {}", path);
			}
		}
	}

	// Success
	instance_out = std::move(instance);

	LOGGER->info("Successfully read problem instance with {} points and {} subsets in {}s",
	             instance_out.points_number,
	             instance_out.subsets_number,
	             timer.elapsed());
	return true;
}

bool uscp::problem::gvcp::write(const uscp::problem::instance& instance,
                                const std::filesystem::path& path,
                                bool override_file) noexcept
{
	(void)instance;
	(void)path;
	(void)override_file;
	//TODO
	assert(false);
	return false;
}
