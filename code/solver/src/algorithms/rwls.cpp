//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include "solver/algorithms/rwls.hpp"
#include "solver/data/solution.hpp"
#include "common/utils/timer.hpp"
#include "common/utils/utils.hpp"

#include <cassert>
#include <algorithm>
#include <utility>

#if defined(__GNUC__)
#	define COND_LIKELY(expr) __builtin_expect(!!(expr), 1)
#	define COND_UNLIKELY(expr) __builtin_expect(!!(expr), 0)
#else
#	define COND_LIKELY(expr) (!!(expr))
#	define COND_UNLIKELY(expr) (!!(expr))
#endif

#define ensure(expr)                                                        \
	do                                                                      \
	{                                                                       \
		if(COND_UNLIKELY(!(expr)))                                          \
		{                                                                   \
			LOGGER->error("[{}:{}] failed: {}", __FILE__, __LINE__, #expr); \
			abort();                                                        \
		}                                                                   \
	} while(false)

#define NDEBUG_SCORE
#if !defined(NDEBUG) && !defined(NDEBUG_SCORE)
#	define assert_score(expr) assert(expr)
#else
#	define assert_score(expr) static_cast<void>(0)
#endif

template<bool restricted = false>
uscp::rwls::report uscp::rwls::rwls::improve_impl(
  const uscp::solution& solution,
  const std::vector<ssize_t>& points_weights_initial,
  uscp::random_engine& generator,
  uscp::rwls::position stopping_criterion,
  [[maybe_unused]] const dynamic_bitset<>& authorized_subsets) noexcept
{
	assert(points_weights_initial.size() == m_problem.points_number);
#ifndef NDEBUG
	if constexpr(restricted)
	{
		assert(authorized_subsets.size() == m_problem.subsets_number);
		/*solution.selected_subsets.iterate_bits_on([&](size_t bit_on) noexcept {
			assert(authorized_subsets.test(bit_on));
		});*/
	}
#endif

	if(!m_initialized)
	{
		initialize();
	}

	m_logger->info("({}) Start optimising by RWLS solution with {} subsets",
	               solution.problem.name,
	               solution.selected_subsets.count());

	report report(m_problem);
	report.solution_initial = solution;
	report.solution_final = solution;
	report.found_at = {0, 0};
	report.ended_at = {0, 0};
	report.stopping_criterion = stopping_criterion;
	report.points_weights_initial = points_weights_initial;

	timer timer;
	resolution_data data(report.solution_final, generator);
	init(data, points_weights_initial);
	SPDLOG_LOGGER_DEBUG(m_logger, "({}) RWLS inited in {}s", m_problem.name, timer.elapsed());

	timer.reset();
	size_t step = 0;
	while(step < stopping_criterion.steps && timer.elapsed() < stopping_criterion.time)
	{
		if(data.uncovered_points.none())
		{
			do
			{
				data.best_solution = data.current_solution;
				report.found_at.steps = step;
				report.found_at.time = timer.elapsed();
				for(size_t i = 0; i < m_problem.points_number; ++i)
				{
					report.points_weights_final[i] = data.points_information[i].weight;
				}
				SPDLOG_LOGGER_DEBUG(m_logger,
				                    "({}) RWLS new best solution with {} subsets at step {} in {}s",
				                    m_problem.name,
				                    data.best_solution.selected_subsets.count(),
				                    step,
				                    timer.elapsed());

				size_t selected_subset;
				if constexpr(restricted)
				{
					const std::optional<size_t> subset =
					  restricted_select_subset_to_remove_no_timestamp(data, authorized_subsets);
					if(!subset)
					{
						report.ended_at.steps = step;
						report.ended_at.time = timer.elapsed();
						m_logger->warn("({}) There is no authorized subset to remove: RWLS stopped",
						               m_problem.name);
						return report;
					}
					selected_subset = subset.value();
				}
				else
				{
					selected_subset = select_subset_to_remove_no_timestamp(data);
				}
				remove_subset(data, selected_subset);
			} while(data.uncovered_points.none());

			data.best_solution.compute_cover();
			assert(data.best_solution.cover_all_points);
			if(!data.best_solution.cover_all_points)
			{
				LOGGER->error("RWLS new best solution doesn't cover all points");
				abort();
			}
		}

		// remove subset
		size_t subset_to_remove;
		if constexpr(restricted)
		{
			const std::optional<size_t> subset =
			  restricted_select_subset_to_remove(data, authorized_subsets);
			if(!subset)
			{
				report.ended_at.steps = step;
				report.ended_at.time = timer.elapsed();
				m_logger->warn("({}) There is no authorized subset to remove: RWLS stopped",
				               m_problem.name);
				return report;
			}
			subset_to_remove = subset.value();
		}
		else
		{
			subset_to_remove = select_subset_to_remove(data);
		}
		remove_subset(data, subset_to_remove);
		data.subsets_information[subset_to_remove].timestamp = static_cast<ssize_t>(step);

		// add subset
		const size_t selected_point = select_uncovered_point(data);
		size_t subset_to_add;
		if constexpr(restricted)
		{
			const std::optional<size_t> subset =
			  restricted_select_subset_to_add(data, selected_point, authorized_subsets);
			if(!subset)
			{
				report.ended_at.steps = step;
				report.ended_at.time = timer.elapsed();
				m_logger->warn("({}) There is no authorized subset to add: RWLS stopped",
				               m_problem.name);
				return report;
			}
			subset_to_add = subset.value();
		}
		else
		{
			subset_to_add = select_subset_to_add(data, selected_point);
		}
		add_subset(data, subset_to_add);

		data.subsets_information[subset_to_add].timestamp = static_cast<ssize_t>(step);
		make_tabu(data, subset_to_add);

		// update points weights
		data.uncovered_points.iterate_bits_on([&](size_t uncovered_points_bit_on) noexcept {
			assert(data.points_information[uncovered_points_bit_on].subsets_covering_in_solution
			       == 0);

			++data.points_information[uncovered_points_bit_on].weight;

			// update subsets score depending on this point weight
			// subset that can cover the point if added to solution
			for(size_t subset_covering_point: m_subsets_covering_points[uncovered_points_bit_on])
			{
				++data.subsets_information[subset_covering_point].score;
			}
		});
#if !defined(NDEBUG) && !defined(NDEBUG_SCORE)
		for(size_t i = 0; i < m_problem.subsets_number; ++i)
		{
			assert(data.subsets_information[i].score == compute_subset_score(data, i));
		}
#endif

		++step;
	}
	report.ended_at.steps = step;
	report.ended_at.time = timer.elapsed();

	m_logger->info("({}) Optimised RWLS solution to {} subsets in {} steps {}s",
	               m_problem.name,
	               data.best_solution.selected_subsets.count(),
	               step,
	               timer.elapsed());

	return report;
}

uscp::rwls::position& uscp::rwls::position::operator+=(const uscp::rwls::position& other) noexcept
{
	steps += other.steps;
	time += other.time;
	return *this;
}

uscp::rwls::position_serial uscp::rwls::position::serialize() const noexcept
{
	position_serial serial;
	serial.steps = steps;
	serial.time = time;
	return serial;
}

bool uscp::rwls::position::load(const uscp::rwls::position_serial& serial) noexcept
{
	steps = serial.steps;
	time = serial.time;
	return true;
}

uscp::rwls::position uscp::rwls::operator+(const uscp::rwls::position& lhs,
                                           const uscp::rwls::position& rhs) noexcept
{
	position result = lhs;
	result += rhs;
	return result;
}

bool uscp::rwls::operator<(const uscp::rwls::position& lhs,
                           const uscp::rwls::position& rhs) noexcept
{
	return lhs.steps < rhs.steps && lhs.time < rhs.time;
}

uscp::rwls::report::report(const problem::instance& problem) noexcept
  : solution_initial(problem)
  , points_weights_initial(problem.points_number, 0)
  , solution_final(problem)
  , points_weights_final(problem.points_number, 0)
  , found_at()
  , ended_at()
  , stopping_criterion()
{
}

uscp::rwls::report_serial uscp::rwls::report::serialize() const noexcept
{
	report_serial serial;
	assert(solution_initial.problem.name == solution_final.problem.name);
	serial.solution_initial = solution_initial.serialize();
	serial.points_weights_initial = points_weights_initial;
	serial.solution_final = solution_final.serialize();
	serial.points_weights_final = points_weights_final;
	serial.found_at = found_at.serialize();
	serial.ended_at = ended_at.serialize();
	serial.stopping_criterion = stopping_criterion.serialize();
	return serial;
}

bool uscp::rwls::report::load(const uscp::rwls::report_serial& serial) noexcept
{
	if(!solution_initial.load(serial.solution_initial))
	{
		LOGGER->warn("Failed to load initial solution");
		return false;
	}
	points_weights_initial = serial.points_weights_initial;
	if(!solution_final.load(serial.solution_final))
	{
		LOGGER->warn("Failed to load final solution");
		return false;
	}
	points_weights_final = serial.points_weights_final;
	if(!found_at.load(serial.found_at))
	{
		LOGGER->warn("Failed to load found at position");
		return false;
	}
	if(!ended_at.load(serial.ended_at))
	{
		LOGGER->warn("Failed to load ended at position");
		return false;
	}
	if(!stopping_criterion.load(serial.stopping_criterion))
	{
		LOGGER->warn("Failed to load stopping criterion");
		return false;
	}

	return true;
}

uscp::rwls::rwls::rwls(const problem::instance& problem,
                       std::shared_ptr<spdlog::logger> logger) noexcept
  : m_problem(problem)
  , m_subsets_points()
  , m_subsets_covering_points()
  , m_initialized(false)
  , m_logger(std::move(logger))
{
	m_subsets_points.resize(m_problem.subsets_number);
	m_subsets_covering_points.resize(m_problem.points_number);
}

void uscp::rwls::rwls::initialize() noexcept
{
	for(size_t i = 0; i < m_problem.subsets_number; ++i)
	{
		m_problem.subsets_points[i].iterate_bits_on([&](size_t bit_on) noexcept {
			m_subsets_points[i].push_back(bit_on);
			m_subsets_covering_points[bit_on].push_back(i);
		});
	}

	m_initialized = true;
}

uscp::rwls::report uscp::rwls::rwls::improve(const uscp::solution& solution,
                                             uscp::random_engine& generator,
                                             uscp::rwls::position stopping_criterion) noexcept
{
	std::vector<ssize_t> points_initial_weights(m_problem.points_number, 1);
	return improve(solution, points_initial_weights, generator, stopping_criterion);
}

uscp::rwls::report uscp::rwls::rwls::improve(const uscp::solution& solution,
                                             const std::vector<ssize_t>& points_weights_initial,
                                             uscp::random_engine& generator,
                                             uscp::rwls::position stopping_criterion) noexcept
{
	return improve_impl<false>(
	  solution, points_weights_initial, generator, stopping_criterion, dynamic_bitset<>{});
}

uscp::rwls::report uscp::rwls::rwls::restricted_improve(
  const uscp::solution& solution,
  uscp::random_engine& generator,
  uscp::rwls::position stopping_criterion,
  const dynamic_bitset<>& authorized_subsets) noexcept
{
	std::vector<ssize_t> points_initial_weights(m_problem.points_number, 1);
	return restricted_improve(
	  solution, points_initial_weights, generator, stopping_criterion, authorized_subsets);
}

uscp::rwls::report uscp::rwls::rwls::restricted_improve(
  const uscp::solution& solution,
  const std::vector<ssize_t>& points_weights_initial,
  uscp::random_engine& generator,
  uscp::rwls::position stopping_criterion,
  const dynamic_bitset<>& authorized_subsets) noexcept
{
	return improve_impl<true>(
	  solution, points_weights_initial, generator, stopping_criterion, authorized_subsets);
}

uscp::rwls::rwls::resolution_data::resolution_data(uscp::solution& solution,
                                                   uscp::random_engine& generator_) noexcept
  : generator(generator_)
  , best_solution(solution)
  , current_solution(solution)
  , uncovered_points(solution.problem.points_number)
  , points_information()
  , subsets_information()
  , tabu_subsets()
  , subsets_tmp(solution.problem.subsets_number)
{
	points_information.resize(solution.problem.points_number);
	subsets_information.resize(solution.problem.subsets_number);
}

ssize_t uscp::rwls::rwls::compute_subset_score(const uscp::rwls::rwls::resolution_data& data,
                                               size_t subset_number) noexcept
{
	assert(subset_number < m_problem.subsets_number);

	ssize_t subset_score = 0;
	if(data.current_solution.selected_subsets[subset_number])
	{
		// if in solution, gain score for points covered only by the subset
		m_problem.subsets_points[subset_number].iterate_bits_on([&](size_t bit_on) noexcept {
			if(data.points_information[bit_on].subsets_covering_in_solution == 1)
			{
				assert(!data.uncovered_points[bit_on]);
				subset_score -= data.points_information[bit_on].weight;
			}
		});
		assert(subset_score <= 0);
	}
	else
	{
		// if out of solution, gain score for uncovered points it can cover
		m_problem.subsets_points[subset_number].iterate_bits_on([&](size_t bit_on) noexcept {
			if(data.points_information[bit_on].subsets_covering_in_solution == 0)
			{
				assert(data.uncovered_points[bit_on]);
				subset_score += data.points_information[bit_on].weight;
			}
			else
			{
				assert(!data.uncovered_points[bit_on]);
			}
		});
		assert(subset_score >= 0);
	}

	return subset_score;
}

void uscp::rwls::rwls::init(uscp::rwls::rwls::resolution_data& data,
                            const std::vector<ssize_t>& points_weights_initial) noexcept
{
	assert(points_weights_initial.size() == m_problem.points_number);

	// points information
	dynamic_bitset<> tmp;
#pragma omp parallel for default(none) shared(data, points_weights_initial) private(tmp)
	for(size_t i = 0; i < m_problem.points_number; ++i)
	{
		data.points_information[i].weight = points_weights_initial[i];
		data.points_information[i].subsets_covering_in_solution = 0;
		for(size_t subset_covering_point: m_subsets_covering_points[i])
		{
			if(data.current_solution.selected_subsets.test(subset_covering_point))
			{
				++data.points_information[i].subsets_covering_in_solution;
			}
		}
	}

// subset scores
#pragma omp parallel for default(none) shared(data)
	for(size_t i = 0; i < m_problem.subsets_number; ++i)
	{
		data.subsets_information[i].score = compute_subset_score(data, i);
		assert(data.current_solution.selected_subsets[i] ? data.subsets_information[i].score <= 0
		                                                 : data.subsets_information[i].score >= 0);
	}
}

void uscp::rwls::rwls::add_subset(uscp::rwls::rwls::resolution_data& data,
                                  size_t subset_number) noexcept
{
	assert(subset_number < m_problem.subsets_number);
	assert(!data.current_solution.selected_subsets[subset_number]);
	assert(data.subsets_information[subset_number].score >= 0);

	// add subset to solution
	data.current_solution.selected_subsets.set(subset_number);
	data.uncovered_points -= m_problem.subsets_points[subset_number];

	// compute new score
	const ssize_t new_score = -data.subsets_information[subset_number].score;

	// update subsets and points information
	for(size_t subset_point: m_subsets_points[subset_number])
	{
		++data.points_information[subset_point].subsets_covering_in_solution;
		if(data.points_information[subset_point].subsets_covering_in_solution == 1)
		{
			// point newly covered
			const ssize_t point_weight = data.points_information[subset_point].weight;
			for(size_t neighbor: m_subsets_covering_points[subset_point])
			{
				data.subsets_information[neighbor].canAddToSolution = true;

				// lost score because this point is now covered in the solution
				data.subsets_information[neighbor].score -= point_weight;
			}
		}
		else if(data.points_information[subset_point].subsets_covering_in_solution == 2)
		{
			// point now covered twice
			const ssize_t point_weight = data.points_information[subset_point].weight;
			for(size_t neighbor: m_subsets_covering_points[subset_point])
			{
				data.subsets_information[neighbor].canAddToSolution = true;

				if(neighbor != subset_number
				   && data.current_solution.selected_subsets.test(neighbor))
				{
					// lost score because it is no longer the only one to cover this point
					data.subsets_information[neighbor].score += point_weight;
					break;
				}
			}
		}
	}

	// update subset information
	data.subsets_information[subset_number].score = new_score;
	assert_score(data.subsets_information[subset_number].score
	             == compute_subset_score(data, subset_number));

#if !defined(NDEBUG) && !defined(NDEBUG_SCORE)
	for(size_t i = 0; i < m_problem.subsets_number; ++i)
	{
		assert(data.subsets_information[i].score == compute_subset_score(data, i));
	}
#endif
}

void uscp::rwls::rwls::remove_subset(uscp::rwls::rwls::resolution_data& data,
                                     size_t subset_number) noexcept
{
	assert(subset_number < m_problem.subsets_number);
	assert(data.current_solution.selected_subsets[subset_number]);
	assert(data.subsets_information[subset_number].score <= 0);

	// remove subset from solution
	data.current_solution.selected_subsets.reset(subset_number);

	// compute new score
	const ssize_t new_score = -data.subsets_information[subset_number].score;

	// update subsets and points information
	for(size_t subset_point: m_subsets_points[subset_number])
	{
		--data.points_information[subset_point].subsets_covering_in_solution;
		if(data.points_information[subset_point].subsets_covering_in_solution == 0)
		{
			// point newly uncovered
			data.uncovered_points.set(subset_point);
			const ssize_t point_weight = data.points_information[subset_point].weight;
			for(size_t neighbor: m_subsets_covering_points[subset_point])
			{
				data.subsets_information[neighbor].canAddToSolution = true;

				// gain score because this point is now uncovered in the solution
				data.subsets_information[neighbor].score += point_weight;
			}
		}
		else if(data.points_information[subset_point].subsets_covering_in_solution == 1)
		{
			// point now covered once
			const ssize_t point_weight = data.points_information[subset_point].weight;
			for(size_t neighbor: m_subsets_covering_points[subset_point])
			{
				data.subsets_information[neighbor].canAddToSolution = true;

				if(neighbor != subset_number
				   && data.current_solution.selected_subsets.test(neighbor))
				{
					// gain score because it is now the only one to cover this point in the solution
					data.subsets_information[neighbor].score -= point_weight;
					break;
				}
			}
		}
	}

	// update subset information
	data.subsets_information[subset_number].score = new_score;
	assert_score(data.subsets_information[subset_number].score
	             == compute_subset_score(data, subset_number));
	data.subsets_information[subset_number].canAddToSolution = false;

#if !defined(NDEBUG) && !defined(NDEBUG_SCORE)
	for(size_t i = 0; i < m_problem.subsets_number; ++i)
	{
		assert(data.subsets_information[i].score == compute_subset_score(data, i));
	}
#endif
}

void uscp::rwls::rwls::make_tabu(uscp::rwls::rwls::resolution_data& data,
                                 size_t subset_number) noexcept
{
	assert(subset_number < m_problem.subsets_number);
	data.tabu_subsets.push_back(subset_number);
	if(COND_LIKELY(data.tabu_subsets.size() > TABU_LIST_LENGTH))
	{
		data.tabu_subsets.pop_front();
	}
}

bool uscp::rwls::rwls::is_tabu(const uscp::rwls::rwls::resolution_data& data,
                               size_t subset_number) noexcept
{
	assert(subset_number < m_problem.subsets_number);
	return std::find(std::cbegin(data.tabu_subsets), std::cend(data.tabu_subsets), subset_number)
	       != std::cend(data.tabu_subsets);
}

size_t uscp::rwls::rwls::select_subset_to_remove_no_timestamp(
  const uscp::rwls::rwls::resolution_data& data) noexcept
{
	assert(data.current_solution.selected_subsets.any());
	size_t selected_subset = data.current_solution.selected_subsets.find_first();
	ssize_t best_score = data.subsets_information[selected_subset].score;
	data.current_solution.selected_subsets.iterate_bits_on([&](size_t bit_on) noexcept {
		if(data.subsets_information[bit_on].score > best_score)
		{
			best_score = data.subsets_information[bit_on].score;
			selected_subset = bit_on;
		}
	});
	ensure(data.current_solution.selected_subsets.test(selected_subset));
	return selected_subset;
}

std::optional<size_t> uscp::rwls::rwls::restricted_select_subset_to_remove_no_timestamp(
  uscp::rwls::rwls::resolution_data& data,
  const dynamic_bitset<>& authorized_subsets) noexcept
{
	assert(data.current_solution.selected_subsets.any());
	dynamic_bitset<>& removable_subsets = data.subsets_tmp;
	removable_subsets = authorized_subsets;
	removable_subsets &= data.current_solution.selected_subsets;
	if(removable_subsets.none())
	{
		return {};
	}
	size_t selected_subset = removable_subsets.find_first();
	ssize_t best_score = data.subsets_information[selected_subset].score;
	removable_subsets.iterate_bits_on([&](size_t bit_on) noexcept {
		if(data.subsets_information[bit_on].score > best_score)
		{
			best_score = data.subsets_information[bit_on].score;
			selected_subset = bit_on;
		}
	});
	ensure(data.current_solution.selected_subsets.test(selected_subset));
	ensure(authorized_subsets.test(selected_subset));
	return selected_subset;
}

size_t uscp::rwls::rwls::select_subset_to_remove(
  const uscp::rwls::rwls::resolution_data& data) noexcept
{
	assert(data.current_solution.selected_subsets.any());
	size_t remove_subset = data.current_solution.selected_subsets.find_first();
	std::pair<ssize_t, ssize_t> best_score_minus_timestamp(
	  data.subsets_information[remove_subset].score,
	  -data.subsets_information[remove_subset].timestamp);
	data.current_solution.selected_subsets.iterate_bits_on([&](size_t bit_on) noexcept {
		const std::pair<ssize_t, ssize_t> current_score_timestamp(
		  data.subsets_information[bit_on].score, -data.subsets_information[bit_on].timestamp);
		if(current_score_timestamp > best_score_minus_timestamp && !is_tabu(data, bit_on))
		{
			best_score_minus_timestamp = current_score_timestamp;
			remove_subset = bit_on;
		}
	});
	ensure(data.current_solution.selected_subsets.test(remove_subset));
	return remove_subset;
}

std::optional<size_t> uscp::rwls::rwls::restricted_select_subset_to_remove(
  uscp::rwls::rwls::resolution_data& data,
  const dynamic_bitset<>& authorized_subsets) noexcept
{
	assert(data.current_solution.selected_subsets.any());
	dynamic_bitset<>& removable_subsets = data.subsets_tmp;
	removable_subsets = authorized_subsets;
	removable_subsets &= data.current_solution.selected_subsets;
	if(removable_subsets.none())
	{
		return {};
	}
	size_t remove_subset = removable_subsets.find_first();
	std::pair<ssize_t, ssize_t> best_score_minus_timestamp(
	  data.subsets_information[remove_subset].score,
	  -data.subsets_information[remove_subset].timestamp);
	removable_subsets.iterate_bits_on([&](size_t bit_on) noexcept {
		const std::pair<ssize_t, ssize_t> current_score_timestamp(
		  data.subsets_information[bit_on].score, -data.subsets_information[bit_on].timestamp);
		if(current_score_timestamp > best_score_minus_timestamp && !is_tabu(data, bit_on))
		{
			best_score_minus_timestamp = current_score_timestamp;
			remove_subset = bit_on;
		}
	});
	ensure(data.current_solution.selected_subsets.test(remove_subset));
	ensure(authorized_subsets.test(remove_subset));
	return remove_subset;
}

size_t uscp::rwls::rwls::select_subset_to_add(const uscp::rwls::rwls::resolution_data& data,
                                              size_t point_to_cover) noexcept
{
	assert(point_to_cover < m_problem.points_number);
	assert(data.uncovered_points.test(point_to_cover));

	size_t add_subset = 0;
	bool add_subset_is_tabu = true;
	std::pair<ssize_t, ssize_t> best_score_minus_timestamp(std::numeric_limits<ssize_t>::min(),
	                                                       std::numeric_limits<ssize_t>::max());
	bool found = false;
	for(size_t subset_covering: m_subsets_covering_points[point_to_cover])
	{
		if(data.current_solution.selected_subsets.test(subset_covering))
		{
			continue;
		}
		if(!data.subsets_information[subset_covering].canAddToSolution)
		{
			continue;
		}

		const std::pair<ssize_t, ssize_t> current_score_minus_timestamp(
		  data.subsets_information[subset_covering].score,
		  -data.subsets_information[subset_covering].timestamp);
		if(add_subset_is_tabu)
		{
			best_score_minus_timestamp = current_score_minus_timestamp;
			add_subset = subset_covering;
			add_subset_is_tabu = is_tabu(data, add_subset);
			found = true;
			continue;
		}
		if(current_score_minus_timestamp > best_score_minus_timestamp
		   && !is_tabu(data, subset_covering))
		{
			best_score_minus_timestamp = current_score_minus_timestamp;
			add_subset = subset_covering;
		}
	}
	ensure(found);

	if(is_tabu(data, add_subset))
	{
		m_logger->warn("Selected subset is tabu");
	}
	ensure(!data.current_solution.selected_subsets.test(add_subset));
	return add_subset;
}

std::optional<size_t> uscp::rwls::rwls::restricted_select_subset_to_add(
  const uscp::rwls::rwls::resolution_data& data,
  size_t point_to_cover,
  const dynamic_bitset<>& authorized_subsets) noexcept
{
	assert(point_to_cover < m_problem.points_number);
	assert(data.uncovered_points.test(point_to_cover));
	assert(authorized_subsets.size() == m_problem.subsets_number);

	size_t add_subset = 0;
	bool add_subset_is_tabu = true;
	std::pair<ssize_t, ssize_t> best_score_minus_timestamp(std::numeric_limits<ssize_t>::min(),
	                                                       std::numeric_limits<ssize_t>::max());
	bool found = false;
	for(size_t subset_covering: m_subsets_covering_points[point_to_cover])
	{
		if(!authorized_subsets.test(subset_covering))
		{
			continue;
		}
		if(data.current_solution.selected_subsets.test(subset_covering))
		{
			continue;
		}
		if(!data.subsets_information[subset_covering].canAddToSolution)
		{
			continue;
		}

		const std::pair<ssize_t, ssize_t> current_score_minus_timestamp(
		  data.subsets_information[subset_covering].score,
		  -data.subsets_information[subset_covering].timestamp);
		if(add_subset_is_tabu)
		{
			best_score_minus_timestamp = current_score_minus_timestamp;
			add_subset = subset_covering;
			add_subset_is_tabu = is_tabu(data, add_subset);
			found = true;
			continue;
		}
		if(current_score_minus_timestamp > best_score_minus_timestamp
		   && !is_tabu(data, subset_covering))
		{
			best_score_minus_timestamp = current_score_minus_timestamp;
			add_subset = subset_covering;
		}
	}
	if(!found)
	{
		return {};
	}

	if(is_tabu(data, add_subset))
	{
		m_logger->warn("Selected subset is tabu");
	}
	ensure(!data.current_solution.selected_subsets.test(add_subset));
	ensure(authorized_subsets.test(add_subset));
	return add_subset;
}

size_t uscp::rwls::rwls::select_uncovered_point(uscp::rwls::rwls::resolution_data& data) noexcept
{
	assert(data.uncovered_points.count() > 0);
	size_t selected_point = 0;
	std::uniform_int_distribution<size_t> uncovered_point_dist(0, data.uncovered_points.count());
	const size_t selected_point_number = uncovered_point_dist(data.generator);
	size_t current_point_number = 0;
	data.uncovered_points.iterate_bits_on([&](size_t bit_on) noexcept {
		if(++current_point_number >= selected_point_number)
		{
			selected_point = bit_on;
			return false;
		}
		return true;
	});
	ensure(data.uncovered_points.test(selected_point));
	return selected_point;
}

uscp::solution uscp::rwls::improve(const uscp::solution& solution_initial,
                                   random_engine& generator,
                                   position stopping_criterion)
{
	rwls rwls(solution_initial.problem);
	rwls.initialize();
	report report = rwls.improve(solution_initial, generator, stopping_criterion);
	return report.solution_final;
}

uscp::rwls::report uscp::rwls::improve_report(const uscp::solution& solution_initial,
                                              uscp::random_engine& generator,
                                              position stopping_criterion)
{
	rwls rwls(solution_initial.problem);
	rwls.initialize();
	report report = rwls.improve(solution_initial, generator, stopping_criterion);
	return report;
}

uscp::rwls::report uscp::rwls::expand(const uscp::rwls::report& reduced_report) noexcept
{
	if(!reduced_report.solution_final.problem.reduction.has_value())
	{
		LOGGER->error("Tried to expand report of non-reduced instance");
		return reduced_report;
	}
	report expanded_report(*reduced_report.solution_final.problem.reduction->parent_instance);
	expanded_report.solution_initial = expand(reduced_report.solution_initial);
	expanded_report.points_weights_initial = expand_points_info(
	  reduced_report.solution_initial.problem, reduced_report.points_weights_initial);
	expanded_report.solution_final = expand(reduced_report.solution_final);
	expanded_report.points_weights_final = expand_points_info(reduced_report.solution_final.problem,
	                                                          reduced_report.points_weights_final);
	expanded_report.found_at = reduced_report.found_at;
	expanded_report.ended_at = reduced_report.ended_at;
	expanded_report.stopping_criterion = reduced_report.stopping_criterion;
	return expanded_report;
}
