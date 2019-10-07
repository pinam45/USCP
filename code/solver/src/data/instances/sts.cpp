//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include "data/instances/sts.hpp"

#include <common/utils/logger.hpp>
#include <common/utils/timer.hpp>

#include <fstream>

bool uscp::problem::sts::read(const std::filesystem::path& path,
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

	// Read subsets covering points
	instance.subsets_points.resize(subsets_number);
	for(size_t i = 0; i < subsets_number; ++i)
	{
		instance.subsets_points[i].resize(points_number);
	}
	for(size_t i_point = 0; i_point < points_number; ++i_point)
	{
		for(size_t i_subset = 0; i_subset < 3; ++i_subset)
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

bool uscp::problem::sts::write(const uscp::problem::instance& instance,
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
