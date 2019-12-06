//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_SHUFFLE_HPP
#define USCP_SHUFFLE_HPP

#include "common/data/instance.hpp"
#include "common/data/solution.hpp"
#include "common/utils/random.hpp"

#include <dynamic_bitset.hpp>

#include <string_view>
#include <algorithm>

namespace uscp::wcrossover
{
	struct shuffle final
	{
		explicit shuffle(const uscp::problem::instance& problem_): problem(problem_)
		{
		}
		shuffle(const shuffle&) = default;
		shuffle(shuffle&&) noexcept = default;
		shuffle& operator=(const shuffle& other) = delete;
		shuffle& operator=(shuffle&& other) noexcept = delete;

		std::vector<ssize_t> apply1(const std::vector<ssize_t>& a,
		                            [[maybe_unused]] const std::vector<ssize_t>& b,
		                            random_engine& generator) const noexcept
		{
			assert(a.size() == problem.points_number && b.size() == problem.points_number);
			std::vector<ssize_t> weights = a;
			std::shuffle(std::begin(weights), std::end(weights), generator);
			return weights;
		}

		std::vector<ssize_t> apply2([[maybe_unused]] const std::vector<ssize_t>& a,
		                            const std::vector<ssize_t>& b,
		                            random_engine& generator) const noexcept
		{
			assert(a.size() == problem.points_number && b.size() == problem.points_number);
			std::vector<ssize_t> weights = b;
			std::shuffle(std::begin(weights), std::end(weights), generator);
			return weights;
		}

		[[nodiscard]] static std::string_view to_string() noexcept
		{
			return "shuffle";
		}

		const uscp::problem::instance& problem;
	};
} // namespace uscp::wcrossover

#endif //USCP_SHUFFLE_HPP
