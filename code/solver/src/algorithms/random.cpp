//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include "solver/algorithms/random.hpp"
#include "common/utils/logger.hpp"
#include "common/utils/timer.hpp"

uscp::solution uscp::random::solve(random_engine& generator,
                                   const problem::instance& problem,
                                   std::shared_ptr<spdlog::logger> logger) noexcept
{
	SPDLOG_LOGGER_DEBUG(logger, "({}) Start building random solution", problem.name);
	const timer timer;
	solution solution(problem);

	assert(problem.subsets_number > 0);
	std::uniform_int_distribution<size_t> dist(0, problem.subsets_number - 1);
	while(!solution.covered_points.all())
	{
		assert(!solution.selected_subsets.all());
		size_t selected_subset = dist(generator);
		while(solution.selected_subsets.test(selected_subset))
		{
			selected_subset = dist(generator);
		}
		solution.selected_subsets.set(selected_subset);
		solution.covered_points |= problem.subsets_points[selected_subset];
	}
	solution.cover_all_points = true;

	SPDLOG_LOGGER_DEBUG(logger,
	                    "({}) Built random solution with {} subsets in {}s",
	                    problem.name,
	                    solution.selected_subsets.count(),
	                    timer.elapsed());
	return solution;
}
