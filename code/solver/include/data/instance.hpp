//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_INSTANCE_HPP
#define USCP_INSTANCE_HPP

#include "utils/random.hpp"

#include <dynamic_bitset.hpp>
#include <spdlog/fmt/ostr.h> // for operator<<
#include <nlohmann/json.hpp>

#include <filesystem>

namespace uscp::problem
{
	struct instance final
	{
		size_t points_number;
		size_t subsets_number;
		std::vector<dynamic_bitset<>> subsets_points;

		instance() = default;
		instance(const instance&) = default;
		instance(instance&&) noexcept = default;
		instance& operator=(const instance&) = default;
		instance& operator=(instance&&) noexcept = default;
	};
	void to_json(nlohmann::json& j, const instance& instance);
	std::ostream& operator<<(std::ostream& os, const instance& instance);

	bool read(const std::filesystem::path& path, instance& instance) noexcept;

	bool write(const instance& instance,
	           const std::filesystem::path& path,
	           bool override_file = false) noexcept;

	[[nodiscard]] instance generate(size_t points_number,
	                                size_t subsets_number,
	                                random_engine& generator,
	                                size_t min_covering_subsets,
	                                size_t max_covering_subsets) noexcept;

	[[nodiscard]] instance generate(size_t points_number,
	                                size_t subsets_number,
	                                random_engine& generator) noexcept;

	[[nodiscard]] bool has_solution(const instance& instance) noexcept;
} // namespace uscp::problem

#endif //USCP_INSTANCE_HPP
