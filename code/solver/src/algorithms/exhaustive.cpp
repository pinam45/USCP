//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include "algorithms/exhaustive.hpp"
#include "utils/permutations.hpp"

#include <common/utils/logger.hpp>
#include <common/utils/timer.hpp>

uscp::solution uscp::exhaustive::solve(const uscp::problem::instance& problem)
{
	SPDLOG_LOGGER_DEBUG(
	  LOGGER,
	  "Started exhaustive search for problem instance with {} points and {} subsets",
	  problem.points_number,
	  problem.subsets_number);
	const timer timer;
	solution current_solution(problem);

	for(size_t bits_on = 0; bits_on <= problem.subsets_number && !current_solution.cover_all_points;
	    ++bits_on)
	{
		SPDLOG_LOGGER_DEBUG(LOGGER, "Started trying permutations of {} bits", bits_on);
		PermutationsGenerator generator(problem.subsets_number, bits_on);
		while(!generator.finished() && !current_solution.cover_all_points)
		{
			current_solution.selected_subsets = generator.next();
			current_solution.compute_cover();
		}
	}

	if(!current_solution.cover_all_points)
	{
		LOGGER->warn(
		  "Exhaustive search enumerated all possible solution and failed to find one covering all points in {}s",
		  timer.elapsed());
	}
	else
	{
		LOGGER->info("Found optimal solution by exhaustive search with {} subsets in {}s",
		             current_solution.selected_subsets.count(),
		             timer.elapsed());
	}
	return current_solution;
}

uscp::solution uscp::exhaustive::solve_ram(const uscp::problem::instance& problem)
{
	(void)problem;
	//TODO
	assert(false);
	return uscp::solution(problem::instance());
}
