//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_SUBPROBLEM_RWLS_HPP
#define USCP_SUBPROBLEM_RWLS_HPP

#include "common/data/instance.hpp"
#include "common/data/solution.hpp"
#include "common/utils/random.hpp"
#include "common/utils/logger.hpp"
#include "solver/algorithms/rwls.hpp"

#include <dynamic_bitset.hpp>

#include <string_view>

namespace uscp::crossover
{
	struct subproblem_rwls final
	{
		explicit subproblem_rwls(const uscp::problem::instance& problem_)
		  : problem(problem_), rwls(problem_, NULL_LOGGER)
		{
			rwls.initialize();
			stopping_criterion.steps = problem.subsets_number;
			subsets_covering_points.resize(problem.points_number);
			for(size_t i = 0; i < problem.points_number; ++i)
			{
				subsets_covering_points[i].resize(problem.subsets_number);
			}
			for(size_t i = 0; i < problem.subsets_number; ++i)
			{
				problem.subsets_points[i].iterate_bits_on([&](size_t bit_on) noexcept {
					assert(bit_on < subsets_covering_points.size());
					assert(i < subsets_covering_points[bit_on].size());
					subsets_covering_points[bit_on].set(i);
				});
			}
		}
		subproblem_rwls(const subproblem_rwls&) = default;
		subproblem_rwls(subproblem_rwls&&) noexcept = default;
		subproblem_rwls& operator=(const subproblem_rwls& other) = delete;
		subproblem_rwls& operator=(subproblem_rwls&& other) noexcept = delete;

		solution apply(solution solution,
		               dynamic_bitset<> authorized_subsets,
		               random_engine& generator)
		{
			// inclusion reduction for RWLS
			dynamic_bitset<> tmp(problem.subsets_number);
			dynamic_bitset<> included_subsets(problem.subsets_number);
			for(size_t i = 0; i < problem.points_number; ++i)
			{
				tmp = subsets_covering_points[i];
				tmp &= authorized_subsets;
				assert(tmp.count() > 0);
				if(tmp.count() == 1)
				{
					const size_t only_authorized_subset_covering = tmp.find_first();
					included_subsets.set(only_authorized_subset_covering);
				}
			}
			authorized_subsets -= included_subsets;
			solution.selected_subsets |= included_subsets;

			uscp::rwls::report report =
			  rwls.restricted_improve(solution, generator, stopping_criterion, authorized_subsets);

			return report.solution_final;
		}

		solution apply1(const solution& a, const solution& b, random_engine& generator) noexcept
		{
			dynamic_bitset<> authorized_subsets = a.selected_subsets;
			authorized_subsets |= b.selected_subsets;
			solution solution =
			  uscp::greedy::restricted_solve(problem, authorized_subsets, NULL_LOGGER);
			assert(solution.cover_all_points);

			return apply(std::move(solution), std::move(authorized_subsets), generator);
		}

		solution apply2(const solution& a, const solution& b, random_engine& generator) noexcept
		{
			dynamic_bitset<> authorized_subsets = a.selected_subsets;
			authorized_subsets |= b.selected_subsets;
			solution solution =
			  uscp::greedy::restricted_rsolve(problem, authorized_subsets, NULL_LOGGER);
			assert(solution.cover_all_points);

			return apply(std::move(solution), std::move(authorized_subsets), generator);
		}

		[[nodiscard]] static std::string_view to_string() noexcept
		{
			return "subproblem_rwls";
		}

		const uscp::problem::instance& problem;
		std::vector<dynamic_bitset<>> subsets_covering_points;
		uscp::rwls::rwls rwls;
		uscp::rwls::position stopping_criterion;
	};
} // namespace uscp::crossover

#endif //USCP_SUBPROBLEM_RWLS_HPP
