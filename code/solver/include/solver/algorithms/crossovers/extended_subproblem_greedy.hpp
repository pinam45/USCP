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
#include "solver/algorithms/greedy.hpp"

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

		solution apply1(const solution& a, const solution& b, random_engine& generator) const
		  noexcept
		{
			solution random_solution =
			  uscp::random::solve(generator, problem, NULL_LOGGER); // to extend the subproblem
			dynamic_bitset<> authorized_subsets = random_solution.selected_subsets;
			authorized_subsets |= a.selected_subsets;
			authorized_subsets |= b.selected_subsets;
			return uscp::greedy::restricted_solve(problem, authorized_subsets, NULL_LOGGER);
		}

		solution apply2(const solution& a, const solution& b, random_engine& generator) const
		  noexcept
		{
			solution random_solution =
			  uscp::random::solve(generator, problem, NULL_LOGGER); // to extend the subproblem
			dynamic_bitset<> authorized_subsets = random_solution.selected_subsets;
			authorized_subsets |= a.selected_subsets;
			authorized_subsets |= b.selected_subsets;
			return uscp::greedy::restricted_rsolve(problem, authorized_subsets, NULL_LOGGER);
		}

		[[nodiscard]] static std::string_view to_string() noexcept
		{
			return "extended_subproblem_greedy";
		}

		const uscp::problem::instance& problem;
	};
} // namespace uscp::crossover

#endif //USCP_EXTENDED_SUBPROBLEM_GREEDY_HPP
