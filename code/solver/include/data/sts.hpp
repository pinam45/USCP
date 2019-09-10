//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_STS_HPP
#define USCP_STS_HPP

#include "data/instance.hpp"

#include <array>
#include <string_view>
#include <filesystem>

#define STS_INSTANCES_TABLE_SIZE 9
#define STS_INSTANCES_TABLE(ENTRY)                            \
	ENTRY("data.9", "STS9", 12, 9, 0.333, 1, 1, 5)            \
	ENTRY("data.15", "STS15", 35, 15, 0.2, 1, 1, 9)           \
	ENTRY("data.27", "STS27", 117, 27, 0.111, 1, 1, 18)       \
	ENTRY("data.45", "STS45", 330, 45, 0.067, 1, 1, 30)       \
	ENTRY("data.81", "STS81", 1080, 81, 0.037, 1, 1, 61)      \
	ENTRY("data.135", "STS135", 3015, 135, 0.022, 1, 1, 103)  \
	ENTRY("data.243", "STS243", 9801, 243, 0.012, 1, 1, 198)  \
	ENTRY("data.405", "STS405", 27270, 405, 0.007, 1, 1, 335) \
	ENTRY("data.729", "STS729", 88452, 729, 0.004, 1, 1, 617)
//	ENTRY("data.1215", "STS1215", 245835, 1215, 0.002, 1, 1, 1063)

#define STS_INSTANCES_FOLDER "./resources/steiner-triple-covering/"
#define _EXPAND_AS_FILE(file, name, points, subsets, density, cost_min, cost_max, BKS) \
	STS_INSTANCES_FOLDER file,
#define _EXPAND_AS_NAME(file, name, points, subsets, density, cost_min, cost_max, BKS) name,
#define _EXPAND_AS_POINTS(file, name, points, subsets, density, cost_min, cost_max, BKS) points,
#define _EXPAND_AS_SUBSETS(file, name, points, subsets, density, cost_min, cost_max, BKS) subsets,
#define _EXPAND_AS_DENSITY(file, name, points, subsets, density, cost_min, cost_max, BKS) density,
#define _EXPAND_AS_COST_MIN(file, name, points, subsets, density, cost_min, cost_max, BKS) cost_min,
#define _EXPAND_AS_COST_MAX(file, name, points, subsets, density, cost_min, cost_max, BKS) cost_max,
#define _EXPAND_AS_BKS(file, name, points, subsets, density, cost_min, cost_max, BKS) BKS,
#define _EXPAND_AS_INSTANCE_INFO(file, name, points, subsets, density, cost_min, cost_max, BKS) \
	instance_info{                                                                              \
	  STS_INSTANCES_FOLDER file, name, points, subsets, density, cost_min, cost_max, BKS},

namespace uscp::problem::sts
{
	static constexpr std::array<std::string_view, STS_INSTANCES_TABLE_SIZE> instances_names = {
	  STS_INSTANCES_TABLE(_EXPAND_AS_NAME)};
	static constexpr std::array<std::string_view, STS_INSTANCES_TABLE_SIZE> instances_files = {
	  STS_INSTANCES_TABLE(_EXPAND_AS_FILE)};
	static constexpr std::array<size_t, STS_INSTANCES_TABLE_SIZE> instances_points = {
	  STS_INSTANCES_TABLE(_EXPAND_AS_POINTS)};
	static constexpr std::array<size_t, STS_INSTANCES_TABLE_SIZE> instances_subsets = {
	  STS_INSTANCES_TABLE(_EXPAND_AS_SUBSETS)};
	static constexpr std::array<float, STS_INSTANCES_TABLE_SIZE> instances_density = {
	  STS_INSTANCES_TABLE(_EXPAND_AS_DENSITY)};
	static constexpr std::array<size_t, STS_INSTANCES_TABLE_SIZE> instances_cost_min = {
	  STS_INSTANCES_TABLE(_EXPAND_AS_COST_MIN)};
	static constexpr std::array<size_t, STS_INSTANCES_TABLE_SIZE> instances_cost_max = {
	  STS_INSTANCES_TABLE(_EXPAND_AS_COST_MAX)};
	static constexpr std::array<size_t, STS_INSTANCES_TABLE_SIZE> instances_bks = {
	  STS_INSTANCES_TABLE(_EXPAND_AS_BKS)};
	static constexpr std::array<instance_info, STS_INSTANCES_TABLE_SIZE> instances = {
	  STS_INSTANCES_TABLE(_EXPAND_AS_INSTANCE_INFO)};

	bool read(const std::filesystem::path& path, uscp::problem::instance& instance) noexcept;

	bool write(const uscp::problem::instance& instance,
	           const std::filesystem::path& path,
	           bool override_file = false) noexcept;

	bool check_instances() noexcept;
} // namespace uscp::problem::sts

#undef STS_INSTANCES_TABLE_SIZE
#undef STS_INSTANCES_TABLE
#undef STS_INSTANCES_FOLDER
#undef _EXPAND_AS_FILE
#undef _EXPAND_AS_NAME
#undef _EXPAND_AS_POINTS
#undef _EXPAND_AS_SUBSETS
#undef _EXPAND_AS_DENSITY
#undef _EXPAND_AS_COST_MIN
#undef _EXPAND_AS_COST_MAX
#undef _EXPAND_AS_BKS
#undef _EXPAND_AS_INSTANCE_INFO

#endif //USCP_STS_HPP
