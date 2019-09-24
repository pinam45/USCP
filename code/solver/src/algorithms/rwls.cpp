//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include "algorithms/rwls.hpp"
#include "algorithms/greedy.hpp"
#include "utils/logger.hpp"
#include "utils/timer.hpp"

#include <vector>
#include <deque>

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

	//=================================================================================================
	// functions declarations
	//=================================================================================================

	std::vector<size_t> compute_subset_neighbors(const uscp::problem::instance& problem,
	                                             size_t subset_number) noexcept;

	int rwls_compute_subset_score(const rwls_data& data, size_t subset_number) noexcept;

	void rwls_init(rwls_data& data) noexcept;

	void rwls_add_subset(rwls_data& data, size_t subset_number) noexcept;
	void rwls_remove_subset(rwls_data& data, size_t subset_number) noexcept;

	void rwls_make_tabu(rwls_data& data, size_t subset_number) noexcept;
	bool rwls_is_tabu(const rwls_data& data, size_t subset_number) noexcept;

	size_t rwls_select_subset_no_timestamp(const rwls_data& data) noexcept;
	size_t rwls_select_subset_to_remove(const rwls_data& data) noexcept;
	size_t rwls_select_subset_to_add(const rwls_data& data, size_t point_to_cover) noexcept;
	size_t rwls_select_uncovered_point(rwls_data& data) noexcept;

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

	void rwls_init(rwls_data& data) noexcept
	{
		data.best_solution = uscp::greedy::solve(data.problem);
		data.best_solution_subset_numbers = data.best_solution.selected_subsets.count();
		data.current_solution = data.best_solution;

		// points information
		for(size_t i = 0; i < data.problem.subsets_number; ++i)
		{
			data.problem.subsets_points[i].iterate_bits_on([&](size_t bit_on) noexcept {
				data.points_information[bit_on].subsets_covering.set(i);
			});
		}
		for(size_t i = 0; i < data.problem.points_number; ++i)
		{
			data.points_information[i].subsets_covering_in_solution =
			  (data.points_information[i].subsets_covering & data.current_solution.selected_subsets)
			    .count();
		}

		// subset information
		for(size_t i = 0; i < data.problem.subsets_number; ++i)
		{
			data.subsets_information[i].score = rwls_compute_subset_score(data, i);
			assert(data.current_solution.selected_subsets[i]
			         ? data.subsets_information[i].score <= 0
			         : data.subsets_information[i].score >= 0);
			data.subsets_information[i].neighbors = compute_subset_neighbors(data.problem, i);
		}
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
		for(size_t i_neighbor: data.subsets_information[subset_number].neighbors)
		{
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
		for(size_t i_neighbor: data.subsets_information[subset_number].neighbors)
		{
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
		size_t add_subset = subsets_covering_not_selected.find_first();
		std::pair<int, int> best_score_minus_timestamp(
		  data.subsets_information[add_subset].score,
		  -data.subsets_information[add_subset].timestamp);
		subsets_covering_not_selected.iterate_bits_on([&](size_t bit_on) noexcept {
			if(!data.subsets_information[bit_on].canAddToSolution)
			{
				return;
			}
			const std::pair<int, int> current_score_timestamp(
			  data.subsets_information[bit_on].score, -data.subsets_information[bit_on].timestamp);
			if(current_score_timestamp > best_score_minus_timestamp && !rwls_is_tabu(data, bit_on))
			{
				best_score_minus_timestamp = current_score_timestamp;
				add_subset = bit_on;
			}
		});
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
} // namespace

uscp::solution uscp::rwls::solve(const uscp::problem::instance& problem,
                                 random_engine& generator,
                                 size_t steps_number)
{
	const timer timer;

	//TODO: preprocessing?

	rwls_data data(problem, generator);
	rwls_init(data);

	size_t step = 1;
	while(step <= steps_number)
	{
		while(data.uncovered_points.none())
		{
			data.best_solution = data.current_solution;
			data.best_solution_subset_numbers = data.best_solution.selected_subsets.count();
			SPDLOG_LOGGER_DEBUG(LOGGER,
			                    "RWLS new best solution with {} subsets at {}s",
			                    data.best_solution_subset_numbers,
			                    timer.elapsed());

			const size_t selected_subset = rwls_select_subset_no_timestamp(data);
			rwls_remove_subset(data, selected_subset);
		}

		// remove subset
		const size_t remove_subset = rwls_select_subset_to_remove(data);
		rwls_remove_subset(data, remove_subset);
		data.subsets_information[remove_subset].timestamp = step;

		// add subset
		const size_t selected_point = rwls_select_uncovered_point(data);
		const size_t add_subset = rwls_select_subset_to_add(data, selected_point);
		rwls_add_subset(data, add_subset);

		data.subsets_information[add_subset].timestamp = step;
		rwls_make_tabu(data, add_subset);

		// update points weights
		data.uncovered_points.iterate_bits_on([&](size_t uncovered_points_bit_on) noexcept {
			++data.points_information[uncovered_points_bit_on].weight;

			// update subsets score depending on this point weight
			if(data.points_information[uncovered_points_bit_on].subsets_covering_in_solution == 0)
			{
				// subset that can cover the point if added to solution
				data.points_information[uncovered_points_bit_on]
				  .subsets_covering.iterate_bits_on([&](size_t subsets_covering_bit_on) noexcept {
					  ++data.subsets_information[subsets_covering_bit_on].score;
				  });
			}
			else if(data.points_information[uncovered_points_bit_on].subsets_covering_in_solution
			        == 1)
			{
				// only subset in the solution covering the point
				data.points_information[uncovered_points_bit_on]
				  .subsets_covering.iterate_bits_on([&](size_t subsets_covering_bit_on) noexcept {
					  if(data.current_solution.selected_subsets[subsets_covering_bit_on])
					  {
						  ++data.subsets_information[subsets_covering_bit_on].score;
						  return false;
					  }
					  return true;
				  });
			}
		});
#if !defined(NDEBUG) && !defined(NDEBUG_SCORE)
		for(size_t i = 0; i < data.problem.subsets_number; ++i)
		{
			assert(data.subsets_information[i].score == rwls_compute_subset_score(data, i));
		}
#endif

		++step;
	}

	LOGGER->info("Found RWLS solution to {} with {} subsets in {}s",
	             problem.name,
	             data.best_solution.selected_subsets.count(),
	             timer.elapsed());

	return data.best_solution;
}
