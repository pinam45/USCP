//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_AVERAGE_HPP
#define USCP_AVERAGE_HPP

#include "common/data/instance.hpp"
#include "common/data/solution.hpp"
#include "common/utils/random.hpp"

#include <dynamic_bitset.hpp>

#include <string_view>

namespace uscp::wcrossover
{
	struct average final
	{
		explicit average(const uscp::problem::instance& problem_): problem(problem_)
		{
		}
		average(const average&) = default;
		average(average&&) noexcept = default;
		average& operator=(const average& other) = delete;
		average& operator=(average&& other) noexcept = delete;

		[[nodiscard]] std::vector<long long> apply(const std::vector<long long>& a,
		                                           const std::vector<long long>& b) const noexcept
		{
			std::vector<long long> weights;
			weights.reserve(problem.points_number);
			std::transform(std::cbegin(a),
			               std::cend(a),
			               std::cbegin(b),
			               std::back_inserter(weights),
			               [](long long wa, long long wb) { return (wa + wb) / 2; });
			return weights;
		}

		std::vector<long long> apply1(const std::vector<long long>& a,
		                              const std::vector<long long>& b,
		                              [[maybe_unused]] random_engine& generator) const noexcept
		{
			assert(a.size() == problem.points_number && b.size() == problem.points_number);
			return apply(a, b);
		}

		std::vector<long long> apply2(const std::vector<long long>& a,
		                              const std::vector<long long>& b,
		                              [[maybe_unused]] random_engine& generator) const noexcept
		{
			assert(a.size() == problem.points_number && b.size() == problem.points_number);
			return apply(a, b);
		}

		[[nodiscard]] static std::string_view to_string() noexcept
		{
			return "average";
		}

		const uscp::problem::instance& problem;
	};
} // namespace uscp::wcrossover

#endif //USCP_AVERAGE_HPP
