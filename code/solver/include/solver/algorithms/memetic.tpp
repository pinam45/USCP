//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_MEMETIC_TPP
#define USCP_MEMETIC_TPP

#ifndef USCP_MEMETIC_HPP
#	error __FILE__ should only be included from solver/algorithms/memetic.hpp.
#endif // USCP_MEMETIC_HPP

#include "common/utils/timer.hpp"
#include "common/utils/logger.hpp"
#include "solver/algorithms/random.hpp"

#include <array>

template<typename Crossover>
uscp::memetic::memetic<Crossover>::memetic(const problem::instance& problem) noexcept
  : m_problem(problem), m_crossover(problem), m_rwls(problem), m_initialized(false)
{
}

template<typename Crossover>
void uscp::memetic::memetic<Crossover>::initialize() noexcept
{
	m_rwls.initialize();
	m_initialized = true;
}

template<typename Crossover>
uscp::memetic::report uscp::memetic::memetic<Crossover>::solve(
  uscp::random_engine& generator,
  const uscp::memetic::config& config) noexcept
{
	if(!m_initialized)
	{
		initialize();
	}

	LOGGER->info("({}) Start solving problem with memetic algorithm", m_problem.name);

	report report(m_problem);
	report.solve_config = config;
	report.crossover_operator = m_crossover.to_string();

	timer timer;
	std::array<solution, 2> population{solution(m_problem), solution(m_problem)};

	for(solution& solution: population)
	{
		solution = uscp::random::solve(generator, m_problem);
	}
	SPDLOG_LOGGER_DEBUG(
	  LOGGER, "({}) Memetic population initialized in {}s", m_problem.name, timer.elapsed());

	size_t generation = 0;
	uscp::rwls::position rwls_cumulative_position;
	rwls_cumulative_position.steps = 0;
	rwls_cumulative_position.time = 0;
	size_t best_solution_subsets_number = std::numeric_limits<size_t>::max();
	while(generation < config.stopping_criterion.generation
	      && rwls_cumulative_position < config.stopping_criterion.rwls_cumulative_position
	      && timer.elapsed() < config.stopping_criterion.time)
	{
		for(solution& solution: population)
		{
			rwls_cumulative_position +=
			  m_rwls.improve(solution, generator, config.rwls_stopping_criterion);

			const size_t solution_subsets_number = solution.selected_subsets.count();
			if(solution_subsets_number < best_solution_subsets_number)
			{
				solution.compute_cover();
				assert(solution.cover_all_points);
				if(!solution.cover_all_points)
				{
					LOGGER->error("Memetic new best solution doesn't cover all points");
					abort();
				}

				report.solution_final = solution;
				report.found_at.generation = generation;
				report.found_at.rwls_cumulative_position = rwls_cumulative_position;
				report.found_at.time = timer.elapsed();
				best_solution_subsets_number = solution_subsets_number;
				LOGGER->info(
				  "({}) Memetic new best solution with {} subsets at generation {} in {}s",
				  m_problem.name,
				  solution_subsets_number,
				  generation,
				  timer.elapsed());
			}
		}

		const solution solution(population[0]);
		population[0] = m_crossover(population[0], population[1], generator);
		population[1] = m_crossover(solution, population[1], generator);

		++generation;
	}

	LOGGER->info("({}) Memetic found solution with {} subsets in {}s",
	             m_problem.name,
	             report.solution_final.selected_subsets.count(),
	             timer.elapsed());

	return report;
}

#endif //USCP_MEMETIC_TPP
