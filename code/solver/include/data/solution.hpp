//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_SOLUTION_HPP
#define USCP_SOLUTION_HPP

#include "data/instance.hpp"

#include <dynamic_bitset.hpp>
#include <spdlog/fmt/ostr.h> // for operator<<
#include <nlohmann/json.hpp>

namespace uscp
{
	struct solution final
	{
		const problem::instance& problem;
		dynamic_bitset<> selected_subsets;
		dynamic_bitset<> covered_points;
		bool cover_all_points;

		explicit solution(const problem::instance& problem);
		solution(const solution&) = default;
		solution(solution&&) noexcept = default;
		solution& operator=(const solution& other);
		solution& operator=(solution&& other) noexcept;

		void compute_cover() noexcept;
	};
	void to_json(nlohmann::json& j, const solution& solution);
	std::ostream& operator<<(std::ostream& os, const solution& solution);
} // namespace uscp

#endif //USCP_SOLUTION_HPP
