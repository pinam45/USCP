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
	struct instance_info final
	{
		std::string_view file; // std::filesystem::path is not constexpr
		std::string_view name;
		size_t points;
		size_t subsets;
		float density;
		size_t cost_min;
		size_t cost_max;
		size_t bks;
	};
	void to_json(nlohmann::json& j, const instance_info& instance);
	std::ostream& operator<<(std::ostream& os, const instance_info& instance);

	struct instance final
	{
		const instance_info* info = nullptr;
		size_t points_number = 0;
		size_t subsets_number = 0;
		std::vector<dynamic_bitset<>> subsets_points;

		instance() = default;
		instance(const instance&) = default;
		instance(instance&&) noexcept = default;
		instance& operator=(const instance&) = default;
		instance& operator=(instance&&) noexcept = default;
	};
	void to_json(nlohmann::json& j, const instance& instance);
	std::ostream& operator<<(std::ostream& os, const instance& instance);

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
