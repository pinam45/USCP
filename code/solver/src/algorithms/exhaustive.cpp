//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include "algorithms/exhaustive.hpp"
#include "utils/logger.hpp"
#include "utils/permutations.hpp"

uscp::solution uscp::exhaustive::solve(const uscp::problem::instance& problem)
{
	SPDLOG_LOGGER_DEBUG(
	  LOGGER,
	  "Started exhaustive search for problem instance with {} points and {} subsets",
	  problem.points_number,
	  problem.subsets_number);
	const auto start = std::chrono::system_clock::now();
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

	const auto end = std::chrono::system_clock::now();
	const std::chrono::duration<double> elapsed_seconds = end - start;
	if(!current_solution.cover_all_points)
	{
		LOGGER->warn(
		  "Exhaustive search enumerated all possible solution and failed to find one covering all points in {}s",
		  elapsed_seconds.count());
	}
	else
	{
		LOGGER->info("Found optimal solution by exhaustive search with {} subsets in {}s",
		             current_solution.selected_subsets.count(),
		             elapsed_seconds.count());
	}
	return current_solution;
}

uscp::solution uscp::exhaustive::solve_ram(const uscp::problem::instance& problem)
{
	//TODO
	assert(false);
	return uscp::solution(problem::instance());
}
