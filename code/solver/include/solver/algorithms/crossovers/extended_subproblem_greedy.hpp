//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_EXTENDED_SUBPROBLEM_GREEDY_HPP
#define USCP_EXTENDED_SUBPROBLEM_GREEDY_HPP

#include "common/data/instance.hpp"
#include "common/data/solution.hpp"
#include "common/utils/random.hpp"
#include "common/utils/logger.hpp"
#include "solver/algorithms/random.hpp"

#include <dynamic_bitset.hpp>

#include <string_view>

namespace uscp::crossover
{
	struct extended_subproblem_greedy final
	{
		explicit extended_subproblem_greedy(const uscp::problem::instance& problem_)
		  : problem(problem_)
		{
		}
		extended_subproblem_greedy(const extended_subproblem_greedy&) = default;
		extended_subproblem_greedy(extended_subproblem_greedy&&) noexcept = default;
		extended_subproblem_greedy& operator=(const extended_subproblem_greedy& other) = delete;
		extended_subproblem_greedy& operator=(extended_subproblem_greedy&& other) noexcept = delete;

		solution apply(const solution& a, const solution& b, random_engine& generator) const
		  noexcept
		{
			solution random_solution =
			  uscp::random::solve(generator, problem, NULL_LOGGER); // to extend the subproblem
			solution solution(problem);
			dynamic_bitset<> authorized_subsets = random_solution.selected_subsets;
			authorized_subsets |= a.selected_subsets;
			authorized_subsets |= b.selected_subsets;
			while(!solution.cover_all_points)
			{
				size_t max_subset_number = solution.selected_subsets.size(); //invalid initial value
				dynamic_bitset<> covered_points_with_max_subset(problem.points_number);
				size_t covered_points_number_with_max_subset = solution.covered_points.count();
				dynamic_bitset<> new_covered_points;
				authorized_subsets.iterate_bits_on([&](size_t i) {
					if(solution.selected_subsets[i])
					{
						// already selected
						return;
					}

					new_covered_points = solution.covered_points;
					new_covered_points |= problem.subsets_points[i];
					const size_t new_covered_points_number = new_covered_points.count();
					if(new_covered_points_number > covered_points_number_with_max_subset)
					{
						max_subset_number = i;
						covered_points_with_max_subset = new_covered_points;
						covered_points_number_with_max_subset = new_covered_points_number;
					}
				});

				// all subset already included or no subset add covered points
				if(max_subset_number == solution.selected_subsets.size())
				{
					LOGGER->error("The subproblem has no solution");
					abort();
				}

				// update solution
				solution.selected_subsets[max_subset_number].set();
				solution.covered_points = covered_points_with_max_subset;
				solution.cover_all_points = solution.covered_points.all();
			}
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
			return "extended_subproblem_greedy";
		}

		const uscp::problem::instance& problem;
	};
} // namespace uscp::crossover

#endif //USCP_EXTENDED_SUBPROBLEM_GREEDY_HPP
