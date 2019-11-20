//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_COMMON_MEMETIC_HPP
#define USCP_COMMON_MEMETIC_HPP

#include "common/algorithms/rwls.hpp"

#include <cstddef>
#include <limits>
#include <string>
#include <vector>

namespace uscp::memetic
{
	struct position_serial final
	{
		size_t generation = std::numeric_limits<size_t>::max();
		uscp::rwls::position_serial rwls_cumulative_position;
		double time = std::numeric_limits<double>::max();
	};
	void to_json(nlohmann::json& j, const position_serial& serial);
	void from_json(const nlohmann::json& j, position_serial& serial);

	struct config_serial final
	{
		position_serial stopping_criterion;
		uscp::rwls::position_serial rwls_stopping_criterion;
	};
	void to_json(nlohmann::json& j, const config_serial& serial);
	void from_json(const nlohmann::json& j, config_serial& serial);

	struct report_serial final
	{
		solution_serial solution_final;
		std::vector<ssize_t> points_weights_final;
		position_serial found_at;
		position_serial ended_at;
		config_serial solve_config;
		std::string crossover_operator;
		std::string wcrossover_operator;
	};
	void to_json(nlohmann::json& j, const report_serial& serial);
	void from_json(const nlohmann::json& j, report_serial& serial);
} // namespace uscp::memetic

#endif //USCP_COMMON_MEMETIC_HPP
