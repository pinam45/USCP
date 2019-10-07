//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_INSTANCE_HPP
#define USCP_INSTANCE_HPP

#include <common/utils/random.hpp>

#include <dynamic_bitset.hpp>
#include <spdlog/fmt/ostr.h> // for operator<<
#include <nlohmann/json.hpp>

#include <filesystem>
#include <optional>

namespace uscp::problem
{
	struct instance_serial final
	{
		bool reduced = false;
		std::string name;
		size_t points_number = 0;
		size_t subsets_number = 0;

		instance_serial() = default;
		instance_serial(const instance_serial&) = default;
		instance_serial(instance_serial&&) noexcept = default;
		instance_serial& operator=(const instance_serial&) = default;
		instance_serial& operator=(instance_serial&&) noexcept = default;
	};
	void to_json(nlohmann::json& j, const instance_serial& serial);
	void from_json(const nlohmann::json& j, instance_serial& serial);

	struct reduction final
	{
		// relative to full solution
		dynamic_bitset<> points_covered;
		dynamic_bitset<> subsets_dominated;
		dynamic_bitset<> subsets_included;

		reduction(size_t points_number, size_t subsets_number) noexcept;
		reduction(const reduction&) = default;
		reduction(reduction&&) noexcept = default;
		reduction& operator=(const reduction&) = default;
		reduction& operator=(reduction&&) noexcept = default;
	};

	struct instance;
	struct reduction_info final
	{
		const instance* parent_instance;
		reduction reduction_applied;

		explicit reduction_info(const instance* parent_instance) noexcept;
		reduction_info(const reduction_info&) = default;
		reduction_info(reduction_info&&) noexcept = default;
		reduction_info& operator=(const reduction_info&) = default;
		reduction_info& operator=(reduction_info&&) noexcept = default;
	};

	struct instance final
	{
		std::optional<reduction_info> reduction;
		std::string name;
		size_t points_number = 0;
		size_t subsets_number = 0;
		std::vector<dynamic_bitset<>> subsets_points;

		explicit instance(std::optional<reduction_info> reduction = {}) noexcept;
		instance(const instance&) = default;
		instance(instance&&) noexcept = default;
		instance& operator=(const instance&) = default;
		instance& operator=(instance&&) noexcept = default;

		[[nodiscard]] instance_serial serialize() const noexcept;
		// no load: use name to parse instance from file
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

	[[nodiscard]] instance reduce(const instance& full_instance);

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
		bool can_reduce;

		bool (*read_function)(const std::filesystem::path&, uscp::problem::instance&);

		constexpr instance_info(const std::string_view& file,
		                        const std::string_view& name,
		                        size_t points,
		                        size_t subsets,
		                        float density,
		                        size_t cost_min,
		                        size_t cost_max,
		                        size_t bks,
		                        bool can_reduce,
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
	                                       bool can_reduce_,
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
	  , can_reduce(can_reduce_)
	  , read_function(read_function_)
	{
	}
} // namespace uscp::problem

#endif //USCP_INSTANCE_HPP
