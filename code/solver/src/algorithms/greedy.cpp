//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include "algorithms/greedy.hpp"
#include "utils/logger.hpp"
#include "utils/timer.hpp"

uscp::solution uscp::greedy::solve(const uscp::problem::instance& problem) noexcept
{
	const timer timer;

	solution solution(problem);
	while(!solution.cover_all_points)
	{
		size_t max_subset_number = solution.selected_subsets.size(); //invalid initial value
		dynamic_bitset<> covered_points_with_max_subset(problem.points_number);
		size_t covered_points_number_with_max_subset = solution.covered_points.count();
		for(size_t i = 0; i < problem.subsets_number; ++i)
		{
			if(solution.selected_subsets[i])
			{
				// already selected
				continue;
			}

			dynamic_bitset<> new_covered_points =
			  solution.covered_points | problem.subsets_points[i];
			const size_t new_covered_points_number = new_covered_points.count();
			if(new_covered_points_number > covered_points_number_with_max_subset)
			{
				max_subset_number = i;
				covered_points_with_max_subset = new_covered_points;
				covered_points_number_with_max_subset = new_covered_points_number;
			}
		}

		// all subset already included or no subset add covered points
		if(max_subset_number == solution.selected_subsets.size())
		{
			LOGGER->error("The problem has no solution");
			return solution;
		}

		// update solution (faster)
		solution.selected_subsets[max_subset_number].set();
		solution.covered_points = covered_points_with_max_subset;
		solution.cover_all_points = solution.covered_points.all();

		// update solution (slower)
		//solution.compute_cover();
		//solution.compute_cost();
	}

	SPDLOG_LOGGER_DEBUG(LOGGER,
	                    "Found greedy solution to {} with {} subsets in {}s",
	                    problem.name,
	                    solution.selected_subsets.count(),
	                    timer.elapsed());

	return solution;
}
