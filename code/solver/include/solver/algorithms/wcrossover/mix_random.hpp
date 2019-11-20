//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_MIX_RANDOM_HPP
#define USCP_MIX_RANDOM_HPP

#include "common/data/instance.hpp"
#include "common/data/solution.hpp"
#include "common/utils/random.hpp"

#include <dynamic_bitset.hpp>

#include <string_view>

namespace uscp::wcrossover
{
	struct mix_random final
	{
		explicit mix_random(const uscp::problem::instance& problem_): problem(problem_)
		{
		}

		[[nodiscard]] std::vector<ssize_t> apply(const std::vector<ssize_t>& a,
		                                         const std::vector<ssize_t>& b,
		                                         random_engine& generator) const noexcept
		{
			std::vector<ssize_t> weights;
			weights.reserve(problem.points_number);
			std::bernoulli_distribution dist;
			for(size_t i = 0; i < problem.points_number; ++i)
			{
				weights.push_back(dist(generator) ? a[i] : b[i]);
			}
			return weights;
		}

		std::vector<ssize_t> apply1(const std::vector<ssize_t>& a,
		                            const std::vector<ssize_t>& b,
		                            random_engine& generator) const noexcept
		{
			assert(a.size() == problem.points_number && b.size() == problem.points_number);
			return apply(a, b, generator);
		}

		std::vector<ssize_t> apply2(const std::vector<ssize_t>& a,
		                            const std::vector<ssize_t>& b,
		                            random_engine& generator) const noexcept
		{
			assert(a.size() == problem.points_number && b.size() == problem.points_number);
			return apply(a, b, generator);
		}

		[[nodiscard]] static std::string_view to_string() noexcept
		{
			return "mix_random";
		}

		const uscp::problem::instance& problem;
	};
} // namespace uscp::wcrossover

#endif //USCP_MIX_RANDOM_HPP
