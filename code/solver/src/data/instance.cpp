//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include "solver/data/instance.hpp"
#include "common/utils/logger.hpp"
#include "common/utils/timer.hpp"

#include <cassert>
#include <vector>

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

namespace
{
	[[gnu::hot]] bool reduce_domination_iterate(uscp::problem::reduction_info& reduction);

	[[gnu::hot]] bool reduce_domination(uscp::problem::reduction_info& reduction);

	[[gnu::hot]] bool reduce_inclusion(const std::vector<dynamic_bitset<>>& points_subsets,
	                                   uscp::problem::reduction_info& reduction);

	bool reduce_domination_iterate(uscp::problem::reduction_info& reduction)
	{
		timer timer;
		bool reduced = false;
		dynamic_bitset<> remaining_subsets = ~(reduction.reduction_applied.subsets_dominated
		                                       | reduction.reduction_applied.subsets_included);
		remaining_subsets.iterate_bits_on([&](size_t current_subset_bit_on) {
			dynamic_bitset<> extended_subset =
			  (reduction.parent_instance->subsets_points[current_subset_bit_on]
			   | reduction.reduction_applied.points_covered);
			remaining_subsets.iterate_bits_on([&](size_t other_subset_bit_on) {
				if(reduction.parent_instance->subsets_points[other_subset_bit_on].is_subset_of(
				     extended_subset))
				{
					reduction.reduction_applied.subsets_dominated.set(other_subset_bit_on);
					remaining_subsets.reset(other_subset_bit_on);
					reduced = true;
				}
			});
		});
		SPDLOG_LOGGER_DEBUG(LOGGER,
		                    "({}) Applied domination reduction in {}s",
		                    reduction.parent_instance->name,
		                    timer.elapsed());
		return reduced;
	}

	bool reduce_domination(uscp::problem::reduction_info& reduction)
	{
		timer timer;
		bool reduced = false;
		dynamic_bitset<> ignored_subsets = reduction.reduction_applied.subsets_dominated
		                                   | reduction.reduction_applied.subsets_included;
		dynamic_bitset<> extended_subset; // to minimize memory allocations
#pragma omp parallel for default(none) shared(reduction, ignored_subsets, reduced) private( \
  extended_subset) if(reduction.parent_instance->subsets_number > 128)
		for(size_t i_current_subset = 0;
		    i_current_subset < reduction.parent_instance->subsets_number;
		    ++i_current_subset)
		{
			if(ignored_subsets[i_current_subset])
			{
				// possible concurrent access when a dominated subset is added
				// not a problem: it will just check if a dominated subset dominate other subsets and domination is transitive
				continue;
			}
			extended_subset = reduction.parent_instance->subsets_points[i_current_subset];
			extended_subset |= reduction.reduction_applied.points_covered;
			for(size_t i_other_subset = 0;
			    i_other_subset < reduction.parent_instance->subsets_number;
			    ++i_other_subset)
			{
				if(i_other_subset == i_current_subset)
				{
					continue;
				}
				if(ignored_subsets[i_other_subset])
				{
					// possible concurrent access when a dominated subset is added
					// not a problem: it will just check if an already dominated subset is dominated
					continue;
				}
				if(reduction.parent_instance->subsets_points[i_other_subset].is_subset_of(
				     extended_subset))
				{
					reduction.reduction_applied.subsets_dominated.set(i_other_subset);
					ignored_subsets.set(i_other_subset);
					reduced = true;
				}
			}
		}
		SPDLOG_LOGGER_DEBUG(LOGGER,
		                    "({}) Computed domination reduction in {}s",
		                    reduction.parent_instance->name,
		                    timer.elapsed());
		return reduced;
	}

	bool reduce_inclusion(const std::vector<dynamic_bitset<>>& points_subsets,
	                      uscp::problem::reduction_info& reduction)
	{
		timer timer;
		bool reduced = false;
		dynamic_bitset<> points_remaining_subsets;
		for(size_t i_point = 0; i_point < reduction.parent_instance->points_number; ++i_point)
		{
			if(reduction.reduction_applied.points_covered[i_point])
			{
				continue;
			}
			points_remaining_subsets = points_subsets[i_point];
			points_remaining_subsets -= reduction.reduction_applied.subsets_dominated;
			assert(points_remaining_subsets.any());
			if(points_remaining_subsets.count() == 1)
			{
				const size_t only_subset_covering_point = points_remaining_subsets.find_first();
				reduction.reduction_applied.subsets_included.set(only_subset_covering_point);
				reduction.parent_instance->subsets_points[only_subset_covering_point]
				  .iterate_bits_on([&](size_t bit_on) noexcept {
					  reduction.reduction_applied.points_covered.set(bit_on);
				  });
				reduced = true;
			}
		}
		SPDLOG_LOGGER_DEBUG(LOGGER,
		                    "({}) Computed inclusion reduction in {}s",
		                    reduction.parent_instance->name,
		                    timer.elapsed());
		return reduced;
	}
} // namespace

