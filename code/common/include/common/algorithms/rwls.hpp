//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_COMMON_RWLS_HPP
#define USCP_COMMON_RWLS_HPP

#include "common/data/solution.hpp"

#include <nlohmann/json.hpp>

#include <cstddef>

namespace uscp::rwls
{
	struct position_serial final
	{
		size_t steps = std::numeric_limits<size_t>::max();
		double time = std::numeric_limits<double>::max();
	};
	void to_json(nlohmann::json& j, const position_serial& serial);
	void from_json(const nlohmann::json& j, position_serial& serial);

	struct report_serial final
	{
		solution_serial solution_initial;
		std::vector<ssize_t> points_weights_initial;
		solution_serial solution_final;
		std::vector<ssize_t> points_weights_final;
		position_serial found_at;
		position_serial ended_at;
		position_serial stopping_criterion;
	};
	void to_json(nlohmann::json& j, const report_serial& serial);
	void from_json(const nlohmann::json& j, report_serial& serial);
} // namespace uscp::rwls

#endif //USCP_COMMON_RWLS_HPP
