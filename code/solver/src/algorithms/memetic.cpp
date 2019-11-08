//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include "solver/algorithms/memetic.hpp"
#include "solver/data/solution.hpp"
#include "common/utils/logger.hpp"

uscp::memetic::position_serial uscp::memetic::position::serialize() const noexcept
{
	position_serial serial;
	serial.generation = generation;
	serial.rwls_cumulative_position = rwls_cumulative_position.serialize();
	serial.time = time;
	return serial;
}

bool uscp::memetic::position::load(const uscp::memetic::position_serial& serial) noexcept
{
	generation = serial.generation;
	if(!rwls_cumulative_position.load(serial.rwls_cumulative_position))
	{
		LOGGER->warn("Failed to load rwls cumulative position");
		return false;
	}
	time = serial.time;
	return true;
}

uscp::memetic::config_serial uscp::memetic::config::serialize() const noexcept
{
	config_serial serial;
	serial.stopping_criterion = stopping_criterion.serialize();
	serial.rwls_stopping_criterion = rwls_stopping_criterion.serialize();
	return serial;
}

bool uscp::memetic::config::load(const uscp::memetic::config_serial& serial) noexcept
{
	if(!stopping_criterion.load(serial.stopping_criterion))
	{
		LOGGER->warn("Failed to load stopping criterion");
		return false;
	}
	if(!rwls_stopping_criterion.load(serial.rwls_stopping_criterion))
	{
		LOGGER->warn("Failed to load rwls stopping criterion");
		return false;
	}
	return true;
}

uscp::memetic::report::report(const uscp::problem::instance& problem) noexcept
  : solution_final(problem), found_at(), solve_config(), crossover_operator()
{
}

uscp::memetic::report_serial uscp::memetic::report::serialize() const noexcept
{
	report_serial serial;
	serial.solution_final = solution_final.serialize();
	serial.found_at = found_at.serialize();
	serial.ended_at = ended_at.serialize();
	serial.solve_config = solve_config.serialize();
	serial.crossover_operator = crossover_operator;
	return serial;
}

bool uscp::memetic::report::load(const uscp::memetic::report_serial& serial) noexcept
{
	if(!solution_final.load(serial.solution_final))
	{
		LOGGER->warn("Failed to load final solution");
		return false;
	}
	if(!found_at.load(serial.found_at))
	{
		LOGGER->warn("Failed to load solution found at position");
		return false;
	}
	if(!ended_at.load(serial.ended_at))
	{
		LOGGER->warn("Failed to load solution ended at position");
		return false;
	}
	if(!solve_config.load(serial.solve_config))
	{
		LOGGER->warn("Failed to load solving config");
		return false;
	}
	crossover_operator = serial.crossover_operator;
	return true;
}

uscp::memetic::report uscp::memetic::expand(const uscp::memetic::report& reduced_report) noexcept
{
	if(!reduced_report.solution_final.problem.reduction.has_value())
	{
		LOGGER->error("Tried to expand report of non-reduced instance");
		return reduced_report;
	}
	report expanded_report(*reduced_report.solution_final.problem.reduction->parent_instance);
	expanded_report.solution_final = expand(reduced_report.solution_final);
	expanded_report.found_at = reduced_report.found_at;
	expanded_report.ended_at = reduced_report.ended_at;
	expanded_report.solve_config = reduced_report.solve_config;
	expanded_report.crossover_operator = reduced_report.crossover_operator;
	return expanded_report;
}
