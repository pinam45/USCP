//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_ADD_HPP
#define USCP_ADD_HPP

#include "common/data/instance.hpp"
#include "common/data/solution.hpp"
#include "common/utils/random.hpp"

#include <dynamic_bitset.hpp>

#include <string_view>
#include <algorithm>

namespace uscp::wcrossover
{
	struct add final
	{
		explicit add(const uscp::problem::instance& problem_): problem(problem_)
		{
		}
		add(const add&) = default;
		add(add&&) noexcept = default;
		add& operator=(const add& other) = delete;
		add& operator=(add&& other) noexcept = delete;

		[[nodiscard]] std::vector<ssize_t> apply(const std::vector<ssize_t>& a,
		                                         const std::vector<ssize_t>& b) const noexcept
		{
			std::vector<ssize_t> weights;
			weights.reserve(problem.points_number);
			std::transform(std::cbegin(a),
			               std::cend(a),
			               std::cbegin(b),
			               std::back_inserter(weights),
			               std::plus<>());
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
			return "add";
		}

		const uscp::problem::instance& problem;
	};
} // namespace uscp::wcrossover

#endif //USCP_ADD_HPP
