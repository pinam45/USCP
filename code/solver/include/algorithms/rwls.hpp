//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_RWLS_HPP
#define USCP_RWLS_HPP

#include "data/instance.hpp"
#include "data/solution.hpp"
#include "utils/random.hpp"

namespace uscp::rwls
{
	[[nodiscard]] solution solve(const problem::instance& problem,
	                             random_engine& generator,
	                             size_t steps_number);
} // namespace uscp::rwls

#endif //USCP_RWLS_HPP