uscp::problem::instance uscp::problem::reduce(const uscp::problem::instance& full_instance)
{
	LOGGER->info("({}) Start reducing instance", full_instance.name);
	timer partial_timer;
	const timer timer;

	assert(!full_instance.reduction.has_value());
	if(full_instance.reduction)
	{
		LOGGER->warn("Tried to reduce instance already reduced");
		return full_instance;
	}

	// Generate flipped instance matrix
	std::vector<dynamic_bitset<>> points_subsets;
	points_subsets.resize(full_instance.points_number);
	for(dynamic_bitset<>& point_subsets: points_subsets)
	{
		point_subsets.resize(full_instance.subsets_number);
	}
	for(size_t i_subset = 0; i_subset < full_instance.subsets_number; ++i_subset)
	{
		full_instance.subsets_points[i_subset].iterate_bits_on([&](size_t point_bit_on) noexcept {
			points_subsets[point_bit_on].set(i_subset);
		});
	}
	SPDLOG_LOGGER_DEBUG(LOGGER,
	                    "({}) Generated flipped instance matrix in {}s",
	                    full_instance.name,
	                    partial_timer.elapsed());

	// Compute reduction
	partial_timer.reset();
	reduction_info reduction(&full_instance);
	reduce_domination(reduction);
	if(reduce_inclusion(points_subsets, reduction))
	{
		while(reduce_domination(reduction) && reduce_inclusion(points_subsets, reduction))
			;
	}
	if((reduction.reduction_applied.subsets_included
	    & reduction.reduction_applied.subsets_dominated)
	     .any())
	{
		LOGGER->error("Reduction generated subset dominated and included at the same time");
		abort();
	}
	SPDLOG_LOGGER_DEBUG(
	  LOGGER, "({}) Computed full reduction in {}s", full_instance.name, partial_timer.elapsed());

	// Apply reduction
	partial_timer.reset();
	instance reduced_instance(reduction);
	reduced_instance.name = full_instance.name;
	reduced_instance.points_number =
	  full_instance.points_number - reduction.reduction_applied.points_covered.count();
	reduced_instance.subsets_number = full_instance.subsets_number
	                                  - reduction.reduction_applied.subsets_dominated.count()
	                                  - reduction.reduction_applied.subsets_included.count();

	dynamic_bitset<> removed_subsets =
	  reduction.reduction_applied.subsets_dominated | reduction.reduction_applied.subsets_included;
	reduced_instance.subsets_points.resize(reduced_instance.subsets_number);
	size_t i_subset_full_instance = 0;
	while(removed_subsets[i_subset_full_instance])
	{
		++i_subset_full_instance;
	}
	for(size_t i_subset = 0; i_subset < reduced_instance.subsets_number; ++i_subset)
	{
		assert(i_subset_full_instance < full_instance.subsets_number);
		reduced_instance.subsets_points[i_subset].resize(reduced_instance.points_number);
		size_t i_point_full_instance = 0;
		while(reduction.reduction_applied.points_covered[i_point_full_instance])
		{
			++i_point_full_instance;
		}
		for(size_t i_point = 0; i_point < reduced_instance.points_number; ++i_point)
		{
			assert(i_point_full_instance < full_instance.points_number);
			reduced_instance.subsets_points[i_subset][i_point] =
			  full_instance.subsets_points[i_subset_full_instance][i_point_full_instance];
			do
			{
				++i_point_full_instance;
			} while(i_point_full_instance < full_instance.points_number
			        && reduction.reduction_applied.points_covered[i_point_full_instance]);
		}
		assert(i_point_full_instance == full_instance.points_number);
		if(i_point_full_instance != full_instance.points_number)
		{
			LOGGER->error("Solution reduction failed, only {}/{} points for subset {}",
			              i_subset_full_instance,
			              full_instance.subsets_number,
			              i_subset_full_instance);
			abort();
		}
		do
		{
			++i_subset_full_instance;
		} while(i_subset_full_instance < full_instance.subsets_number
		        && removed_subsets[i_subset_full_instance]);
	}
	assert(i_subset_full_instance == full_instance.subsets_number);
	if(i_subset_full_instance != full_instance.subsets_number)
	{
		LOGGER->error("Solution reduction failed, only {}/{} subsets",
		              i_subset_full_instance,
		              full_instance.subsets_number);
		abort();
	}
	SPDLOG_LOGGER_DEBUG(
	  LOGGER, "({}) Applied reduction in {}s", full_instance.name, partial_timer.elapsed());

	reduced_instance.name += " reduced";
	LOGGER->info("({}) Reduced instance from {} subsets {} points to {} subsets {} points in {}s",
	             full_instance.name,
	             full_instance.subsets_number,
	             full_instance.points_number,
	             reduced_instance.subsets_number,
	             reduced_instance.points_number,
	             timer.elapsed());
	return reduced_instance;
}
