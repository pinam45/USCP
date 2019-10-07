//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_ORLIBRARY_HPP
#define USCP_ORLIBRARY_HPP

#include "common/data/instance.hpp"

#include <filesystem>

#define ORLIBRARY_ORLIBRARY_INSTANCES_FOLDER "./resources/OR-Library/"
#define ORLIBRARY_ORLIBRARY_ENTRY(                                      \
  ENTRY, file, name, points, subsets, density, cost_min, cost_max, BKS) \
	ENTRY(ORLIBRARY_ORLIBRARY_INSTANCES_FOLDER file,                    \
	      name,                                                         \
	      points,                                                       \
	      subsets,                                                      \
	      density,                                                      \
	      cost_min,                                                     \
	      cost_max,                                                     \
	      BKS,                                                          \
	      false,                                                        \
	      &uscp::problem::orlibrary::orlibrary::read)
#define ORLIBRARY_ORLIBRARY_INSTANCES_TABLE_SIZE 80
#define ORLIBRARY_ORLIBRARY_INSTANCES_TABLE(ENTRY)                                             \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scp41.txt", "4.1", 200, 1000, 0.02f, 1, 100, 38)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scp42.txt", "4.2", 200, 1000, 0.02f, 1, 100, 37)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scp43.txt", "4.3", 200, 1000, 0.02f, 1, 100, 38)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scp44.txt", "4.4", 200, 1000, 0.02f, 1, 100, 38)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scp45.txt", "4.5", 200, 1000, 0.02f, 1, 100, 38)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scp46.txt", "4.6", 200, 1000, 0.02f, 1, 100, 37)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scp47.txt", "4.7", 200, 1000, 0.02f, 1, 100, 38)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scp48.txt", "4.8", 200, 1000, 0.02f, 1, 100, 37)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scp49.txt", "4.9", 200, 1000, 0.02f, 1, 100, 38)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scp410.txt", "4.10", 200, 1000, 0.02f, 1, 100, 38)       \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scp51.txt", "5.1", 200, 2000, 0.02f, 1, 100, 34)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scp52.txt", "5.2", 200, 2000, 0.02f, 1, 100, 34)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scp53.txt", "5.3", 200, 2000, 0.02f, 1, 100, 34)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scp54.txt", "5.4", 200, 2000, 0.02f, 1, 100, 34)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scp55.txt", "5.5", 200, 2000, 0.02f, 1, 100, 34)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scp56.txt", "5.6", 200, 2000, 0.02f, 1, 100, 34)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scp57.txt", "5.7", 200, 2000, 0.02f, 1, 100, 34)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scp58.txt", "5.8", 200, 2000, 0.02f, 1, 100, 34)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scp59.txt", "5.9", 200, 2000, 0.02f, 1, 100, 35)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scp510.txt", "5.10", 200, 2000, 0.02f, 1, 100, 34)       \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scp61.txt", "6.1", 200, 1000, 0.05f, 1, 100, 21)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scp62.txt", "6.2", 200, 1000, 0.05f, 1, 100, 20)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scp63.txt", "6.3", 200, 1000, 0.05f, 1, 100, 21)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scp64.txt", "6.4", 200, 1000, 0.05f, 1, 100, 20)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scp65.txt", "6.5", 200, 1000, 0.05f, 1, 100, 21)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpa1.txt", "A.1", 300, 3000, 0.02f, 1, 100, 38)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpa2.txt", "A.2", 300, 3000, 0.02f, 1, 100, 38)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpa3.txt", "A.3", 300, 3000, 0.02f, 1, 100, 38)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpa4.txt", "A.4", 300, 3000, 0.02f, 1, 100, 37)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpa5.txt", "A.5", 300, 3000, 0.02f, 1, 100, 38)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpb1.txt", "B.1", 300, 3000, 0.05f, 1, 100, 22)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpb2.txt", "B.2", 300, 3000, 0.05f, 1, 100, 22)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpb3.txt", "B.3", 300, 3000, 0.05f, 1, 100, 22)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpb4.txt", "B.4", 300, 3000, 0.05f, 1, 100, 22)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpb5.txt", "B.5", 300, 3000, 0.05f, 1, 100, 22)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpc1.txt", "C.1", 400, 4000, 0.02f, 1, 100, 40)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpc2.txt", "C.2", 400, 4000, 0.02f, 1, 100, 40)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpc3.txt", "C.3", 400, 4000, 0.02f, 1, 100, 40)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpc4.txt", "C.4", 400, 4000, 0.02f, 1, 100, 40)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpc5.txt", "C.5", 400, 4000, 0.02f, 1, 100, 43)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpd1.txt", "D.1", 400, 4000, 0.05f, 1, 100, 24)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpd2.txt", "D.2", 400, 4000, 0.05f, 1, 100, 24)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpd3.txt", "D.3", 400, 4000, 0.05f, 1, 100, 24)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpd4.txt", "D.4", 400, 4000, 0.05f, 1, 100, 24)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpd5.txt", "D.5", 400, 4000, 0.05f, 1, 100, 24)         \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpe1.txt", "E.1", 50, 500, 0.2f, 1, 1, 5)               \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpe2.txt", "E.2", 50, 500, 0.2f, 1, 1, 5)               \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpe3.txt", "E.3", 50, 500, 0.2f, 1, 1, 5)               \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpe4.txt", "E.4", 50, 500, 0.2f, 1, 1, 5)               \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpe5.txt", "E.5", 50, 500, 0.2f, 1, 1, 5)               \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpnre1.txt", "NRE.1", 500, 5000, 0.1f, 1, 100, 16)      \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpnre2.txt", "NRE.2", 500, 5000, 0.1f, 1, 100, 16)      \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpnre3.txt", "NRE.3", 500, 5000, 0.1f, 1, 100, 16)      \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpnre4.txt", "NRE.4", 500, 5000, 0.1f, 1, 100, 16)      \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpnre5.txt", "NRE.5", 500, 5000, 0.1f, 1, 100, 16)      \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpnrf1.txt", "NRF.1", 500, 5000, 0.2f, 1, 100, 10)      \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpnrf2.txt", "NRF.2", 500, 5000, 0.2f, 1, 100, 10)      \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpnrf3.txt", "NRF.3", 500, 5000, 0.2f, 1, 100, 10)      \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpnrf4.txt", "NRF.4", 500, 5000, 0.2f, 1, 100, 10)      \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpnrf5.txt", "NRF.5", 500, 5000, 0.2f, 1, 100, 10)      \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpnrg1.txt", "NRG.1", 1000, 10000, 0.02f, 1, 100, 61)   \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpnrg2.txt", "NRG.2", 1000, 10000, 0.02f, 1, 100, 61)   \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpnrg3.txt", "NRG.3", 1000, 10000, 0.02f, 1, 100, 61)   \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpnrg4.txt", "NRG.4", 1000, 10000, 0.02f, 1, 100, 61)   \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpnrg5.txt", "NRG.5", 1000, 10000, 0.02f, 1, 100, 61)   \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpnrh1.txt", "NRH.1", 1000, 10000, 0.05f, 1, 100, 34)   \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpnrh2.txt", "NRH.2", 1000, 10000, 0.05f, 1, 100, 34)   \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpnrh3.txt", "NRH.3", 1000, 10000, 0.05f, 1, 100, 34)   \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpnrh4.txt", "NRH.4", 1000, 10000, 0.05f, 1, 100, 34)   \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpnrh5.txt", "NRH.5", 1000, 10000, 0.05f, 1, 100, 34)   \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpclr10.txt", "CLR10", 511, 210, 0.12f, 1, 1, 25)       \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpclr11.txt", "CLR11", 1023, 330, 0.12f, 1, 1, 23)      \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpclr12.txt", "CLR12", 2047, 495, 0.12f, 1, 1, 23)      \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpclr13.txt", "CLR13", 4095, 715, 0.12f, 1, 1, 23)      \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpcyc06.txt", "CYC6", 240, 192, 0.021f, 1, 1, 60)       \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpcyc07.txt", "CYC7", 672, 448, 0.009f, 1, 1, 144)      \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpcyc08.txt", "CYC8", 1792, 1024, 0.004f, 1, 1, 342)    \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpcyc09.txt", "CYC9", 4608, 2304, 0.002f, 1, 1, 772)    \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpcyc10.txt", "CYC10", 11520, 5120, 0.001f, 1, 1, 1798) \
	ORLIBRARY_ORLIBRARY_ENTRY(ENTRY, "scpcyc11.txt", "CYC11", 28160, 11264, 0.0004f, 1, 1, 3968)

// fixed CLR13 point number compared to DOI 10.1016/j.asoc.2018.10.025

namespace uscp::problem::orlibrary::orlibrary
{
	bool read(const std::filesystem::path& path, uscp::problem::instance& instance) noexcept;

	bool write(const uscp::problem::instance& instance,
	           const std::filesystem::path& path,
	           bool override_file = false) noexcept;
} // namespace uscp::problem::orlibrary::orlibrary

#endif //USCP_ORLIBRARY_HPP
