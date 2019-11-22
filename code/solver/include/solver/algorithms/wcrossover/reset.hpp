//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_RESET_HPP
#define USCP_RESET_HPP

#include "common/data/instance.hpp"
#include "common/data/solution.hpp"
#include "common/utils/random.hpp"

#include <dynamic_bitset.hpp>

#include <string_view>

namespace uscp::wcrossover
{
	struct reset final
	{
		explicit reset(const uscp::problem::instance& problem_)
		  : problem(problem_), weights(problem_.points_number, 1)
		{
		}
		reset(const reset&) = default;
		reset(reset&&) noexcept = default;
		reset& operator=(const reset& other) = delete;
		reset& operator=(reset&& other) noexcept = delete;

		std::vector<ssize_t> apply1([[maybe_unused]] const std::vector<ssize_t>& a,
		                            [[maybe_unused]] const std::vector<ssize_t>& b,
		                            [[maybe_unused]] random_engine& generator) const noexcept
		{
			return weights;
		}

		std::vector<ssize_t> apply2([[maybe_unused]] const std::vector<ssize_t>& a,
		                            [[maybe_unused]] const std::vector<ssize_t>& b,
		                            [[maybe_unused]] random_engine& generator) const noexcept
		{
			return weights;
		}

		[[nodiscard]] static std::string_view to_string() noexcept
		{
			return "reset";
		}

		const uscp::problem::instance& problem;
		std::vector<ssize_t> weights;
	};
} // namespace uscp::wcrossover

#endif //USCP_RESET_HPP
