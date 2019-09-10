//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_GREEDY_HPP
#define USCP_GREEDY_HPP

#include "data/solution.hpp"

namespace uscp::greedy
{
	[[nodiscard]] solution solve(const problem::instance& problem) noexcept;
}

#endif //USCP_GREEDY_HPP
