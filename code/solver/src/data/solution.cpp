//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include "data/solution.hpp"

#include <sstream>
#include <vector>

uscp::solution::solution(const uscp::problem::instance& problem_)
  : problem(problem_)
  , selected_subsets(problem_.subsets_number)
  , covered_points(problem_.points_number)
  , cover_all_points(false)
{
}

uscp::solution& uscp::solution::operator=(const uscp::solution& other)
{
	assert(&problem == &other.problem);
	selected_subsets = other.selected_subsets;
	covered_points = other.covered_points;
	cover_all_points = other.cover_all_points;
	return *this;
}

uscp::solution& uscp::solution::operator=(uscp::solution&& other) noexcept
{
	assert(&problem == &other.problem);
	selected_subsets = std::move(other.selected_subsets);
	covered_points = std::move(other.covered_points);
	cover_all_points = other.cover_all_points;
	return *this;
}

void uscp::solution::compute_cover() noexcept
{
	assert(selected_subsets.size() == problem.subsets_number);
	assert(covered_points.size() == problem.points_number);

	covered_points.reset();
	size_t selected_subset = selected_subsets.find_first();
	while(selected_subset != dynamic_bitset<>::npos)
	{
		covered_points |= problem.subsets_points[selected_subset];
		selected_subset = selected_subsets.find_next(selected_subset);
	}
	cover_all_points = covered_points.all();
}

void uscp::to_json(nlohmann::json& j, const uscp::solution& solution)
{
	nlohmann::json json;
	std::ostringstream problem_address;
	problem_address << static_cast<const void*>(&solution.problem);
	json["problem"] = problem_address.str();

	std::vector<size_t> selected_subsets_numbers;
	size_t selected_subset = solution.selected_subsets.find_first();
	while(selected_subset != dynamic_bitset<>::npos)
	{
		selected_subsets_numbers.push_back(selected_subset);
		selected_subset = solution.selected_subsets.find_next(selected_subset);
	}
	json["selected_subsets_numbers"] = std::move(selected_subsets_numbers);

	json["selected_subsets"] = solution.selected_subsets.to_string();
	json["covered_points"] = solution.covered_points.to_string();
	json["cover_all_points"] = solution.cover_all_points;
	j = std::move(json);
}

std::ostream& uscp::operator<<(std::ostream& os, const uscp::solution& solution)
{
	nlohmann::json json = solution;
	os << json.dump(4);
	return os;
}
