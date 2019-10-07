//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_INSTANCE_HPP
#define USCP_INSTANCE_HPP

#include "common/utils/random.hpp"
#include "common/data/instance.hpp"

#include <string_view>
#include <cstddef>

namespace uscp::problem
{
	[[nodiscard]] instance generate(std::string_view name,
	                                size_t points_number,
	                                size_t subsets_number,
	                                random_engine& generator,
	                                size_t min_covering_subsets,
	                                size_t max_covering_subsets) noexcept;

	[[nodiscard]] instance generate(std::string_view name,
	                                size_t points_number,
	                                size_t subsets_number,
	                                random_engine& generator) noexcept;

	[[nodiscard]] bool has_solution(const instance& instance) noexcept;

	[[nodiscard]] instance reduce(const instance& full_instance);
} // namespace uscp::problem

#endif //USCP_INSTANCE_HPP
