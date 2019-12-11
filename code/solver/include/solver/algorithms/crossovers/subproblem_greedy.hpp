//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_SUBPROBLEM_GREEDY_HPP
#define USCP_SUBPROBLEM_GREEDY_HPP

#include "common/data/instance.hpp"
#include "common/data/solution.hpp"
#include "common/utils/random.hpp"
#include "common/utils/logger.hpp"

#include <dynamic_bitset.hpp>

#include <string_view>

namespace uscp::crossover
{
	struct subproblem_greedy final
	{
		explicit subproblem_greedy(const uscp::problem::instance& problem_): problem(problem_)
		{
		}
		subproblem_greedy(const subproblem_greedy&) = default;
		subproblem_greedy(subproblem_greedy&&) noexcept = default;
		subproblem_greedy& operator=(const subproblem_greedy& other) = delete;
		subproblem_greedy& operator=(subproblem_greedy&& other) noexcept = delete;

		solution apply1(const solution& a,
		                const solution& b,
		                [[maybe_unused]] random_engine& generator) const noexcept
		{
			solution solution(problem);
			dynamic_bitset<> authorized_subsets = a.selected_subsets;
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
					if(new_covered_points_number
					   > covered_points_number_with_max_subset) // > for apply1
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

				// update solution (faster)
				solution.selected_subsets[max_subset_number].set();
				solution.covered_points = covered_points_with_max_subset;
				solution.cover_all_points = solution.covered_points.all();
			}
			return solution;
		}

		solution apply2(const solution& a,
		                const solution& b,
		                [[maybe_unused]] random_engine& generator) const noexcept
		{
			solution solution(problem);
			dynamic_bitset<> authorized_subsets = a.selected_subsets;
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
					if(new_covered_points_number
					   >= covered_points_number_with_max_subset) // >= for apply2
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

				// update solution (faster)
				solution.selected_subsets[max_subset_number].set();
				solution.covered_points = covered_points_with_max_subset;
				solution.cover_all_points = solution.covered_points.all();
			}
			return solution;
		}

		[[nodiscard]] static std::string_view to_string() noexcept
		{
			return "subproblem_greedy";
		}

		const uscp::problem::instance& problem;
	};
} // namespace uscp::crossover

#endif //USCP_SUBPROBLEM_GREEDY_HPP
