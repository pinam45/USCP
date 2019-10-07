//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include "common/data/instances/orlibrary/orlibrary.hpp"

#include <common/utils/logger.hpp>
#include <common/utils/timer.hpp>

#include <fstream>

bool uscp::problem::orlibrary::orlibrary::read(const std::filesystem::path& path,
                                               uscp::problem::instance& instance_out) noexcept
{
	const timer timer;

	std::error_code error;
	if(!std::filesystem::exists(path, error))
	{
		if(error)
		{
			SPDLOG_LOGGER_DEBUG(LOGGER, "std::filesystem::exists failed: {}", error.message());
			LOGGER->warn("Check if file/folder exist failed for {}", path);
		}
		else
		{
			LOGGER->warn("Tried to read problem instance from non-existing file/folder {}", path);
		}
		return false;
	}

	if(!std::filesystem::is_regular_file(path, error))
	{
		if(error)
		{
			SPDLOG_LOGGER_DEBUG(
			  LOGGER, "std::filesystem::is_regular_file failed: {}", error.message());
			LOGGER->warn("Check if path is a regular file failed for: {}", path);
		}
		else
		{
			LOGGER->warn("Tried to read problem instance from non-file {}", path);
		}
		return false;
	}

	std::ifstream instance_stream(path);
	if(!instance_stream)
	{
		SPDLOG_LOGGER_DEBUG(LOGGER, "std::ifstream constructor failed");
		LOGGER->warn("Failed to read file {}", path);
		return false;
	}

	LOGGER->info("Started to read problem instance from file {}", path);
	uscp::problem::instance instance = instance_out;

	// Read points number
	size_t points_number = 0;
	if(!instance_stream.good())
	{
		LOGGER->warn("Invalid file format");
		return false;
	}
	instance_stream >> points_number;
	if(points_number == 0)
	{
		LOGGER->warn("Invalid points number: {}", points_number);
		return false;
	}
	instance.points_number = points_number;

	// Read subsets number
	size_t subsets_number = 0;
	if(!instance_stream.good())
	{
		LOGGER->warn("Invalid file format");
		return false;
	}
	instance_stream >> subsets_number;
	if(subsets_number == 0)
	{
		LOGGER->warn("Invalid subsets number: {}", subsets_number);
		return false;
	}
	instance.subsets_number = subsets_number;

	// Read subsets costs
	for(size_t i = 0; i < subsets_number; ++i)
	{
		if(!instance_stream.good())
		{
			LOGGER->warn("Invalid file format");
			return false;
		}
		size_t ignored_subset_cost;
		instance_stream >> ignored_subset_cost;
	}

	// Read subsets covering points
	instance.subsets_points.resize(subsets_number);
	for(size_t i = 0; i < subsets_number; ++i)
	{
		instance.subsets_points[i].resize(points_number);
	}
	for(size_t i_point = 0; i_point < points_number; ++i_point)
	{
		size_t subsets_covering_point = 0;
		if(!instance_stream.good())
		{
			LOGGER->warn("Invalid file format");
			return false;
		}
		instance_stream >> subsets_covering_point;
		if(subsets_covering_point > subsets_number)
		{
			LOGGER->warn("Invalid value");
			return false;
		}
		for(size_t i_subset = 0; i_subset < subsets_covering_point; ++i_subset)
		{
			size_t subset_number = 0;
			if(!instance_stream.good())
			{
				LOGGER->warn("Invalid file format");
				return false;
			}
			instance_stream >> subset_number;
			if(subset_number == 0)
			{
				LOGGER->warn("Invalid value");
				return false;
			}
			--subset_number; // numbered from 1 in the file
			if(subset_number > subsets_number)
			{
				LOGGER->warn("Invalid value");
				return false;
			}
			instance.subsets_points[subset_number][i_point] = true;
		}
	}

	// Success
	instance_out = std::move(instance);

	LOGGER->info("Successfully read problem instance with {} points and {} subsets in {}s",
	             points_number,
	             subsets_number,
	             timer.elapsed());

	return true;
}

bool uscp::problem::orlibrary::orlibrary::write(const uscp::problem::instance& instance,
                                                const std::filesystem::path& path,
                                                bool override_file) noexcept
{
	const timer timer;

	std::error_code error;
	if(std::filesystem::exists(path, error))
	{
		if(error)
		{
			SPDLOG_LOGGER_DEBUG(LOGGER, "std::filesystem::exists failed: {}", error.message());
			LOGGER->warn("Check if file/folder exist failed for {}", path);
		}
		else if(!override_file)
		{
			LOGGER->warn("Tried to write problem instance to already-existing file/folder {}",
			             path);
			return false;
		}
	}

	std::ofstream instance_stream(path, std::ios::out | std::ios::trunc);
	if(!instance_stream)
	{
		SPDLOG_LOGGER_DEBUG(LOGGER, "std::ofstream constructor failed");
		LOGGER->warn("Failed to write file {}", path);
		return false;
	}

	LOGGER->info("Started to write problem instance to file {}", path);

	// Write points number
	instance_stream << " " << instance.points_number;

	// Write subsets number
	instance_stream << " " << instance.subsets_number << " \n ";

	// Write subsets costs
	const size_t return_at = 12;
	size_t out_counter = 0;
	for(size_t i = 0; i < instance.subsets_number; ++i)
	{
		instance_stream << 1 << " "; // unicost
		if(++out_counter == return_at)
		{
			instance_stream << "\n ";
			out_counter = 0;
		}
	}
	instance_stream << "\n ";
	out_counter = 0;

	// Write subsets covering points
	for(size_t i_point = 0; i_point < instance.points_number; ++i_point)
	{
		std::vector<size_t> subsets_covering_point;
		for(size_t i_subset = 0; i_subset < instance.subsets_number; ++i_subset)
		{
			if(instance.subsets_points[i_subset][i_point])
			{
				subsets_covering_point.push_back(i_subset + 1); // numbered from 1 in the file
			}
		}

		instance_stream << subsets_covering_point.size() << " \n ";
		for(size_t subset_number: subsets_covering_point)
		{
			instance_stream << subset_number << " ";
			if(++out_counter == return_at)
			{
				instance_stream << "\n ";
				out_counter = 0;
			}
		}
		if(out_counter != 0)
		{
			instance_stream << "\n ";
			out_counter = 0;
		}
	}

	if(!instance_stream.good())
	{
		LOGGER->warn("Error writing to file");
		return false;
	}

	// Success
	LOGGER->info("successfully written problem instance in {}s", timer.elapsed());

	return true;
}
