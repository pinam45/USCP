//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_KEEP_HPP
#define USCP_KEEP_HPP

#include "common/data/instance.hpp"
#include "common/data/solution.hpp"
#include "common/utils/random.hpp"

#include <dynamic_bitset.hpp>

#include <string_view>

namespace uscp::wcrossover
{
	struct keep final
	{
		explicit keep(const uscp::problem::instance& problem_): problem(problem_)
		{
		}
		keep(const keep&) = default;
		keep(keep&&) noexcept = default;
		keep& operator=(const keep& other) = delete;
		keep& operator=(keep&& other) noexcept = delete;

		std::vector<long long> apply1(const std::vector<long long>& a,
		                              [[maybe_unused]] const std::vector<long long>& b,
		                              [[maybe_unused]] random_engine& generator) const noexcept
		{
			assert(a.size() == problem.points_number && b.size() == problem.points_number);
			return a;
		}

		std::vector<long long> apply2([[maybe_unused]] const std::vector<long long>& a,
		                              const std::vector<long long>& b,
		                              [[maybe_unused]] random_engine& generator) const noexcept
		{
			assert(a.size() == problem.points_number && b.size() == problem.points_number);
			return b;
		}

		[[nodiscard]] static std::string_view to_string() noexcept
		{
			return "keep";
		}

		const uscp::problem::instance& problem;
	};
} // namespace uscp::wcrossover

#endif //USCP_KEEP_HPP
