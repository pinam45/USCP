//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_ORLIBRARY_HPP
#define USCP_ORLIBRARY_HPP

#include "data/instance.hpp"

#include <array>
#include <string_view>
#include <filesystem>

// BKS = Best Know Solution
// BKS is unicost BKS
#define ORLIBRARY_UNICOST_INSTANCES_TABLE_SIZE 87
#define ORLIBRARY_UNICOST_INSTANCES_TABLE(ENTRY)                       \
	ENTRY("scp41.txt", "4.1", 200, 1000, 0.02, 1, 100, 38)             \
	ENTRY("scp42.txt", "4.2", 200, 1000, 0.02, 1, 100, 37)             \
	ENTRY("scp43.txt", "4.3", 200, 1000, 0.02, 1, 100, 38)             \
	ENTRY("scp44.txt", "4.4", 200, 1000, 0.02, 1, 100, 38)             \
	ENTRY("scp45.txt", "4.5", 200, 1000, 0.02, 1, 100, 38)             \
	ENTRY("scp46.txt", "4.6", 200, 1000, 0.02, 1, 100, 37)             \
	ENTRY("scp47.txt", "4.7", 200, 1000, 0.02, 1, 100, 38)             \
	ENTRY("scp48.txt", "4.8", 200, 1000, 0.02, 1, 100, 37)             \
	ENTRY("scp49.txt", "4.9", 200, 1000, 0.02, 1, 100, 38)             \
	ENTRY("scp410.txt", "4.10", 200, 1000, 0.02, 1, 100, 38)           \
	ENTRY("scp51.txt", "5.1", 200, 2000, 0.02, 1, 100, 34)             \
	ENTRY("scp52.txt", "5.2", 200, 2000, 0.02, 1, 100, 34)             \
	ENTRY("scp53.txt", "5.3", 200, 2000, 0.02, 1, 100, 34)             \
	ENTRY("scp54.txt", "5.4", 200, 2000, 0.02, 1, 100, 34)             \
	ENTRY("scp55.txt", "5.5", 200, 2000, 0.02, 1, 100, 34)             \
	ENTRY("scp56.txt", "5.6", 200, 2000, 0.02, 1, 100, 34)             \
	ENTRY("scp57.txt", "5.7", 200, 2000, 0.02, 1, 100, 34)             \
	ENTRY("scp58.txt", "5.8", 200, 2000, 0.02, 1, 100, 34)             \
	ENTRY("scp59.txt", "5.9", 200, 2000, 0.02, 1, 100, 35)             \
	ENTRY("scp510.txt", "5.10", 200, 2000, 0.02, 1, 100, 34)           \
	ENTRY("scp61.txt", "6.1", 200, 1000, 0.05, 1, 100, 21)             \
	ENTRY("scp62.txt", "6.2", 200, 1000, 0.05, 1, 100, 20)             \
	ENTRY("scp63.txt", "6.3", 200, 1000, 0.05, 1, 100, 21)             \
	ENTRY("scp64.txt", "6.4", 200, 1000, 0.05, 1, 100, 20)             \
	ENTRY("scp65.txt", "6.5", 200, 1000, 0.05, 1, 100, 21)             \
	ENTRY("scpa1.txt", "A.1", 300, 3000, 0.02, 1, 100, 38)             \
	ENTRY("scpa2.txt", "A.2", 300, 3000, 0.02, 1, 100, 38)             \
	ENTRY("scpa3.txt", "A.3", 300, 3000, 0.02, 1, 100, 38)             \
	ENTRY("scpa4.txt", "A.4", 300, 3000, 0.02, 1, 100, 37)             \
	ENTRY("scpa5.txt", "A.5", 300, 3000, 0.02, 1, 100, 38)             \
	ENTRY("scpb1.txt", "B.1", 300, 3000, 0.05, 1, 100, 22)             \
	ENTRY("scpb2.txt", "B.2", 300, 3000, 0.05, 1, 100, 22)             \
	ENTRY("scpb3.txt", "B.3", 300, 3000, 0.05, 1, 100, 22)             \
	ENTRY("scpb4.txt", "B.4", 300, 3000, 0.05, 1, 100, 22)             \
	ENTRY("scpb5.txt", "B.5", 300, 3000, 0.05, 1, 100, 22)             \
	ENTRY("scpc1.txt", "C.1", 400, 4000, 0.02, 1, 100, 40)             \
	ENTRY("scpc2.txt", "C.2", 400, 4000, 0.02, 1, 100, 40)             \
	ENTRY("scpc3.txt", "C.3", 400, 4000, 0.02, 1, 100, 40)             \
	ENTRY("scpc4.txt", "C.4", 400, 4000, 0.02, 1, 100, 40)             \
	ENTRY("scpc5.txt", "C.5", 400, 4000, 0.02, 1, 100, 43)             \
	ENTRY("scpd1.txt", "D.1", 400, 4000, 0.05, 1, 100, 24)             \
	ENTRY("scpd2.txt", "D.2", 400, 4000, 0.05, 1, 100, 24)             \
	ENTRY("scpd3.txt", "D.3", 400, 4000, 0.05, 1, 100, 24)             \
	ENTRY("scpd4.txt", "D.4", 400, 4000, 0.05, 1, 100, 24)             \
	ENTRY("scpd5.txt", "D.5", 400, 4000, 0.05, 1, 100, 24)             \
	ENTRY("scpe1.txt", "E.1", 50, 500, 0.2, 1, 1, 5)                   \
	ENTRY("scpe2.txt", "E.2", 50, 500, 0.2, 1, 1, 5)                   \
	ENTRY("scpe3.txt", "E.3", 50, 500, 0.2, 1, 1, 5)                   \
	ENTRY("scpe4.txt", "E.4", 50, 500, 0.2, 1, 1, 5)                   \
	ENTRY("scpe5.txt", "E.5", 50, 500, 0.2, 1, 1, 5)                   \
	ENTRY("scpnre1.txt", "NRE.1", 500, 5000, 0.1, 1, 100, 16)          \
	ENTRY("scpnre2.txt", "NRE.2", 500, 5000, 0.1, 1, 100, 16)          \
	ENTRY("scpnre3.txt", "NRE.3", 500, 5000, 0.1, 1, 100, 16)          \
	ENTRY("scpnre4.txt", "NRE.4", 500, 5000, 0.1, 1, 100, 16)          \
	ENTRY("scpnre5.txt", "NRE.5", 500, 5000, 0.1, 1, 100, 16)          \
	ENTRY("scpnrf1.txt", "NRF.1", 500, 5000, 0.2, 1, 100, 10)          \
	ENTRY("scpnrf2.txt", "NRF.2", 500, 5000, 0.2, 1, 100, 10)          \
	ENTRY("scpnrf3.txt", "NRF.3", 500, 5000, 0.2, 1, 100, 10)          \
	ENTRY("scpnrf4.txt", "NRF.4", 500, 5000, 0.2, 1, 100, 10)          \
	ENTRY("scpnrf5.txt", "NRF.5", 500, 5000, 0.2, 1, 100, 10)          \
	ENTRY("scpnrg1.txt", "NRG.1", 1000, 10000, 0.02, 1, 100, 61)       \
	ENTRY("scpnrg2.txt", "NRG.2", 1000, 10000, 0.02, 1, 100, 61)       \
	ENTRY("scpnrg3.txt", "NRG.3", 1000, 10000, 0.02, 1, 100, 61)       \
	ENTRY("scpnrg4.txt", "NRG.4", 1000, 10000, 0.02, 1, 100, 61)       \
	ENTRY("scpnrg5.txt", "NRG.5", 1000, 10000, 0.02, 1, 100, 61)       \
	ENTRY("scpnrh1.txt", "NRH.1", 1000, 10000, 0.05, 1, 100, 34)       \
	ENTRY("scpnrh2.txt", "NRH.2", 1000, 10000, 0.05, 1, 100, 34)       \
	ENTRY("scpnrh3.txt", "NRH.3", 1000, 10000, 0.05, 1, 100, 34)       \
	ENTRY("scpnrh4.txt", "NRH.4", 1000, 10000, 0.05, 1, 100, 34)       \
	ENTRY("scpnrh5.txt", "NRH.5", 1000, 10000, 0.05, 1, 100, 34)       \
	ENTRY("scpclr10.txt", "CLR10", 511, 210, 0.12, 1, 1, 25)           \
	ENTRY("scpclr11.txt", "CLR11", 1023, 330, 0.12, 1, 1, 23)          \
	ENTRY("scpclr12.txt", "CLR12", 2047, 495, 0.12, 1, 1, 23)          \
	ENTRY("scpclr13.txt", "CLR13", 4095, 715, 0.12, 1, 1, 23)          \
	ENTRY("scpcyc06.txt", "CYC6", 240, 192, 0.021, 1, 1, 60)           \
	ENTRY("scpcyc07.txt", "CYC7", 672, 448, 0.009, 1, 1, 144)          \
	ENTRY("scpcyc08.txt", "CYC8", 1792, 1024, 0.004, 1, 1, 342)        \
	ENTRY("scpcyc09.txt", "CYC9", 4608, 2304, 0.002, 1, 1, 772)        \
	ENTRY("scpcyc10.txt", "CYC10", 11520, 5120, 0.001, 1, 1, 1798)     \
	ENTRY("scpcyc11.txt", "CYC11", 28160, 11264, 0.0004, 1, 1, 3968)   \
	ENTRY("rail507.txt", "RAIL507", 507, 63009, 0.013, 1, 2, 96)       \
	ENTRY("rail516.txt", "RAIL516", 516, 47311, 0.013, 1, 2, 134)      \
	ENTRY("rail582.txt", "RAIL582", 582, 55515, 0.012, 1, 2, 126)      \
	ENTRY("rail2536.txt", "RAIL2536", 2536, 1081841, 0.004, 1, 2, 378) \
	ENTRY("rail2586.txt", "RAIL2586", 2586, 920683, 0.003, 1, 2, 518)  \
	ENTRY("rail4284.txt", "RAIL4284", 4284, 1092610, 0.002, 1, 2, 594) \
	ENTRY("rail4872.txt", "RAIL4872", 4872, 968672, 0.002, 1, 2, 879)

