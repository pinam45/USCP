//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include "common/data/solution.hpp"

#include <common/utils/logger.hpp>

#include <sstream>
#include <vector>

void uscp::to_json(nlohmann::json& j, const uscp::solution_serial& solution_serial)
{
	j = nlohmann::json{
	  {"problem", solution_serial.problem},
	  {"selected_subsets", solution_serial.selected_subsets},
	};
}

void uscp::from_json(const nlohmann::json& j, uscp::solution_serial& solution_serial)
{
	j.at("problem").get_to(solution_serial.problem);
	j.at("selected_subsets").get_to(solution_serial.selected_subsets);
}

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

uscp::solution_serial uscp::solution::serialize() const noexcept
{
	solution_serial serial;
	serial.problem = problem.serialize();
	serial.selected_subsets.reserve(selected_subsets.count());
	selected_subsets.iterate_bits_on(
	  [&](size_t bit_on) { serial.selected_subsets.push_back(bit_on); });
	return serial;
}

bool uscp::solution::load(const uscp::solution_serial& serial) noexcept
{
	if(serial.problem.name != problem.name)
	{
		LOGGER->warn(
		  "Tried to load solution for problem instance {} to solution for problem instance {}",
		  serial.problem.name,
		  problem.name);
		return false;
	}
	if(serial.problem.points_number != problem.points_number)
	{
		LOGGER->warn("Tried to load solution but problem have {} points instead of {}",
		             serial.problem.points_number,
		             problem.points_number);
		return false;
	}
	if(serial.problem.subsets_number != problem.subsets_number)
	{
		LOGGER->warn("Tried to load solution but problem have {} subsets instead of {}",
		             serial.problem.subsets_number,
		             problem.subsets_number);
		return false;
	}

	assert(selected_subsets.size() == problem.subsets_number);
	selected_subsets.reset();
	for(size_t bit_on: serial.selected_subsets)
	{
		if(bit_on >= problem.subsets_number)
		{
			LOGGER->warn(
			  "Tried to load solution with subset numbered {} for an instance with {} subsets",
			  bit_on,
			  problem.subsets_number);
			return false;
		}
		selected_subsets.set(bit_on);
	}
	compute_cover();

	return true;
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
	json["selected_subsets_number"] = solution.selected_subsets.count();
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
