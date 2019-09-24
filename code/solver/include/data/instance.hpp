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
		std::string name;
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

		bool (*read_function)(const std::filesystem::path&, uscp::problem::instance&);

		constexpr instance_info(const std::string_view& file,
		                        const std::string_view& name,
		                        size_t points,
		                        size_t subsets,
		                        float density,
		                        size_t cost_min,
		                        size_t cost_max,
		                        size_t bks,
		                        bool (*read_function)(const std::filesystem::path&, instance&));
	};
	void to_json(nlohmann::json& j, const instance_info& instance);
	std::ostream& operator<<(std::ostream& os, const instance_info& instance);

	constexpr instance_info::instance_info(const std::string_view& file_,
	                                       const std::string_view& name_,
	                                       size_t points_,
	                                       size_t subsets_,
	                                       float density_,
	                                       size_t cost_min_,
	                                       size_t cost_max_,
	                                       size_t bks_,
	                                       bool (*read_function_)(const std::filesystem::path&,
	                                                              uscp::problem::instance&))
	  : file(file_)
	  , name(name_)
	  , points(points_)
	  , subsets(subsets_)
	  , density(density_)
	  , cost_min(cost_min_)
	  , cost_max(cost_max_)
	  , bks(bks_)
	  , read_function(read_function_)
	{
	}

} // namespace uscp::problem

#endif //USCP_INSTANCE_HPP
