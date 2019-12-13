//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_EXTENDED_SUBPROBLEM_RANDOM_HPP
#define USCP_EXTENDED_SUBPROBLEM_RANDOM_HPP

#include "common/data/instance.hpp"
#include "common/data/solution.hpp"
#include "common/utils/random.hpp"
#include "solver/algorithms/random.hpp"

#include <dynamic_bitset.hpp>

#include <string_view>

namespace uscp::crossover
{
	struct extended_subproblem_random final
	{
		explicit extended_subproblem_random(const uscp::problem::instance& problem_)
		  : problem(problem_)
		{
		}
		extended_subproblem_random(const extended_subproblem_random&) = default;
		extended_subproblem_random(extended_subproblem_random&&) noexcept = default;
		extended_subproblem_random& operator=(const extended_subproblem_random& other) = delete;
		extended_subproblem_random& operator=(extended_subproblem_random&& other) noexcept = delete;

		solution apply(const solution& a, const solution& b, random_engine& generator) const
		  noexcept
		{
			solution random_solution =
			  uscp::random::solve(generator, problem, NULL_LOGGER); // to extend the subproblem
			solution solution(problem);
			dynamic_bitset<> authorized_subsets = random_solution.selected_subsets;
			authorized_subsets |= a.selected_subsets;
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
			return "extended_subproblem_random";
		}

		const uscp::problem::instance& problem;
	};
} // namespace uscp::crossover

#endif //USCP_EXTENDED_SUBPROBLEM_RANDOM_HPP
