//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_SOLUTION_HPP
#define USCP_SOLUTION_HPP

#include "common/data/solution.hpp"
#include "common/utils/logger.hpp"

namespace uscp
{
	[[nodiscard]] solution expand(const solution& reduced_solution) noexcept;

	template<typename T>
	[[nodiscard]] std::vector<T> expand_points_info(const uscp::problem::instance& instance,
	                                                const std::vector<T>& info,
	                                                const T& default_value = {}) noexcept
	{
		if(!instance.reduction.has_value())
		{
			LOGGER->error("Tried to expand points information of non-reduced instance");
			return info;
		}
		if(((instance.subsets_number
		     + instance.reduction->reduction_applied.subsets_dominated.count()
		     + instance.reduction->reduction_applied.subsets_included.count())
		    != instance.reduction->parent_instance->subsets_number)
		   || ((instance.points_number
		        + instance.reduction->reduction_applied.points_covered.count())
		       != instance.reduction->parent_instance->points_number))
		{
			LOGGER->error("Tried to expand solution of an invalid reduced instance");
			abort();
		}
		if(info.size() != instance.points_number)
		{
			LOGGER->error(
			  "Tried to expand point information of size {} of reduced instance with {} points",
			  info.size(),
			  instance.points_number);
			abort();
		}

		std::vector<T> expanded_info;
		expanded_info.resize(instance.reduction->parent_instance->points_number, default_value);
		size_t i_expanded = 0;
		while(instance.reduction->reduction_applied.points_covered.test(i_expanded))
		{
			++i_expanded;
		}
		for(size_t i_reduced = 0; i_reduced < instance.points_number; ++i_reduced)
		{
			assert(i_expanded < instance.reduction->parent_instance->points_number);
			expanded_info[i_expanded] = info[i_reduced];

			do
			{
				++i_expanded;
				if(i_expanded >= instance.reduction->parent_instance->points_number)
				{
					break;
				}
			} while(instance.reduction->reduction_applied.points_covered.test(i_expanded));
		}
		assert(i_expanded == instance.reduction->parent_instance->points_number);
		if(i_expanded != instance.reduction->parent_instance->points_number)
		{
			LOGGER->error("Solution points information expansion failed, only {}/{} points",
			              i_expanded,
			              instance.reduction->parent_instance->points_number);
			abort();
		}

		return expanded_info;
	}
} // namespace uscp

#endif //USCP_SOLUTION_HPP
