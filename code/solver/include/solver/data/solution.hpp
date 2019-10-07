//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_SOLUTION_HPP
#define USCP_SOLUTION_HPP

#include "common/data/solution.hpp"

namespace uscp
{
	[[nodiscard]] solution expand(const solution& reduced_solution) noexcept;
} // namespace uscp

#endif //USCP_SOLUTION_HPP
