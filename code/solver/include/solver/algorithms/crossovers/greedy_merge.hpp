//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_GREEDY_MERGE_HPP
#define USCP_GREEDY_MERGE_HPP

#include "common/data/instance.hpp"
#include "common/data/solution.hpp"
#include "common/utils/random.hpp"

#include <dynamic_bitset.hpp>

#include <string_view>

namespace uscp::crossover
{
	struct greedy_merge final
	{
		explicit greedy_merge(const uscp::problem::instance& problem_): problem(problem_)
		{
		}

		solution apply(const solution& a,
		               const solution& b,
		               [[maybe_unused]] random_engine& generator) const noexcept
		{
			const solution* current = &a;
			bool current_is_a = true;
			solution solution(problem);
			while(!solution.cover_all_points)
			{
				size_t max_subset_number = solution.selected_subsets.size(); //invalid initial value
				dynamic_bitset<> covered_points_with_max_subset(problem.points_number);
				size_t covered_points_number_with_max_subset = solution.covered_points.count();
				dynamic_bitset<> new_covered_points;
				current->selected_subsets.iterate_bits_on([&](size_t i) {
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
					continue;
				}
				// update solution
				solution.selected_subsets[max_subset_number].set();
				solution.covered_points = covered_points_with_max_subset;
				solution.cover_all_points = solution.covered_points.all();

				// change current
				current = current_is_a ? &b : &a;
				current_is_a = !current_is_a;
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
			return apply(b, a, generator);
		}

		[[nodiscard]] static std::string_view to_string() noexcept
		{
			return "greedy_merge";
		}

		const uscp::problem::instance& problem;
	};
} // namespace uscp::crossover

#endif //USCP_GREEDY_MERGE_HPP
