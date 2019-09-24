//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include "data/instance.hpp"
#include "utils/logger.hpp"
#include "utils/ostream_config_guard.hpp"
#include "utils/format.hpp"
#include "utils/timer.hpp"

void uscp::problem::to_json(nlohmann::json& j, const uscp::problem::instance& instance)
{
	nlohmann::json json;
	json["name"] = instance.name;
	json["points_number"] = instance.points_number;
	json["subsets_number"] = instance.subsets_number;
	for(size_t i = 0; i < instance.subsets_points.size(); ++i)
	{
		json["subsets_points"][i] = instance.subsets_points[i].to_string();
	}
	j = std::move(json);
}

std::ostream& uscp::problem::operator<<(std::ostream& os, const uscp::problem::instance& instance)
{
	nlohmann::json json = instance;
	os << json.dump(4);
	return os;
}

uscp::problem::instance uscp::problem::generate(std::string_view name,
                                                size_t points_number,
                                                size_t subsets_number,
                                                uscp::random_engine& generator,
                                                size_t min_covering_subsets,
                                                size_t max_covering_subsets) noexcept
{

	assert(!name.empty());
	assert(points_number > 0);
	assert(subsets_number > 0);
	assert(min_covering_subsets > 0);
	assert(max_covering_subsets > 0);
	assert(min_covering_subsets <= max_covering_subsets);
	assert(max_covering_subsets <= subsets_number);

	const timer timer;

	instance instance;
	instance.name = name;
	instance.points_number = points_number;
	instance.subsets_number = subsets_number;
	instance.subsets_points.resize(subsets_number);
	for(dynamic_bitset<>& subset: instance.subsets_points)
	{
		subset.resize(points_number);
	}

	std::uniform_int_distribution<size_t> covering_subsets_number_dist(min_covering_subsets,
	                                                                   max_covering_subsets);
	std::uniform_int_distribution<size_t> covering_subset_dist(0, subsets_number - 1);
	for(size_t i_point = 0; i_point < points_number; ++i_point)
	{
		const size_t covering_subsets_number = covering_subsets_number_dist(generator);
		for(size_t i_subset_covering = 0; i_subset_covering < covering_subsets_number;
		    ++i_subset_covering)
		{
			size_t covering_subset = covering_subset_dist(generator);
			while(instance.subsets_points[covering_subset][i_point])
			{
				covering_subset = covering_subset_dist(generator);
			}
			instance.subsets_points[covering_subset][i_point].set();
		}
	}

	LOGGER->info("successfully generated problem instance with {} points and {} subsets in {}s",
	             points_number,
	             subsets_number,
	             timer.elapsed());

	return instance;
}

uscp::problem::instance uscp::problem::generate(std::string_view name,
                                                size_t points_number,
                                                size_t subsets_number,
                                                uscp::random_engine& generator) noexcept
{
	// Balas and Ho parameters
	return generate(name, points_number, subsets_number, generator, 2, subsets_number);
}

bool uscp::problem::has_solution(const uscp::problem::instance& instance) noexcept
{
	dynamic_bitset<> cover(instance.points_number);
	for(const auto& subset_points: instance.subsets_points)
	{
		cover |= subset_points;
	}

	return cover.all();
}

void uscp::problem::to_json(nlohmann::json& j, const uscp::problem::instance_info& instance)
{
	nlohmann::json json;
	json["file"] = instance.file;
	json["name"] = instance.name;
	json["points"] = instance.points;
	json["subsets"] = instance.subsets;
	json["density"] = format(instance.density, 4);
	json["cost_min"] = instance.cost_min;
	json["cost_max"] = instance.cost_max;
	json["bks"] = instance.bks;
	j = std::move(json);
}

std::ostream& uscp::problem::operator<<(std::ostream& os,
                                        const uscp::problem::instance_info& instance)
{
	nlohmann::json json = instance;
	os << json.dump(4);
	return os;
}
