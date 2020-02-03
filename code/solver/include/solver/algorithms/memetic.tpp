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
#include "greedy.hpp"

#include <array>

template<typename Crossover, typename WeightsCrossover>
uscp::memetic::memetic<Crossover, WeightsCrossover>::memetic(
  const problem::instance& problem) noexcept
  : m_problem(problem)
  , m_crossover(problem)
  , m_wcrossover(problem)
  , m_rwls(problem, NULL_LOGGER)
  , m_initialized(false)
{
}

template<typename Crossover, typename WeightsCrossover>
void uscp::memetic::memetic<Crossover, WeightsCrossover>::initialize() noexcept
{
	m_rwls.initialize();
	m_initialized = true;
}

template<typename Crossover, typename WeightsCrossover>
uscp::memetic::report uscp::memetic::memetic<Crossover, WeightsCrossover>::solve(
  uscp::random_engine& generator,
  uscp::memetic::config config) noexcept
{
	// Memetic algorithm now uses dynamic RWLS steps, thus some parameters are now ignored
	config.stopping_criterion.generation = std::numeric_limits<size_t>::max();
	config.rwls_stopping_criterion.steps = std::numeric_limits<size_t>::max();
	config.rwls_stopping_criterion.time = std::numeric_limits<double>::max();

	if(!m_initialized)
	{
		initialize();
	}

	LOGGER->info("({}) Start solving problem with memetic algorithm ({}/{})",
	             m_problem.name,
	             Crossover::to_string(),
	             WeightsCrossover::to_string());

	report report(m_problem);
	report.solve_config = config;
	report.crossover_operator = m_crossover.to_string();
	report.wcrossover_operator = m_wcrossover.to_string();

	timer timer;

	// Population weights
	std::array<std::vector<ssize_t>, 2> population_weights;
	for(std::vector<ssize_t>& weights: population_weights)
	{
		weights.resize(m_problem.points_number, 1);
	}

	// Population
	std::array<solution, 2> population{solution(m_problem), solution(m_problem)};
#pragma omp parallel for default(none) shared(population, generator, m_problem, NULL_LOGGER)
	for(size_t i = 0; i < population.size(); ++i)
	{
		population[i] = uscp::greedy::random_solve(generator, m_problem, NULL_LOGGER);
	}
	SPDLOG_LOGGER_DEBUG(
	  LOGGER, "({}) Memetic population initialized in {}s", m_problem.name, timer.elapsed());

	size_t generation = 0;
	uscp::rwls::position rwls_cumulative_position;
	rwls_cumulative_position.steps = 0;
	rwls_cumulative_position.time = 0;
	size_t best_solution_subsets_number = std::numeric_limits<size_t>::max();
	std::array<uscp::rwls::report, 2> rwls_reports{uscp::rwls::report(m_problem),
	                                               uscp::rwls::report(m_problem)};

	// Best initial solution
	{
		const std::array<size_t, 2> initial_population_subsets_number = {
		  population[0].selected_subsets.count(), population[1].selected_subsets.count()};
		const size_t initial_population_best =
		  initial_population_subsets_number[0] < initial_population_subsets_number[1] ? 0 : 1;
		best_solution_subsets_number = initial_population_subsets_number[initial_population_best];
		report.solution_final = population[initial_population_best];
		report.found_at.generation = 0;
		report.found_at.rwls_cumulative_position.steps = 0;
		report.found_at.rwls_cumulative_position.time = 0;
		report.found_at.time = timer.elapsed();
		report.points_weights_final = rwls_reports[initial_population_best].points_weights_final;
	}

	// Dynamic steps setup
	std::deque<size_t> dynamic_steps;
	static constexpr size_t dynamic_steps_recorded_generations = 10;
	const size_t base_steps = m_problem.subsets_number + m_problem.points_number;
	dynamic_steps.resize(dynamic_steps_recorded_generations, base_steps);
	config.rwls_stopping_criterion.steps = base_steps;

	// Main loop
	while(generation < config.stopping_criterion.generation
	      && rwls_cumulative_position < config.stopping_criterion.rwls_cumulative_position
	      && timer.elapsed() < config.stopping_criterion.time)
	{
		LOGGER->info("[------------------------------] ({}) Memetic generation {}, start at {}s",
		             m_problem.name,
		             generation,
		             timer.elapsed());
#pragma omp parallel for default(none) \
  shared(population, population_weights, rwls_reports, config, generator)
		for(size_t i = 0; i < population.size(); ++i)
		{
			rwls_reports[i] = m_rwls.improve(
			  population[i], population_weights[i], generator, config.rwls_stopping_criterion);
		}
		for(size_t i = 0; i < rwls_reports.size(); ++i)
		{
			const size_t solution_subsets_number =
			  rwls_reports[i].solution_final.selected_subsets.count();
			if(solution_subsets_number < best_solution_subsets_number)
			{
				rwls_reports[i].solution_final.compute_cover();
				assert(rwls_reports[i].solution_final.cover_all_points);
				if(!rwls_reports[i].solution_final.cover_all_points)
				{
					LOGGER->error("Memetic new best solution doesn't cover all points");
					abort();
				}

				report.solution_final = rwls_reports[i].solution_final;
				report.found_at.generation = generation;
				report.found_at.rwls_cumulative_position =
				  rwls_cumulative_position + rwls_reports[i].found_at;
				report.found_at.time = timer.elapsed();
				report.points_weights_final = rwls_reports[i].points_weights_final;
				best_solution_subsets_number = solution_subsets_number;
				LOGGER->info(
				  "({}) Memetic new best solution with {} subsets at generation {} in {}s",
				  m_problem.name,
				  solution_subsets_number,
				  generation,
				  timer.elapsed());
			}
		}
		for(size_t i = 0; i < rwls_reports.size(); ++i)
		{
			rwls_cumulative_position += rwls_reports[i].ended_at;
		}
		LOGGER->info("({}) M g{}: applied {} RWLS steps to get parents",
		             m_problem.name,
		             generation,
		             config.rwls_stopping_criterion.steps);
		LOGGER->info("({}) M g{}: parents found at: ({}, {})",
		             m_problem.name,
		             generation,
		             rwls_reports[0].found_at.steps,
		             rwls_reports[1].found_at.steps);
		LOGGER->info("({}) M g{}: parents subsets: ({}, {}){}",
		             m_problem.name,
		             generation,
		             rwls_reports[0].solution_final.selected_subsets.count(),
		             rwls_reports[1].solution_final.selected_subsets.count(),
		             rwls_reports[0].solution_final.selected_subsets
		                 == rwls_reports[1].solution_final.selected_subsets
		               ? " [same]"
		               : "");

		if(rwls_reports[0].found_at.steps == 0 || rwls_reports[1].found_at.steps == 0)
		{
			LOGGER->info(
			  "({}) M g{}: RWLS did not change the parents: randomize parents and double RWLS steps of next generation",
			  m_problem.name,
			  generation);
			config.rwls_stopping_criterion.steps *= 2;
			rwls_reports[0].solution_final = uscp::random::solve(generator, m_problem, NULL_LOGGER);
			rwls_reports[1].solution_final = uscp::random::solve(generator, m_problem, NULL_LOGGER);
			LOGGER->info("({}) M g{}: new parents subsets: ({}, {}){}",
			             m_problem.name,
			             generation,
			             rwls_reports[0].solution_final.selected_subsets.count(),
			             rwls_reports[1].solution_final.selected_subsets.count(),
			             rwls_reports[0].solution_final.selected_subsets
			                 == rwls_reports[1].solution_final.selected_subsets
			               ? " [same]"
			               : "");
		}
		else
		{
			if(rwls_reports[0].solution_final.selected_subsets
			   == rwls_reports[1].solution_final.selected_subsets)
			{
				LOGGER->info(
				  "({}) M g{}: same parents: randomize parents", m_problem.name, generation);
				rwls_reports[0].solution_final =
				  uscp::random::solve(generator, m_problem, NULL_LOGGER);
				rwls_reports[1].solution_final =
				  uscp::random::solve(generator, m_problem, NULL_LOGGER);
				LOGGER->info("({}) M g{}: new parents subsets: ({}, {}){}",
				             m_problem.name,
				             generation,
				             rwls_reports[0].solution_final.selected_subsets.count(),
				             rwls_reports[1].solution_final.selected_subsets.count(),
				             rwls_reports[0].solution_final.selected_subsets
				                 == rwls_reports[1].solution_final.selected_subsets
				               ? " [same]"
				               : "");
			}

			dynamic_steps.push_back(rwls_reports[0].found_at.steps
			                        + rwls_reports[1].found_at.steps);
			dynamic_steps.pop_front();
			config.rwls_stopping_criterion.steps =
			  base_steps
			  + std::accumulate(std::begin(dynamic_steps), std::end(dynamic_steps), size_t(0))
			      / dynamic_steps.size();
		}

#pragma omp parallel sections
		{
#pragma omp section
			{
				population[0] = m_crossover.apply1(
				  rwls_reports[0].solution_final, rwls_reports[1].solution_final, generator);
			}
#pragma omp section
			{
				population[1] = m_crossover.apply2(
				  rwls_reports[0].solution_final, rwls_reports[1].solution_final, generator);
			}
		}
		LOGGER->info("({}) M g{}: children subsets: ({}, {}){}",
		             m_problem.name,
		             generation,
		             population[0].selected_subsets.count(),
		             population[1].selected_subsets.count(),
		             population[0].selected_subsets == population[1].selected_subsets ? " [same]"
		                                                                              : "");

#pragma omp parallel sections
		{
#pragma omp section
			{
				population_weights[0] = m_wcrossover.apply1(rwls_reports[0].points_weights_final,
				                                            rwls_reports[0].points_weights_final,
				                                            generator);
			}
#pragma omp section
			{
				population_weights[1] = m_wcrossover.apply2(rwls_reports[0].points_weights_final,
				                                            rwls_reports[0].points_weights_final,
				                                            generator);
			}
		}
		LOGGER->info("({}) M g{}: current best solution subsets number: {}",
		             m_problem.name,
		             generation,
		             best_solution_subsets_number);

		++generation;
	}

	report.ended_at.generation = generation;
	report.ended_at.rwls_cumulative_position = rwls_cumulative_position;
	report.ended_at.time = timer.elapsed();

	LOGGER->info("({}) Memetic found solution with {} subsets in {}s",
	             m_problem.name,
	             report.solution_final.selected_subsets.count(),
	             timer.elapsed());

	return report;
}

#endif //USCP_MEMETIC_TPP
