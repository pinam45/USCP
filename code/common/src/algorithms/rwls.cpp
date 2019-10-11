//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include "common/algorithms/rwls.hpp"

void uscp::rwls::to_json(nlohmann::json& j, const uscp::rwls::report_serial& serial)
{
	j = nlohmann::json{
	  {"solution_initial", serial.solution_initial},
	  {"solution_final", serial.solution_final},
	  {"steps", serial.steps},
	  {"time", serial.time},
	};
}

void uscp::rwls::from_json(const nlohmann::json& j, uscp::rwls::report_serial& serial)
{
	j.at("solution_initial").get_to(serial.solution_initial);
	j.at("solution_final").get_to(serial.solution_final);
	j.at("steps").get_to(serial.steps);
	j.at("time").get_to(serial.time);
}
