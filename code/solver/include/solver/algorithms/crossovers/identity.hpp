//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_IDENTITY_HPP
#define USCP_IDENTITY_HPP

#include "common/data/instance.hpp"
#include "common/data/solution.hpp"
#include "common/utils/random.hpp"

#include <dynamic_bitset.hpp>

#include <string_view>

namespace uscp::crossover
{
	struct identity final
	{
		explicit identity(const uscp::problem::instance& problem_): problem(problem_)
		{
		}
		identity(const identity&) = default;
		identity(identity&&) noexcept = default;
		identity& operator=(const identity& other) = delete;
		identity& operator=(identity&& other) noexcept = delete;

		solution apply1(const solution& a,
		                [[maybe_unused]] const solution& b,
		                [[maybe_unused]] random_engine& generator) const noexcept
		{
			return a;
		}

		solution apply2([[maybe_unused]] const solution& a,
		                const solution& b,
		                [[maybe_unused]] random_engine& generator) const noexcept
		{
			return b;
		}

		[[nodiscard]] static std::string_view to_string() noexcept
		{
			return "identity";
		}

		const uscp::problem::instance& problem;
	};
} // namespace uscp::crossover

#endif //USCP_IDENTITY_HPP
