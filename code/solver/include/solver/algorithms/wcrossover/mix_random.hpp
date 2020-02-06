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
		mix_random(const mix_random&) = default;
		mix_random(mix_random&&) noexcept = default;
		mix_random& operator=(const mix_random& other) = delete;
		mix_random& operator=(mix_random&& other) noexcept = delete;

		[[nodiscard]] std::vector<long long> apply(const std::vector<long long>& a,
		                                           const std::vector<long long>& b,
		                                           random_engine& generator) const noexcept
		{
			std::vector<long long> weights;
			weights.reserve(problem.points_number);
			std::bernoulli_distribution dist;
			for(size_t i = 0; i < problem.points_number; ++i)
			{
				weights.push_back(dist(generator) ? a[i] : b[i]);
			}
			return weights;
		}

		std::vector<long long> apply1(const std::vector<long long>& a,
		                              const std::vector<long long>& b,
		                              random_engine& generator) const noexcept
		{
			assert(a.size() == problem.points_number && b.size() == problem.points_number);
			return apply(a, b, generator);
		}

		std::vector<long long> apply2(const std::vector<long long>& a,
		                              const std::vector<long long>& b,
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
