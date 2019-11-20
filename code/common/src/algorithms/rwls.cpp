//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include "common/algorithms/rwls.hpp"

void uscp::rwls::to_json(nlohmann::json& j, const uscp::rwls::position_serial& serial)
{
	j = nlohmann::json{
	  {"steps", serial.steps},
	  {"time", serial.time},
	};
}

void uscp::rwls::from_json(const nlohmann::json& j, uscp::rwls::position_serial& serial)
{
	j.at("steps").get_to(serial.steps);
	j.at("time").get_to(serial.time);
}

void uscp::rwls::to_json(nlohmann::json& j, const uscp::rwls::report_serial& serial)
{
	j = nlohmann::json{
	  {"solution_initial", serial.solution_initial},
	  {"points_weights_initial", serial.points_weights_initial},
	  {"solution_final", serial.solution_final},
	  {"points_weights_final", serial.points_weights_final},
	  {"found_at", serial.found_at},
	  {"ended_at", serial.ended_at},
	  {"stopping_criterion", serial.stopping_criterion},
	};
}

void uscp::rwls::from_json(const nlohmann::json& j, uscp::rwls::report_serial& serial)
{
	j.at("solution_initial").get_to(serial.solution_initial);
	j.at("solution_final").get_to(serial.solution_final);

	// support for versions until 870574146e828cc634c7fa02823e27c66164c5a1
	serial.found_at.steps = j.value<size_t>("steps", 0);
	serial.found_at.time = j.value<size_t>("time", 0);

	// support for versions after 870574146e828cc634c7fa02823e27c66164c5a1
	serial.found_at = j.value("found_at", serial.found_at);
	serial.ended_at = j.value<position_serial>("ended_at", {0, 0});
	serial.stopping_criterion = j.value<position_serial>("stopping_criterion", {0, 0});

	// support for versions after 3a02851fe586fd37329ebd5309e9d92ba5d76dfc
	serial.points_weights_initial = j.value<std::vector<ssize_t>>("points_weights_initial", {});
	serial.points_weights_final = j.value<std::vector<ssize_t>>("points_weights_final", {});
}