#define ORLIBRARY_INSTANCES_FOLDER "./resources/OR-Library/"
#define _EXPAND_AS_FILE(file, name, points, subsets, density, cost_min, cost_max, BKS) \
	ORLIBRARY_INSTANCES_FOLDER file,
#define _EXPAND_AS_NAME(file, name, points, subsets, density, cost_min, cost_max, BKS) name,
#define _EXPAND_AS_POINTS(file, name, points, subsets, density, cost_min, cost_max, BKS) points,
#define _EXPAND_AS_SUBSETS(file, name, points, subsets, density, cost_min, cost_max, BKS) subsets,
#define _EXPAND_AS_DENSITY(file, name, points, subsets, density, cost_min, cost_max, BKS) density,
#define _EXPAND_AS_COST_MIN(file, name, points, subsets, density, cost_min, cost_max, BKS) cost_min,
#define _EXPAND_AS_COST_MAX(file, name, points, subsets, density, cost_min, cost_max, BKS) cost_max,
#define _EXPAND_AS_BKS(file, name, points, subsets, density, cost_min, cost_max, BKS) BKS,
#define _EXPAND_AS_INSTANCE_INFO(file, name, points, subsets, density, cost_min, cost_max, BKS) \
	instance_info{                                                                              \
	  ORLIBRARY_INSTANCES_FOLDER file, name, points, subsets, density, cost_min, cost_max, BKS},

