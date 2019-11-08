//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include "common/algorithms/memetic.hpp"

void uscp::memetic::to_json(nlohmann::json& j, const uscp::memetic::position_serial& serial)
{
	j = nlohmann::json{
	  {"generation", serial.generation},
	  {"rwls_cumulative_position", serial.rwls_cumulative_position},
	  {"time", serial.time},
	};
}

void uscp::memetic::from_json(const nlohmann::json& j, uscp::memetic::position_serial& serial)
{
	j.at("generation").get_to(serial.generation);
	j.at("rwls_cumulative_position").get_to(serial.rwls_cumulative_position);
	j.at("time").get_to(serial.time);
}

void uscp::memetic::to_json(nlohmann::json& j, const uscp::memetic::config_serial& serial)
{
	j = nlohmann::json{
	  {"stopping_criterion", serial.stopping_criterion},
	  {"rwls_stopping_criterion", serial.rwls_stopping_criterion},
	};
}

void uscp::memetic::from_json(const nlohmann::json& j, uscp::memetic::config_serial& serial)
{
	j.at("stopping_criterion").get_to(serial.stopping_criterion);
	j.at("rwls_stopping_criterion").get_to(serial.rwls_stopping_criterion);
}

void uscp::memetic::to_json(nlohmann::json& j, const uscp::memetic::report_serial& serial)
{
	j = nlohmann::json{
	  {"solution_final", serial.solution_final},
	  {"found_at", serial.found_at},
	  {"ended_at", serial.ended_at},
	  {"solve_config", serial.solve_config},
	  {"crossover_operator", serial.crossover_operator},
	};
}

void uscp::memetic::from_json(const nlohmann::json& j, uscp::memetic::report_serial& serial)
{
	j.at("solution_final").get_to(serial.solution_final);
	j.at("found_at").get_to(serial.found_at);
	j.at("ended_at").get_to(serial.ended_at);
	j.at("solve_config").get_to(serial.solve_config);
	j.at("crossover_operator").get_to(serial.crossover_operator);
}
