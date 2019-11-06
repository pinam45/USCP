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

namespace uscp::crossover
{
	struct merge final
	{
		explicit merge(const uscp::problem::instance& problem_): problem(problem_)
		{
		}

		solution operator()(const solution& a,
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

		std::string to_string() const noexcept
		{
			return "merge";
		}

		const uscp::problem::instance& problem;
	};
} // namespace uscp::crossover

#endif //USCP_MERGE_HPP
