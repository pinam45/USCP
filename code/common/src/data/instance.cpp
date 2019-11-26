//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include "common/data/instance.hpp"
#include "common/utils/logger.hpp"
#include "common/utils/format.hpp"

#include <utility>
#include <cassert>

void uscp::problem::to_json(nlohmann::json& j, const uscp::problem::instance_serial& serial)
{
	j = nlohmann::json{{"reduced", serial.reduced},
	                   {"name", serial.name},
	                   {"points_number", serial.points_number},
	                   {"subsets_number", serial.subsets_number}};
}

void uscp::problem::from_json(const nlohmann::json& j, uscp::problem::instance_serial& serial)
{
	j.at("reduced").get_to(serial.reduced);
	j.at("name").get_to(serial.name);
	j.at("points_number").get_to(serial.points_number);
	j.at("subsets_number").get_to(serial.subsets_number);
}

void uscp::problem::to_json(nlohmann::json& j, const uscp::problem::reduction_serial& serial)
{
	j = nlohmann::json{
	  {"points_number", serial.points_number},
	  {"subsets_number", serial.subsets_number},
	  {"points_covered", serial.points_covered},
	  {"subsets_dominated", serial.subsets_dominated},
	  {"subsets_included", serial.subsets_included},
	};
}

void uscp::problem::from_json(const nlohmann::json& j, uscp::problem::reduction_serial& serial)
{
	j.at("points_number").get_to(serial.points_number);
	j.at("subsets_number").get_to(serial.subsets_number);
	j.at("points_covered").get_to(serial.points_covered);
	j.at("subsets_dominated").get_to(serial.subsets_dominated);
	j.at("subsets_included").get_to(serial.subsets_included);
}

uscp::problem::reduction_serial uscp::problem::reduction::serialize() const noexcept
{
	reduction_serial serial;
	assert(subsets_dominated.size() == subsets_included.size());
	serial.points_number = points_covered.size();
	serial.subsets_number = subsets_dominated.size();

	serial.points_covered.reserve(points_covered.count());
	points_covered.iterate_bits_on([&](size_t bit_on) { serial.points_covered.push_back(bit_on); });

	serial.subsets_dominated.reserve(subsets_dominated.count());
	subsets_dominated.iterate_bits_on(
	  [&](size_t bit_on) { serial.subsets_dominated.push_back(bit_on); });

	serial.subsets_included.reserve(subsets_included.count());
	subsets_included.iterate_bits_on(
	  [&](size_t bit_on) { serial.subsets_included.push_back(bit_on); });
	return serial;
}

bool uscp::problem::reduction::load(const uscp::problem::reduction_serial& serial) noexcept
{
	points_covered.resize(serial.points_number);
	points_covered.reset();
	for(size_t point: serial.points_covered)
	{
		points_covered.set(point);
	}

	subsets_dominated.resize(serial.subsets_number);
	subsets_dominated.reset();
	for(size_t subset: serial.subsets_dominated)
	{
		subsets_dominated.set(subset);
	}

	subsets_included.resize(serial.subsets_number);
	subsets_included.reset();
	for(size_t subset: serial.subsets_included)
	{
		subsets_included.set(subset);
	}

	return true;
}

uscp::problem::reduction::reduction(size_t points_number, size_t subsets_number) noexcept
  : points_covered(points_number)
  , subsets_dominated(subsets_number)
  , subsets_included(subsets_number)
{
}

uscp::problem::reduction_info::reduction_info(
  const uscp::problem::instance* parent_instance_) noexcept
  : parent_instance(parent_instance_)
  , reduction_applied(parent_instance->points_number, parent_instance->subsets_number)
{
}

uscp::problem::instance_serial uscp::problem::instance::serialize() const noexcept
{
	assert(
	  !reduction.has_value()
	  && "do not serialize reduced instances or solutions, expand them to original instance before");
	if(reduction)
	{
		LOGGER->warn("Serialization of a reduced instance: expand solution before");
	}
	instance_serial serial;
	serial.reduced = reduction.has_value();
	serial.name = name;
	serial.points_number = points_number;
	serial.subsets_number = subsets_number;
	return serial;
}

uscp::problem::instance::instance(std::optional<reduction_info> reduction_) noexcept
  : reduction(std::move(reduction_)), name(), points_number(), subsets_number(), subsets_points()
{
}

void uscp::problem::to_json(nlohmann::json& j, const uscp::problem::instance& instance)
{
	nlohmann::json json;
	json["reduced"] = instance.reduction.has_value();
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
	json["can_reduce"] = instance.can_reduce;
	j = std::move(json);
}

std::ostream& uscp::problem::operator<<(std::ostream& os,
                                        const uscp::problem::instance_info& instance)
{
	nlohmann::json json = instance;
	os << json.dump(4);
	return os;
}
