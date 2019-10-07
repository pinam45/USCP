//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include "solver/data/solution.hpp"

#include <common/utils/logger.hpp>

#include <vector>

uscp::solution uscp::expand(const uscp::solution& reduced_solution) noexcept
{
	if(!reduced_solution.problem.reduction.has_value())
	{
		LOGGER->error("Tried to expand solution of non-reduced instance");
		return reduced_solution;
	}
	if(reduced_solution.problem.subsets_number
	     + reduced_solution.problem.reduction->reduction_applied.subsets_dominated.count()
	     + reduced_solution.problem.reduction->reduction_applied.subsets_included.count()
	   != reduced_solution.problem.reduction->parent_instance->subsets_number)
	{
		LOGGER->error("Tried to expand solution of an invalid reduced instance");
		return reduced_solution;
	}

	solution expanded_solution(*reduced_solution.problem.reduction->parent_instance);
	size_t i_expanded = 0;
	bool passed;
	do
	{
		passed = false;
		if(reduced_solution.problem.reduction->reduction_applied.subsets_included[i_expanded])
		{
			expanded_solution.selected_subsets.set(i_expanded);
			++i_expanded;
			passed = true;
		}
		else if(reduced_solution.problem.reduction->reduction_applied.subsets_dominated[i_expanded])
		{
			++i_expanded;
			passed = true;
		}
	} while(passed);
	for(size_t i_reduced = 0; i_reduced < reduced_solution.problem.subsets_number; ++i_reduced)
	{
		assert(i_expanded < expanded_solution.problem.subsets_number);
		expanded_solution.selected_subsets[i_expanded] =
		  reduced_solution.selected_subsets[i_reduced];

		do
		{
			++i_expanded;
			passed = false;
			if(reduced_solution.problem.reduction->reduction_applied.subsets_included[i_expanded])
			{
				expanded_solution.selected_subsets.set(i_expanded);
				passed = true;
			}
			else if(reduced_solution.problem.reduction->reduction_applied
			          .subsets_dominated[i_expanded])
			{
				passed = true;
			}
		} while(passed);
	}
	assert(i_expanded == expanded_solution.problem.subsets_number);
	if(i_expanded != expanded_solution.problem.subsets_number)
	{
		LOGGER->error("Solution expansion failed, only {}/{} subsets",
		              i_expanded,
		              expanded_solution.problem.subsets_number);
	}

	expanded_solution.compute_cover();
	return expanded_solution;
}
