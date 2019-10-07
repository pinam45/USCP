//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_RWLS_HPP
#define USCP_RWLS_HPP

#include "common/data/instance.hpp"
#include "common/data/solution.hpp"
#include "common/utils/random.hpp"

#include <nlohmann/json.hpp>

#include <cstddef>
#include <limits>

namespace uscp::rwls
{
	struct stop final
	{
		size_t steps = std::numeric_limits<size_t>::max();
		double time = std::numeric_limits<double>::max();
	};

	struct report_serial final
	{
		solution_serial solution_initial;
		solution_serial solution_final;
		size_t steps = 0;
		double time = 0;
	};
	void to_json(nlohmann::json& j, const report_serial& serial);
	void from_json(const nlohmann::json& j, report_serial& serial);

	struct report final
	{
		solution solution_initial;
		solution solution_final;
		stop found_at;

		explicit report(const problem::instance& problem) noexcept;
		report(const report&) = default;
		report(report&&) noexcept = default;
		report& operator=(const report& other) = default;
		report& operator=(report&& other) noexcept = default;

		[[nodiscard]] report_serial serialize() const noexcept;
		bool load(const report_serial& serial) noexcept;
	};

	[[nodiscard]] solution improve(const uscp::solution& solution,
	                               random_engine& generator,
	                               stop stopping_criterion);
	[[nodiscard]] report improve_report(const uscp::solution& solution,
	                                    random_engine& generator,
	                                    stop stopping_criterion);

	[[nodiscard]] report expand(const report& reduced_report) noexcept;
} // namespace uscp::rwls

#endif //USCP_RWLS_HPP