namespace uscp::problem::orlibrary
{
	static constexpr std::array<std::string_view, ORLIBRARY_UNICOST_INSTANCES_TABLE_SIZE>
	  instances_names = {ORLIBRARY_UNICOST_INSTANCES_TABLE(_EXPAND_AS_NAME)};
	static constexpr std::array<std::string_view, ORLIBRARY_UNICOST_INSTANCES_TABLE_SIZE>
	  instances_files = {ORLIBRARY_UNICOST_INSTANCES_TABLE(_EXPAND_AS_FILE)};
	static constexpr std::array<size_t, ORLIBRARY_UNICOST_INSTANCES_TABLE_SIZE> instances_points = {
	  ORLIBRARY_UNICOST_INSTANCES_TABLE(_EXPAND_AS_POINTS)};
	static constexpr std::array<size_t, ORLIBRARY_UNICOST_INSTANCES_TABLE_SIZE> instances_subsets =
	  {ORLIBRARY_UNICOST_INSTANCES_TABLE(_EXPAND_AS_SUBSETS)};
	static constexpr std::array<float, ORLIBRARY_UNICOST_INSTANCES_TABLE_SIZE> instances_density = {
	  ORLIBRARY_UNICOST_INSTANCES_TABLE(_EXPAND_AS_DENSITY)};
	static constexpr std::array<size_t, ORLIBRARY_UNICOST_INSTANCES_TABLE_SIZE> instances_cost_min =
	  {ORLIBRARY_UNICOST_INSTANCES_TABLE(_EXPAND_AS_COST_MIN)};
	static constexpr std::array<size_t, ORLIBRARY_UNICOST_INSTANCES_TABLE_SIZE> instances_cost_max =
	  {ORLIBRARY_UNICOST_INSTANCES_TABLE(_EXPAND_AS_COST_MAX)};
	static constexpr std::array<size_t, ORLIBRARY_UNICOST_INSTANCES_TABLE_SIZE> instances_bks = {
	  ORLIBRARY_UNICOST_INSTANCES_TABLE(_EXPAND_AS_BKS)};
	static constexpr std::array<instance_info, ORLIBRARY_UNICOST_INSTANCES_TABLE_SIZE> instances = {
	  ORLIBRARY_UNICOST_INSTANCES_TABLE(_EXPAND_AS_INSTANCE_INFO)};

	bool read(const std::filesystem::path& path, uscp::problem::instance& instance) noexcept;
	bool read(const uscp::problem::instance_info& info, uscp::problem::instance& instance) noexcept;

	bool write(const uscp::problem::instance& instance,
	           const std::filesystem::path& path,
	           bool override_file = false) noexcept;

	bool check_instances() noexcept;
} // namespace uscp::problem::orlibrary

#undef ORLIBRARY_UNICOST_INSTANCES_TABLE_SIZE
#undef ORLIBRARY_UNICOST_INSTANCES_TABLE
#undef ORLIBRARY_INSTANCES_FOLDER
#undef _EXPAND_AS_FILE
#undef _EXPAND_AS_NAME
#undef _EXPAND_AS_POINTS
#undef _EXPAND_AS_SUBSETS
#undef _EXPAND_AS_DENSITY
#undef _EXPAND_AS_COST_MIN
#undef _EXPAND_AS_COST_MAX
#undef _EXPAND_AS_BKS
#undef _EXPAND_AS_INSTANCE_INFO

#endif //USCP_ORLIBRARY_HPP
