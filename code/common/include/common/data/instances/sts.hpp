//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_STS_HPP
#define USCP_STS_HPP

#include "common/data/instance.hpp"

#include <filesystem>

#define STS_INSTANCES_FOLDER "./resources/steiner-triple-covering/"
#define STS_ENTRY(ENTRY, file, name, points, subsets, density, cost_min, cost_max, BKS) \
	ENTRY(STS_INSTANCES_FOLDER file,                                                    \
	      name,                                                                         \
	      points,                                                                       \
	      subsets,                                                                      \
	      density,                                                                      \
	      cost_min,                                                                     \
	      cost_max,                                                                     \
	      BKS,                                                                          \
	      false,                                                                        \
	      &uscp::problem::sts::read)
#define STS_INSTANCES_TABLE_SIZE 11
#define STS_INSTANCES_TABLE(ENTRY)                                             \
	STS_ENTRY(ENTRY, "data.9", "STS9", 12, 9, 0.333f, 1, 1, 5)                 \
	STS_ENTRY(ENTRY, "data.15", "STS15", 35, 15, 0.2f, 1, 1, 9)                \
	STS_ENTRY(ENTRY, "data.27", "STS27", 117, 27, 0.111f, 1, 1, 18)            \
	STS_ENTRY(ENTRY, "data.45", "STS45", 330, 45, 0.067f, 1, 1, 30)            \
	STS_ENTRY(ENTRY, "data.81", "STS81", 1080, 81, 0.037f, 1, 1, 61)           \
	STS_ENTRY(ENTRY, "data.135", "STS135", 3015, 135, 0.022f, 1, 1, 103)       \
	STS_ENTRY(ENTRY, "data.243", "STS243", 9801, 243, 0.012f, 1, 1, 198)       \
	STS_ENTRY(ENTRY, "data.405", "STS405", 27270, 405, 0.007f, 1, 1, 335)      \
	STS_ENTRY(ENTRY, "data.729", "STS729", 88452, 729, 0.004f, 1, 1, 617)      \
	STS_ENTRY(ENTRY, "data.1215", "STS1215", 245835, 1215, 0.002f, 1, 1, 1063) \
	STS_ENTRY(ENTRY, "data.2187", "STS2187", 796797, 2187, 0.001f, 1, 1, 1963)

namespace uscp::problem::sts
{
	bool read(const std::filesystem::path& path, uscp::problem::instance& instance) noexcept;

	bool write(const uscp::problem::instance& instance,
	           const std::filesystem::path& path,
	           bool override_file = false) noexcept;
} // namespace uscp::problem::sts

#endif //USCP_STS_HPP
