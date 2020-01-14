//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include "common/data/instances/orlibrary/rail.hpp"
#include "common/data/instance.hpp"
#include "common/utils/logger.hpp"
#include "common/utils/timer.hpp"

#include <fstream>
#include <cassert>
#include <cstddef>
#include <string>
#include <utility>

bool uscp::problem::orlibrary::rail::read(const std::filesystem::path& path,
                                          uscp::problem::instance& instance_out) noexcept
{
	instance_out.subsets_number = 0;
	instance_out.points_number = 0;
	instance_out.subsets_points.clear();
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

	// Read subsets information
	instance.subsets_points.resize(subsets_number);
	for(size_t i = 0; i < subsets_number; ++i)
	{
		instance.subsets_points[i].resize(points_number);

		// cost
		if(!instance_stream.good())
		{
			LOGGER->warn("Invalid file format");
			return false;
		}
		size_t ignored_subset_cost;
		instance_stream >> ignored_subset_cost;

		// number of points
		if(!instance_stream.good())
		{
			LOGGER->warn("Invalid file format");
			return false;
		}
		size_t subset_points;
		instance_stream >> subset_points;

		for(size_t i_point = 0; i_point < subset_points; ++i_point)
		{
			if(!instance_stream.good())
			{
				LOGGER->warn("Invalid file format");
				return false;
			}
			size_t point_number;
			instance_stream >> point_number;
			if(point_number == 0)
			{
				LOGGER->warn("Invalid value");
				return false;
			}
			--point_number; // numbered from 1 in the file
			if(point_number > points_number)
			{
				LOGGER->warn("Invalid value");
				return false;
			}
			instance.subsets_points[i].set(point_number);
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

bool uscp::problem::orlibrary::rail::write(const uscp::problem::instance& instance,
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
