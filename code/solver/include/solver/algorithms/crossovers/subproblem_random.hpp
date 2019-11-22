//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_SUBPROBLEM_RANDOM_HPP
#define USCP_SUBPROBLEM_RANDOM_HPP

#include "common/data/instance.hpp"
#include "common/data/solution.hpp"
#include "common/utils/random.hpp"

#include <dynamic_bitset.hpp>

#include <string_view>

namespace uscp::crossover
{
	struct subproblem_random final
	{
		explicit subproblem_random(const uscp::problem::instance& problem_): problem(problem_)
		{
		}
		subproblem_random(const subproblem_random&) = default;
		subproblem_random(subproblem_random&&) noexcept = default;
		subproblem_random& operator=(const subproblem_random& other) = delete;
		subproblem_random& operator=(subproblem_random&& other) noexcept = delete;

		solution apply(const solution& a, const solution& b, random_engine& generator) const
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

		solution apply1(const solution& a, const solution& b, random_engine& generator) const
		  noexcept
		{
			return apply(a, b, generator);
		}

		solution apply2(const solution& a, const solution& b, random_engine& generator) const
		  noexcept
		{
			return apply(a, b, generator);
		}

		[[nodiscard]] static std::string_view to_string() noexcept
		{
			return "subproblem_random";
		}

		const uscp::problem::instance& problem;
	};
} // namespace uscp::crossover

#endif //USCP_SUBPROBLEM_RANDOM_HPP
