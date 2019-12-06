//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_MAX_HPP
#define USCP_MAX_HPP

#include "common/data/instance.hpp"
#include "common/data/solution.hpp"
#include "common/utils/random.hpp"

#include <dynamic_bitset.hpp>

#include <string_view>
#include <algorithm>

namespace uscp::wcrossover
{
	struct max final
	{
		explicit max(const uscp::problem::instance& problem_): problem(problem_)
		{
		}
		max(const max&) = default;
		max(max&&) noexcept = default;
		max& operator=(const max& other) = delete;
		max& operator=(max&& other) noexcept = delete;

		[[nodiscard]] std::vector<ssize_t> apply(const std::vector<ssize_t>& a,
		                                         const std::vector<ssize_t>& b) const noexcept
		{
			std::vector<ssize_t> weights;
			weights.reserve(problem.points_number);
			std::transform(std::cbegin(a),
			               std::cend(a),
			               std::cbegin(b),
			               std::back_inserter(weights),
			               [](ssize_t wa, ssize_t wb) { return wa > wb ? wa : wb; });
			return weights;
		}

		std::vector<ssize_t> apply1(const std::vector<ssize_t>& a,
		                            const std::vector<ssize_t>& b,
		                            [[maybe_unused]] random_engine& generator) const noexcept
		{
			assert(a.size() == problem.points_number && b.size() == problem.points_number);
			return apply(a, b);
		}

		std::vector<ssize_t> apply2(const std::vector<ssize_t>& a,
		                            const std::vector<ssize_t>& b,
		                            [[maybe_unused]] random_engine& generator) const noexcept
		{
			assert(a.size() == problem.points_number && b.size() == problem.points_number);
			return apply(a, b);
		}

		[[nodiscard]] static std::string_view to_string() noexcept
		{
			return "max";
		}

		const uscp::problem::instance& problem;
	};
} // namespace uscp::wcrossover

#endif //USCP_MAX_HPP
