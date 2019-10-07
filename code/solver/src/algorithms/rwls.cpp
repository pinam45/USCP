//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include "solver/algorithms/rwls.hpp"
#include "solver/data/solution.hpp"
#include "common/utils/logger.hpp"
#include "common/utils/timer.hpp"

#include <vector>
#include <deque>
#include <cassert>
#include <algorithm>
#include <utility>

//#define NDEBUG_SCORE
#if !defined(NDEBUG) && !defined(NDEBUG_SCORE)
#	define assert_score(expr) assert(expr)
#else
#	define assert_score(expr) static_cast<void>(0)
#endif

namespace
{
	//=================================================================================================
	// variables
	//=================================================================================================

	constexpr size_t TABU_LIST_LENGTH = 2;

	//=================================================================================================
	// data structures
	//=================================================================================================

	struct point_information final // row
	{
		int weight = 1;
		dynamic_bitset<> subsets_covering;
		size_t subsets_covering_in_solution;
	};

	struct subset_information final // column
	{
		int score = 0;
		int timestamp = 1;
		bool canAddToSolution = true;
		std::vector<size_t> neighbors;
	};

	struct rwls_data final
	{
		uscp::random_engine& generator;
		const uscp::problem::instance& problem;
		size_t best_solution_subset_numbers; // = std::numeric_limits<size_t>::max();
		uscp::solution best_solution;
		uscp::solution current_solution;
		dynamic_bitset<> uncovered_points; // RWLS name: L
		//dynamic_bitset<> points_covered_by_only_one_subset;
		std::vector<point_information> points_information;
		std::vector<subset_information> subsets_information;
		std::deque<size_t> tabu_subsets;

		explicit rwls_data(const uscp::problem::instance& problem,
		                   uscp::random_engine& generator) noexcept;
	};

	struct rwls_solve_info
	{
		size_t steps;
		double time;
	};

	//=================================================================================================
	// functions declarations
	//=================================================================================================

	std::vector<size_t> compute_subset_neighbors(const uscp::problem::instance& problem,
	                                             size_t subset_number) noexcept;

	void rwls_compute_subsets_neighbors(rwls_data& data) noexcept;

	int rwls_compute_subset_score(const rwls_data& data, size_t subset_number) noexcept;

	void rwls_init(rwls_data& data, const uscp::solution& solution) noexcept;

	void rwls_add_subset(rwls_data& data, size_t subset_number) noexcept;
	void rwls_remove_subset(rwls_data& data, size_t subset_number) noexcept;

	void rwls_make_tabu(rwls_data& data, size_t subset_number) noexcept;
	bool rwls_is_tabu(const rwls_data& data, size_t subset_number) noexcept;

	size_t rwls_select_subset_no_timestamp(const rwls_data& data) noexcept;
	size_t rwls_select_subset_to_remove(const rwls_data& data) noexcept;
	size_t rwls_select_subset_to_add(const rwls_data& data, size_t point_to_cover) noexcept;
	size_t rwls_select_uncovered_point(rwls_data& data) noexcept;

	[[nodiscard]] uscp::solution improve_impl(const uscp::solution& solution,
	                                          uscp::random_engine& generator,
	                                          uscp::rwls::stop stopping_criterion,
	                                          uscp::rwls::stop& found_at);

	//=================================================================================================
	// implementations
	//=================================================================================================

	rwls_data::rwls_data(const uscp::problem::instance& problem_,
	                     uscp::random_engine& generator_) noexcept
	  : generator(generator_)
	  , problem(problem_)
	  , best_solution_subset_numbers(std::numeric_limits<size_t>::max())
	  , best_solution(problem)
	  , current_solution(problem)
	  , uncovered_points(problem.points_number)
	  , points_information()
	  , subsets_information()
	  , tabu_subsets()
	{
		points_information.resize(problem.points_number);
		for(point_information& point_information: points_information)
		{
			point_information.subsets_covering.resize(problem.subsets_number);
		}
		subsets_information.resize(problem.subsets_number);
	}

