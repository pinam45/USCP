//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include "solver/algorithms/greedy.hpp"
#include "common/utils/logger.hpp"
#include "common/utils/timer.hpp"
#include "common/data/instance.hpp"
#include "solver/data/solution.hpp"

#include <dynamic_bitset.hpp>

namespace
{
	template<bool restricted>
	[[nodiscard, gnu::hot]] uscp::greedy::report solve_report_impl(
	  const uscp::problem::instance& problem,
	  [[maybe_unused]] const dynamic_bitset<>& authorized_subsets) noexcept
	{
		if constexpr(restricted)
		{
			assert(authorized_subsets.size() == problem.subsets_number);
		}
		SPDLOG_LOGGER_DEBUG(LOGGER, "({}) Start building greedy solution", problem.name);
		const timer timer;

		uscp::greedy::report report(problem);
		while(!report.solution_final.cover_all_points)
		{
			size_t max_subset_number =
			  report.solution_final.selected_subsets.size(); //invalid initial value
			dynamic_bitset<> covered_points_with_max_subset(problem.points_number);
			size_t covered_points_number_with_max_subset =
			  report.solution_final.covered_points.count();
			dynamic_bitset<> new_covered_points;
			for(size_t i = 0; i < problem.subsets_number; ++i)
			{
				if constexpr(restricted)
				{
					if(!authorized_subsets.test(i))
					{
						continue;
					}
				}
				if(report.solution_final.selected_subsets[i])
				{
					// already selected
					continue;
				}

				new_covered_points = report.solution_final.covered_points;
				new_covered_points |= problem.subsets_points[i];
				const size_t new_covered_points_number = new_covered_points.count();
				if(new_covered_points_number > covered_points_number_with_max_subset)
				{
					max_subset_number = i;
					covered_points_with_max_subset = new_covered_points;
					covered_points_number_with_max_subset = new_covered_points_number;
				}
			}

			// all subset already included or no subset add covered points
			if(max_subset_number == report.solution_final.selected_subsets.size())
			{
				LOGGER->error("The problem has no solution");
				abort();
			}

			// update solution (faster)
			report.solution_final.selected_subsets[max_subset_number].set();
			report.solution_final.covered_points = covered_points_with_max_subset;
			report.solution_final.cover_all_points = report.solution_final.covered_points.all();

			// update solution (slower)
			//solution.compute_cover();
			//solution.compute_cost();
		}

		report.time = timer.elapsed();
		SPDLOG_LOGGER_DEBUG(LOGGER,
		                    "({}) Built greedy solution with {} subsets in {}s",
		                    problem.name,
		                    report.solution_final.selected_subsets.count(),
		                    report.time);

		return report;
	}
} // namespace

uscp::greedy::report::report(const uscp::problem::instance& problem) noexcept
  : solution_final(problem), time()
{
}

uscp::greedy::report_serial uscp::greedy::report::serialize() const noexcept
{
	report_serial serial;
	serial.solution_final = solution_final.serialize();
	serial.time = time;
	return serial;
}

bool uscp::greedy::report::load(const uscp::greedy::report_serial& serial) noexcept
{
	if(!solution_final.load(serial.solution_final))
	{
		LOGGER->warn("Failed to load solution");
		return false;
	}
	time = serial.time;

	return true;
}

uscp::solution uscp::greedy::solve(const uscp::problem::instance& problem) noexcept
{
	return solve_report(problem).solution_final;
}

uscp::greedy::report uscp::greedy::solve_report(const uscp::problem::instance& problem) noexcept
{
	return solve_report_impl<false>(problem, dynamic_bitset<>{});
}

uscp::solution uscp::greedy::restricted_solve(const uscp::problem::instance& problem,
                                              const dynamic_bitset<>& authorized_subsets) noexcept
{
	return restricted_solve_report(problem, authorized_subsets).solution_final;
}

uscp::greedy::report uscp::greedy::restricted_solve_report(
  const uscp::problem::instance& problem,
  const dynamic_bitset<>& authorized_subsets) noexcept
{
	return solve_report_impl<true>(problem, authorized_subsets);
}

uscp::greedy::report uscp::greedy::expand(const uscp::greedy::report& reduced_report) noexcept
{
	if(!reduced_report.solution_final.problem.reduction.has_value())
	{
		LOGGER->error("Tried to expand report of non-reduced instance");
		return reduced_report;
	}
	report expanded_report(*reduced_report.solution_final.problem.reduction->parent_instance);
	expanded_report.solution_final = expand(reduced_report.solution_final);
	expanded_report.time = reduced_report.time;
	return expanded_report;
}
