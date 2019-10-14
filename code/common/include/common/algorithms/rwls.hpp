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
	struct report_serial final
	{
		solution_serial solution_initial;
		solution_serial solution_final;
		size_t steps = 0;
		double time = 0;
	};
	void to_json(nlohmann::json& j, const report_serial& serial);
	void from_json(const nlohmann::json& j, report_serial& serial);
} // namespace uscp::rwls

#endif //USCP_COMMON_RWLS_HPP