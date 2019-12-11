//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_RANDOM_HPP
#define USCP_RANDOM_HPP

#include "common/data/instance.hpp"
#include "common/data/solution.hpp"
#include "common/utils/random.hpp"
#include "common/utils/logger.hpp"

namespace uscp::random
{
	[[nodiscard]] solution solve(random_engine& generator,
	                             const problem::instance& problem,
	                             std::shared_ptr<spdlog::logger> logger = LOGGER) noexcept;
}

#endif //USCP_RANDOM_HPP
