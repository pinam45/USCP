//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_COMMON_INSTANCES_HPP
#define USCP_COMMON_INSTANCES_HPP

#include "common/data/instances/orlibrary/orlibrary.hpp"
#include "common/data/instances/orlibrary/rail.hpp"
#include "common/data/instances/sts.hpp"
#include "common/data/instances/gvcp.hpp"

#include <array>
#include <string_view>
#include <filesystem>

#define EXPAND_AS_FILE(                                                                     \
  file, name, points, subsets, density, cost_min, cost_max, BKS, can_reduce, read_function) \
	file,
#define EXPAND_AS_NAME(                                                                     \
  file, name, points, subsets, density, cost_min, cost_max, BKS, can_reduce, read_function) \
	name,
#define EXPAND_AS_POINTS(                                                                   \
  file, name, points, subsets, density, cost_min, cost_max, BKS, can_reduce, read_function) \
	points,
#define EXPAND_AS_SUBSETS(                                                                  \
  file, name, points, subsets, density, cost_min, cost_max, BKS, can_reduce, read_function) \
	subsets,
#define EXPAND_AS_DENSITY(                                                                  \
  file, name, points, subsets, density, cost_min, cost_max, BKS, can_reduce, read_function) \
	density,
#define EXPAND_AS_COST_MIN(                                                                 \
  file, name, points, subsets, density, cost_min, cost_max, BKS, can_reduce, read_function) \
	cost_min,
#define EXPAND_AS_COST_MAX(                                                                 \
  file, name, points, subsets, density, cost_min, cost_max, BKS, can_reduce, read_function) \
	cost_max,
#define EXPAND_AS_BKS(                                                                      \
  file, name, points, subsets, density, cost_min, cost_max, BKS, can_reduce, read_function) \
	BKS,
#define EXPAND_AS_CAN_REDUCE(                                                               \
  file, name, points, subsets, density, cost_min, cost_max, BKS, can_reduce, read_function) \
	can_reduce,
#define EXPAND_AS_READ_FUNCTION(                                                            \
  file, name, points, subsets, density, cost_min, cost_max, BKS, can_reduce, read_function) \
	read_function,
#define EXPAND_AS_INSTANCE_INFO(                                                            \
  file, name, points, subsets, density, cost_min, cost_max, BKS, can_reduce, read_function) \
	instance_info{                                                                          \
	  file, name, points, subsets, density, cost_min, cost_max, BKS, can_reduce, read_function},

#define INSTANCES_TABLE_SIZE                                                       \
	ORLIBRARY_ORLIBRARY_INSTANCES_TABLE_SIZE + ORLIBRARY_RAIL_INSTANCES_TABLE_SIZE \
	  + STS_INSTANCES_TABLE_SIZE
#define INSTANCES_TABLE(ENTRY)                 \
	ORLIBRARY_ORLIBRARY_INSTANCES_TABLE(ENTRY) \
	ORLIBRARY_RAIL_INSTANCES_TABLE(ENTRY) STS_INSTANCES_TABLE(ENTRY)

namespace uscp::problem
{
	static constexpr std::array<std::string_view, INSTANCES_TABLE_SIZE> instances_names = {
	  INSTANCES_TABLE(EXPAND_AS_NAME)};
	static constexpr std::array<std::string_view, INSTANCES_TABLE_SIZE> instances_files = {
	  INSTANCES_TABLE(EXPAND_AS_FILE)};
	static constexpr std::array<size_t, INSTANCES_TABLE_SIZE> instances_points = {
	  INSTANCES_TABLE(EXPAND_AS_POINTS)};
	static constexpr std::array<size_t, INSTANCES_TABLE_SIZE> instances_subsets = {
	  INSTANCES_TABLE(EXPAND_AS_SUBSETS)};
	static constexpr std::array<float, INSTANCES_TABLE_SIZE> instances_density = {
	  INSTANCES_TABLE(EXPAND_AS_DENSITY)};
	static constexpr std::array<size_t, INSTANCES_TABLE_SIZE> instances_cost_min = {
	  INSTANCES_TABLE(EXPAND_AS_COST_MIN)};
	static constexpr std::array<size_t, INSTANCES_TABLE_SIZE> instances_cost_max = {
	  INSTANCES_TABLE(EXPAND_AS_COST_MAX)};
	static constexpr std::array<size_t, INSTANCES_TABLE_SIZE> instances_bks = {
	  INSTANCES_TABLE(EXPAND_AS_BKS)};
	static constexpr std::array<bool, INSTANCES_TABLE_SIZE> instances_can_reduce = {
	  INSTANCES_TABLE(EXPAND_AS_CAN_REDUCE)};
	static constexpr std::array<bool (*)(const std::filesystem::path&, uscp::problem::instance&),
	                            INSTANCES_TABLE_SIZE>
	  instances_read_functions = {INSTANCES_TABLE(EXPAND_AS_READ_FUNCTION)};
	static constexpr std::array<instance_info, INSTANCES_TABLE_SIZE> instances = {
	  INSTANCES_TABLE(EXPAND_AS_INSTANCE_INFO)};

	struct problem_reader
	{
		std::string_view name;
		bool (*function)(const std::filesystem::path&, uscp::problem::instance&);
	};
	static constexpr std::array<problem_reader, 4> readers = {
	  problem_reader{"orlibrary", &uscp::problem::orlibrary::orlibrary::read},
	  problem_reader{"orlibrary_rail", &uscp::problem::orlibrary::rail::read},
	  problem_reader{"sts", &uscp::problem::sts::read},
	  problem_reader{"gvcp", &uscp::problem::gvcp::read},
	};
} // namespace uscp::problem

#undef EXPAND_AS_FILE
#undef EXPAND_AS_NAME
#undef EXPAND_AS_POINTS
#undef EXPAND_AS_SUBSETS
#undef EXPAND_AS_DENSITY
#undef EXPAND_AS_COST_MIN
#undef EXPAND_AS_COST_MAX
#undef EXPAND_AS_BKS
#undef EXPAND_AS_INSTANCE_INFO

#endif //USCP_COMMON_INSTANCES_HPP
