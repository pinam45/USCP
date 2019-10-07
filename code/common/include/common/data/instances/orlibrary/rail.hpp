//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_RAIL_HPP
#define USCP_RAIL_HPP

#include "common/data/instance.hpp"

#include <filesystem>

#define ORLIBRARY_RAIL_INSTANCES_FOLDER "./resources/OR-Library/"
#define ORLIBRARY_RAIL_ENTRY(ENTRY, file, name, points, subsets, density, cost_min, cost_max, BKS) \
	ENTRY(ORLIBRARY_RAIL_INSTANCES_FOLDER file,                                                    \
	      name,                                                                                    \
	      points,                                                                                  \
	      subsets,                                                                                 \
	      density,                                                                                 \
	      cost_min,                                                                                \
	      cost_max,                                                                                \
	      BKS,                                                                                     \
	      true,                                                                                    \
	      &uscp::problem::orlibrary::rail::read)
#define ORLIBRARY_RAIL_INSTANCES_TABLE_SIZE 7
#define ORLIBRARY_RAIL_INSTANCES_TABLE(ENTRY)                                                 \
	ORLIBRARY_RAIL_ENTRY(ENTRY, "rail507.txt", "RAIL507", 507, 63009, 0.013f, 1, 2, 96)       \
	ORLIBRARY_RAIL_ENTRY(ENTRY, "rail516.txt", "RAIL516", 516, 47311, 0.013f, 1, 2, 134)      \
	ORLIBRARY_RAIL_ENTRY(ENTRY, "rail582.txt", "RAIL582", 582, 55515, 0.012f, 1, 2, 126)      \
	ORLIBRARY_RAIL_ENTRY(ENTRY, "rail2536.txt", "RAIL2536", 2536, 1081841, 0.004f, 1, 2, 378) \
	ORLIBRARY_RAIL_ENTRY(ENTRY, "rail2586.txt", "RAIL2586", 2586, 920683, 0.003f, 1, 2, 518)  \
	ORLIBRARY_RAIL_ENTRY(ENTRY, "rail4284.txt", "RAIL4284", 4284, 1092610, 0.002f, 1, 2, 594) \
	ORLIBRARY_RAIL_ENTRY(ENTRY, "rail4872.txt", "RAIL4872", 4872, 968672, 0.002f, 1, 2, 879)

namespace uscp::problem::orlibrary::rail
{
	bool read(const std::filesystem::path& path, uscp::problem::instance& instance) noexcept;

	bool write(const uscp::problem::instance& instance,
	           const std::filesystem::path& path,
	           bool override_file = false) noexcept;
} // namespace uscp::problem::orlibrary::rail

#endif //USCP_RAIL_HPP
