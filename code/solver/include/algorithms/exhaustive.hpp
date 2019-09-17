//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_EXHAUSTIVE_HPP
#define USCP_EXHAUSTIVE_HPP

#include "data/instance.hpp"
#include "data/solution.hpp"

namespace uscp::exhaustive
{
	[[nodiscard]] solution solve(const problem::instance& problem);

	[[nodiscard]] solution solve_ram(const problem::instance& problem);
} // namespace uscp::exhaustive

#endif //USCP_EXHAUSTIVE_HPP
