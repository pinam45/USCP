//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include "data/instances.hpp"
#include "utils/logger.hpp"

bool uscp::problem::read(const uscp::problem::instance_info& info,
                         uscp::problem::instance& instance) noexcept
{
	instance.name = info.name;
	if(!info.read_function(info.file, instance))
	{
		return false;
	}
	if(info.points != instance.points_number)
	{
		LOGGER->warn(
		  "Instance have invalid points number, instance information: {}, instance read: {}",
		  info,
		  instance);
		return false;
	}
	if(info.subsets != instance.subsets_number)
	{
		LOGGER->warn(
		  "Invalid subsets number, instance information: {}, instance read: {}", info, instance);
		return false;
	}
	return true;
}

bool uscp::problem::check_instances() noexcept
{
	for(const uscp::problem::instance_info& instance_info: uscp::problem::instances)
	{
		uscp::problem::instance instance;
		if(!uscp::problem::read(instance_info, instance))
		{
			LOGGER->warn("Failed to read problem {}", instance_info);
			return false;
		}

		// check if the problem have a solution
		if(!uscp::problem::has_solution(instance))
		{
			LOGGER->warn(
			  "Instance is unsolvable (some elements cannot be covered using provided subsets), instance information: {}, instance read: {}",
			  instance_info,
			  instance);
			return false;
		}
	}
	return true;
}
