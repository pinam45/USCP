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
#include "solver/algorithms/greedy.hpp"

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
			dynamic_bitset<> authorized_subsets = a.selected_subsets;
			authorized_subsets |= b.selected_subsets;
			return uscp::greedy::restricted_solve(problem, authorized_subsets, NULL_LOGGER);
		}

		solution apply2(const solution& a,
		                const solution& b,
		                [[maybe_unused]] random_engine& generator) const noexcept
		{
			dynamic_bitset<> authorized_subsets = a.selected_subsets;
			authorized_subsets |= b.selected_subsets;
			return uscp::greedy::restricted_rsolve(problem, authorized_subsets, NULL_LOGGER);
		}

		[[nodiscard]] static std::string_view to_string() noexcept
		{
			return "subproblem_greedy";
		}

		const uscp::problem::instance& problem;
	};
} // namespace uscp::crossover

#endif //USCP_SUBPROBLEM_GREEDY_HPP