	std::vector<size_t> compute_subset_neighbors(const uscp::problem::instance& problem,
	                                             size_t subset_number) noexcept
	{
		std::vector<size_t> subset_neighbors;
		for(size_t i = 0; i < problem.subsets_number; ++i)
		{
			if(i == subset_number)
			{
				continue;
			}
			if((problem.subsets_points[subset_number] & problem.subsets_points[i]).any())
			{
				subset_neighbors.push_back(i);
			}
		}
		return subset_neighbors;
	}

	void rwls_compute_subsets_neighbors(rwls_data& data) noexcept
	{
		SPDLOG_LOGGER_DEBUG(LOGGER, "({}) start building subsets neighbors", data.problem.name);
		timer timer;
#pragma omp parallel for default(none) shared(data) if(data.problem.subsets_number > 8)
		for(size_t i_current_subset = 0; i_current_subset < data.problem.subsets_number;
		    ++i_current_subset)
		{
			for(size_t i_other_subset = i_current_subset + 1;
			    i_other_subset < data.problem.subsets_number;
			    ++i_other_subset)
			{
				if((data.problem.subsets_points[i_current_subset]
				    & data.problem.subsets_points[i_other_subset])
				     .any())
				{
#pragma omp critical
					{
						data.subsets_information[i_current_subset].neighbors.push_back(
						  i_other_subset);
						data.subsets_information[i_other_subset].neighbors.push_back(
						  i_current_subset);
					}
				}
			}
		}
		SPDLOG_LOGGER_DEBUG(
		  LOGGER, "({}) Built subsets neighbors in {}s", data.problem.name, timer.elapsed());
	}

