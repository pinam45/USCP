//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_MERGE_HPP
#define USCP_MERGE_HPP

#include "common/data/instance.hpp"
#include "common/data/solution.hpp"
#include "common/utils/random.hpp"

#include <dynamic_bitset.hpp>

#include <string_view>

namespace uscp::crossover
{
	struct merge final
	{
		explicit merge(const uscp::problem::instance& problem_): problem(problem_)
		{
		}

		solution apply(const solution& a,
		                    const solution& b,
		                    [[maybe_unused]] random_engine& generator) const noexcept
		{
			solution solution(problem);
			solution.selected_subsets = a.selected_subsets;
			solution.selected_subsets |= b.selected_subsets;
			solution.covered_points.set();
			solution.cover_all_points = true;
			return solution;
		}

		solution apply1(const solution& a, const solution& b, random_engine& generator) const
		noexcept
		{
			return apply(a, b , generator);
		}

		solution apply2(const solution& a, const solution& b, random_engine& generator) const
		noexcept
		{
			return apply(a, b , generator);
		}

		[[nodiscard]] static std::string_view to_string() noexcept
		{
			return "merge";
		}

		const uscp::problem::instance& problem;
	};
} // namespace uscp::crossover

#endif //USCP_MERGE_HPP
