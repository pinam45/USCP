//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_RANDOM_SUBPROBLEM_HPP
#define USCP_RANDOM_SUBPROBLEM_HPP

#include "common/data/instance.hpp"
#include "common/data/solution.hpp"
#include "common/utils/random.hpp"

#include <dynamic_bitset.hpp>

#include <string_view>

namespace uscp::crossover
{
	struct random_subproblem final
	{
		explicit random_subproblem(const uscp::problem::instance& problem_): problem(problem_)
		{
		}

		solution operator()(const solution& a, const solution& b, random_engine& generator) const
		  noexcept
		{
			solution solution(problem);
			dynamic_bitset<> authorized_subsets = a.selected_subsets;
			authorized_subsets |= b.selected_subsets;
			std::uniform_int_distribution<size_t> dist(0, problem.subsets_number - 1);
			while(!solution.covered_points.all())
			{
				assert(!solution.selected_subsets.all());
				size_t selected_subset = dist(generator);
				while(!authorized_subsets.test(selected_subset)
				      || solution.selected_subsets.test(selected_subset))
				{
					selected_subset = dist(generator);
				}
				solution.selected_subsets.set(selected_subset);
				solution.covered_points |= problem.subsets_points[selected_subset];
			}
			solution.cover_all_points = true;
			return solution;
		}

		[[nodiscard]] static std::string_view to_string() noexcept
		{
			return "random_subproblem";
		}

		const uscp::problem::instance& problem;
	};
} // namespace uscp::crossover

#endif //USCP_RANDOM_SUBPROBLEM_HPP
