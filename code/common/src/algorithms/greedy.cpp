//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include "common/algorithms/greedy.hpp"

void uscp::greedy::to_json(nlohmann::json& j, const uscp::greedy::report_serial& serial)
{
	j = nlohmann::json{{"solution_final", serial.solution_final}, {"time", serial.time}};
}

void uscp::greedy::from_json(const nlohmann::json& j, uscp::greedy::report_serial& serial)
{
	j.at("solution_final").get_to(serial.solution_final);
	j.at("time").get_to(serial.time);
}