	int rwls_compute_subset_score(const rwls_data& data, size_t subset_number) noexcept
	{
		assert(subset_number < data.problem.subsets_number);

		int subset_score = 0;
		if(data.current_solution.selected_subsets[subset_number])
		{
			// if in solution, gain score for points covered only by the subset
			data.problem.subsets_points[subset_number].iterate_bits_on([&](size_t bit_on) noexcept {
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
			data.problem.subsets_points[subset_number].iterate_bits_on([&](size_t bit_on) noexcept {
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

	void rwls_init(rwls_data& data, const uscp::solution& solution) noexcept
	{
		data.best_solution = solution;
		data.best_solution_subset_numbers = data.best_solution.selected_subsets.count();
		data.current_solution = data.best_solution;

		// points information
#pragma omp parallel for default(none) shared(data) if(data.problem.subsets_number > 32)
		for(size_t i = 0; i < data.problem.subsets_number; ++i)
		{
			data.problem.subsets_points[i].iterate_bits_on([&](size_t bit_on) noexcept {
				data.points_information[bit_on].subsets_covering.set(i);
			});
		}
#pragma omp parallel for default(none) shared(data) if(data.problem.points_number > 128)
		for(size_t i = 0; i < data.problem.points_number; ++i)
		{
			data.points_information[i].subsets_covering_in_solution =
			  (data.points_information[i].subsets_covering & data.current_solution.selected_subsets)
			    .count();
		}

		// subset scores
#pragma omp parallel for default(none) shared(data) if(data.problem.subsets_number > 8)
		for(size_t i = 0; i < data.problem.subsets_number; ++i)
		{
			data.subsets_information[i].score = rwls_compute_subset_score(data, i);
			assert(data.current_solution.selected_subsets[i]
			         ? data.subsets_information[i].score <= 0
			         : data.subsets_information[i].score >= 0);
			//data.subsets_information[i].neighbors = compute_subset_neighbors(data.problem, i);
		}

		// subset neighbors
		rwls_compute_subsets_neighbors(data);
	}

	void rwls_add_subset(rwls_data& data, size_t subset_number) noexcept
	{
		assert(subset_number < data.problem.subsets_number);
		assert(!data.current_solution.selected_subsets[subset_number]);
		assert(data.subsets_information[subset_number].score >= 0);

		// update points information
		dynamic_bitset<> points_newly_covered(data.problem.points_number);
		dynamic_bitset<> point_now_covered_twice(data.problem.points_number);
		data.problem.subsets_points[subset_number].iterate_bits_on([&](size_t bit_on) noexcept {
			assert(data.points_information[bit_on].subsets_covering_in_solution
			       == (data.points_information[bit_on].subsets_covering
			           & data.current_solution.selected_subsets)
			            .count());
			++data.points_information[bit_on].subsets_covering_in_solution;
			if(data.points_information[bit_on].subsets_covering_in_solution == 1)
			{
				points_newly_covered.set(bit_on);
			}
			else if(data.points_information[bit_on].subsets_covering_in_solution == 2)
			{
				point_now_covered_twice.set(bit_on);
			}
		});

		// add subset to solution
		data.current_solution.selected_subsets.set(subset_number);
		data.uncovered_points -= data.problem.subsets_points[subset_number];

		// update score
		data.subsets_information[subset_number].score =
		  -data.subsets_information[subset_number].score;
		assert_score(data.subsets_information[subset_number].score
		             == rwls_compute_subset_score(data, subset_number));

		// update neighbors
#pragma omp parallel for default(none) \
  shared(data,                         \
         subset_number,                \
         point_now_covered_twice,      \
         points_newly_covered) if(data.subsets_information[subset_number].neighbors.size() > 8)
		for(size_t i = 0; i < data.subsets_information[subset_number].neighbors.size(); ++i)
		//for(size_t i_neighbor: data.subsets_information[subset_number].neighbors)
		{
			const size_t i_neighbor = data.subsets_information[subset_number].neighbors[i];
			data.subsets_information[i_neighbor].canAddToSolution = true;
			if(data.current_solution.selected_subsets[i_neighbor])
			{
				// lost score because it is no longer the only one to cover these points
				(point_now_covered_twice & data.problem.subsets_points[i_neighbor])
				  .iterate_bits_on([&](size_t bit_on) noexcept {
					  data.subsets_information[i_neighbor].score +=
					    data.points_information[bit_on].weight;
				  });
			}
			else
			{
				// lost score because these points are now covered in the solution
				(points_newly_covered & data.problem.subsets_points[i_neighbor])
				  .iterate_bits_on([&](size_t bit_on) noexcept {
					  data.subsets_information[i_neighbor].score -=
					    data.points_information[bit_on].weight;
				  });
			}
			assert_score(data.subsets_information[i_neighbor].score
			             == rwls_compute_subset_score(data, i_neighbor));
		}
	}

	void rwls_remove_subset(rwls_data& data, size_t subset_number) noexcept
	{
		assert(subset_number < data.problem.subsets_number);
		assert(data.current_solution.selected_subsets[subset_number]);
		assert(data.subsets_information[subset_number].score <= 0);

		// update points information
		dynamic_bitset<> points_newly_ucovered(data.problem.points_number);
		dynamic_bitset<> point_now_covered_once(data.problem.points_number);
		data.problem.subsets_points[subset_number].iterate_bits_on([&](size_t bit_on) noexcept {
			assert(data.points_information[bit_on].subsets_covering_in_solution > 0);
			assert(data.points_information[bit_on].subsets_covering_in_solution
			       == (data.points_information[bit_on].subsets_covering
			           & data.current_solution.selected_subsets)
			            .count());
			--data.points_information[bit_on].subsets_covering_in_solution;
			if(data.points_information[bit_on].subsets_covering_in_solution == 0)
			{
				points_newly_ucovered.set(bit_on);
			}
			else if(data.points_information[bit_on].subsets_covering_in_solution == 1)
			{
				point_now_covered_once.set(bit_on);
			}
		});

		// remove subset from solution
		data.current_solution.selected_subsets.reset(subset_number);
		assert((data.uncovered_points & points_newly_ucovered).none());
		data.uncovered_points |= points_newly_ucovered;

		// update score
		data.subsets_information[subset_number].score =
		  -data.subsets_information[subset_number].score;
		assert_score(data.subsets_information[subset_number].score
		             == rwls_compute_subset_score(data, subset_number));

		data.subsets_information[subset_number].canAddToSolution = false;

		// update neighbors
#pragma omp parallel for default(none) \
  shared(data,                         \
         subset_number,                \
         points_newly_ucovered,        \
         point_now_covered_once) if(data.subsets_information[subset_number].neighbors.size() > 8)
		//for(size_t i_neighbor: data.subsets_information[subset_number].neighbors)
		for(size_t i = 0; i < data.subsets_information[subset_number].neighbors.size(); ++i)
		{
			const size_t i_neighbor = data.subsets_information[subset_number].neighbors[i];
			data.subsets_information[i_neighbor].canAddToSolution = true;
			if(data.current_solution.selected_subsets[i_neighbor])
			{
				// gain score because it is no the only one to cover these points
				(point_now_covered_once & data.problem.subsets_points[i_neighbor])
				  .iterate_bits_on([&](size_t bit_on) noexcept {
					  data.subsets_information[i_neighbor].score -=
					    data.points_information[bit_on].weight;
				  });
			}
			else
			{
				// gain score because these points are now uncovered in the solution
				(points_newly_ucovered & data.problem.subsets_points[i_neighbor])
				  .iterate_bits_on([&](size_t bit_on) noexcept {
					  data.subsets_information[i_neighbor].score +=
					    data.points_information[bit_on].weight;
				  });
			}

			assert_score(data.subsets_information[i_neighbor].score
			             == rwls_compute_subset_score(data, i_neighbor));
		}
	}

	void rwls_make_tabu(rwls_data& data, size_t subset_number) noexcept
	{
		assert(subset_number < data.problem.subsets_number);
		data.tabu_subsets.push_back(subset_number);
		if(data.tabu_subsets.size() > TABU_LIST_LENGTH)
		{
			data.tabu_subsets.pop_front();
		}
	}

	bool rwls_is_tabu(const rwls_data& data, size_t subset_number) noexcept
	{
		assert(subset_number < data.problem.subsets_number);
		return std::find(
		         std::cbegin(data.tabu_subsets), std::cend(data.tabu_subsets), subset_number)
		       != std::cend(data.tabu_subsets);
	}

	size_t rwls_select_subset_no_timestamp(const rwls_data& data) noexcept
	{
		assert(data.current_solution.selected_subsets.any());
		size_t selected_subset = data.current_solution.selected_subsets.find_first();
		int best_score = data.subsets_information[selected_subset].score;
		data.current_solution.selected_subsets.iterate_bits_on([&](size_t bit_on) noexcept {
			if(data.subsets_information[bit_on].score > best_score)
			{
				best_score = data.subsets_information[bit_on].score;
				selected_subset = bit_on;
			}
		});
		return selected_subset;
	}

	size_t rwls_select_subset_to_remove(const rwls_data& data) noexcept
	{
		assert(data.current_solution.selected_subsets.any());
		size_t remove_subset = data.current_solution.selected_subsets.find_first();
		std::pair<int, int> best_score_minus_timestamp(
		  data.subsets_information[remove_subset].score,
		  -data.subsets_information[remove_subset].timestamp);
		data.current_solution.selected_subsets.iterate_bits_on([&](size_t bit_on) noexcept {
			const std::pair<int, int> current_score_timestamp(
			  data.subsets_information[bit_on].score, -data.subsets_information[bit_on].timestamp);
			if(current_score_timestamp > best_score_minus_timestamp && !rwls_is_tabu(data, bit_on))
			{
				best_score_minus_timestamp = current_score_timestamp;
				remove_subset = bit_on;
			}
		});
		return remove_subset;
	}

	size_t rwls_select_subset_to_add(const rwls_data& data, size_t point_to_cover) noexcept
	{
		assert(point_to_cover < data.problem.points_number);

		const dynamic_bitset<> subsets_covering_not_selected =
		  data.points_information[point_to_cover].subsets_covering
		  & ~data.current_solution.selected_subsets;
		assert(subsets_covering_not_selected.any());
		if(subsets_covering_not_selected.none())
		{
			LOGGER->error("No subset not selected cover this point, problem not preprocessed?");
			abort();
		}
		size_t add_subset = subsets_covering_not_selected.find_first();
		bool add_subset_is_tabu = rwls_is_tabu(data, add_subset);
		std::pair<int, int> best_score_minus_timestamp(
		  data.subsets_information[add_subset].score,
		  -data.subsets_information[add_subset].timestamp);
		subsets_covering_not_selected.iterate_bits_on([&](size_t bit_on) noexcept {
			if(!data.subsets_information[bit_on].canAddToSolution)
			{
				return;
			}
			const std::pair<int, int> current_score_minus_timestamp(
			  data.subsets_information[bit_on].score, -data.subsets_information[bit_on].timestamp);
			if(add_subset_is_tabu)
			{
				best_score_minus_timestamp = current_score_minus_timestamp;
				add_subset = bit_on;
				add_subset_is_tabu = rwls_is_tabu(data, add_subset);
				return;
			}
			if(current_score_minus_timestamp > best_score_minus_timestamp
			   && !rwls_is_tabu(data, bit_on))
			{
				best_score_minus_timestamp = current_score_minus_timestamp;
				add_subset = bit_on;
			}
		});

		if(rwls_is_tabu(data, add_subset))
		{
			LOGGER->warn("Selected subset is tabu");
		}
		return add_subset;
	}

	size_t rwls_select_uncovered_point(rwls_data& data) noexcept
	{
		assert(data.uncovered_points.count() > 0);
		size_t selected_point = 0;
		std::uniform_int_distribution<size_t> uncovered_point_dist(0,
		                                                           data.uncovered_points.count());
		const size_t selected_point_number = uncovered_point_dist(data.generator);
		size_t current_point_number = 0;
		data.uncovered_points.iterate_bits_on([&](size_t bit_on) noexcept {
			if(++current_point_number > selected_point_number)
			{
				selected_point = bit_on;
				return false;
			}
			return true;
		});
		return selected_point;
	}

	uscp::solution improve_impl(const uscp::solution& solution,
	                            uscp::random_engine& generator,
	                            uscp::rwls::stop stopping_criterion,
	                            uscp::rwls::stop& found_at)
	{
		LOGGER->info("({}) Start optimising by RWLS solution with {} subsets",
		             solution.problem.name,
		             solution.selected_subsets.count());
		timer timer;
		rwls_data data(solution.problem, generator);
		rwls_init(data, solution);
		SPDLOG_LOGGER_DEBUG(LOGGER, "({}) RWLS inited in {}s", data.problem.name, timer.elapsed());

		timer.reset();
		size_t step = 0;
		found_at.steps = 0;
		found_at.time = 0;
		while(step < stopping_criterion.steps && timer.elapsed() < stopping_criterion.time)
		{
			while(data.uncovered_points.none())
			{
				data.best_solution = data.current_solution;
				data.best_solution_subset_numbers = data.best_solution.selected_subsets.count();

				found_at.steps = step;
				found_at.time = timer.elapsed();
				SPDLOG_LOGGER_DEBUG(LOGGER,
				                    "({}) RWLS new best solution with {} subsets at {}s",
				                    data.problem.name,
				                    data.best_solution_subset_numbers,
				                    timer.elapsed());

				const size_t selected_subset = rwls_select_subset_no_timestamp(data);
				rwls_remove_subset(data, selected_subset);
			}

			// remove subset
			const size_t remove_subset = rwls_select_subset_to_remove(data);
			rwls_remove_subset(data, remove_subset);
			data.subsets_information[remove_subset].timestamp = static_cast<int>(step);

			// add subset
			const size_t selected_point = rwls_select_uncovered_point(data);
			const size_t add_subset = rwls_select_subset_to_add(data, selected_point);
			rwls_add_subset(data, add_subset);

			data.subsets_information[add_subset].timestamp = static_cast<int>(step);
			rwls_make_tabu(data, add_subset);

			// update points weights
			data.uncovered_points.iterate_bits_on([&](size_t uncovered_points_bit_on) noexcept {
				assert(data.points_information[uncovered_points_bit_on].subsets_covering_in_solution
				       == 0);

				++data.points_information[uncovered_points_bit_on].weight;

				// update subsets score depending on this point weight
				// subset that can cover the point if added to solution
				data.points_information[uncovered_points_bit_on]
				  .subsets_covering.iterate_bits_on([&](size_t subsets_covering_bit_on) noexcept {
					  ++data.subsets_information[subsets_covering_bit_on].score;
				  });
			});
#if !defined(NDEBUG) && !defined(NDEBUG_SCORE)
			for(size_t i = 0; i < data.problem.subsets_number; ++i)
			{
				assert(data.subsets_information[i].score == rwls_compute_subset_score(data, i));
			}
#endif

			++step;
		}

		LOGGER->info("({}) Optimised RWLS solution to {} subsets in {} steps {}s",
		             data.problem.name,
		             data.best_solution.selected_subsets.count(),
		             step,
		             timer.elapsed());

		return data.best_solution;
	}
} // namespace

void uscp::rwls::to_json(nlohmann::json& j, const uscp::rwls::report_serial& serial)
{
	j = nlohmann::json{
	  {"solution_initial", serial.solution_initial},
	  {"solution_final", serial.solution_final},
	  {"steps", serial.steps},
	  {"time", serial.time},
	};
}

void uscp::rwls::from_json(const nlohmann::json& j, uscp::rwls::report_serial& serial)
{
	j.at("solution_initial").get_to(serial.solution_initial);
	j.at("solution_final").get_to(serial.solution_final);
	j.at("steps").get_to(serial.steps);
	j.at("time").get_to(serial.time);
}

uscp::rwls::report::report(const problem::instance& problem) noexcept
  : solution_initial(problem), solution_final(problem), found_at()
{
}

uscp::rwls::report_serial uscp::rwls::report::serialize() const noexcept
{
	report_serial serial;
	assert(solution_initial.problem.name == solution_final.problem.name);
	serial.solution_initial = solution_initial.serialize();
	serial.solution_final = solution_final.serialize();
	serial.steps = found_at.steps;
	serial.time = found_at.time;
	return serial;
}

bool uscp::rwls::report::load(const uscp::rwls::report_serial& serial) noexcept
{
	if(!solution_initial.load(serial.solution_initial))
	{
		LOGGER->warn("Failed to load initial solution");
		return false;
	}
	if(!solution_final.load(serial.solution_final))
	{
		LOGGER->warn("Failed to load final solution");
		return false;
	}
	found_at.steps = serial.steps;
	found_at.time = serial.time;

	return true;
}

uscp::solution uscp::rwls::improve(const uscp::solution& solution,
                                   random_engine& generator,
                                   stop stopping_criterion)
{
	stop stopped_at;
	return improve_impl(solution, generator, stopping_criterion, stopped_at);
}

uscp::rwls::report uscp::rwls::improve_report(const uscp::solution& solution,
                                              uscp::random_engine& generator,
                                              stop stopping_criterion)
{
	report report(solution.problem);
	report.solution_initial = solution;
	report.solution_final = improve_impl(solution, generator, stopping_criterion, report.found_at);
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
	expanded_report.solution_final = expand(reduced_report.solution_final);
	expanded_report.found_at = reduced_report.found_at;
	return expanded_report;
}
