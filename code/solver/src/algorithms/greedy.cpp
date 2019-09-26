//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include "algorithms/greedy.hpp"
#include "utils/logger.hpp"
#include "utils/timer.hpp"

void uscp::greedy::to_json(nlohmann::json& j, const uscp::greedy::report_serial& serial)
{
	j = nlohmann::json{{"solution_final", serial.solution_final}, {"time", serial.time}};
}

void uscp::greedy::from_json(const nlohmann::json& j, uscp::greedy::report_serial& serial)
{
	j.at("solution_final").get_to(serial.solution_final);
	j.at("time").get_to(serial.time);
}

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
	const timer timer;

	report report(problem);
	while(!report.solution_final.cover_all_points)
	{
		size_t max_subset_number =
		  report.solution_final.selected_subsets.size(); //invalid initial value
		dynamic_bitset<> covered_points_with_max_subset(problem.points_number);
		size_t covered_points_number_with_max_subset = report.solution_final.covered_points.count();
		for(size_t i = 0; i < problem.subsets_number; ++i)
		{
			if(report.solution_final.selected_subsets[i])
			{
				// already selected
				continue;
			}

			dynamic_bitset<> new_covered_points =
			  report.solution_final.covered_points | problem.subsets_points[i];
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
			report.time = timer.elapsed();
			return report;
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
	                    "Found greedy solution to {} with {} subsets in {}s",
	                    problem.name,
	                    report.solution_final.selected_subsets.count(),
	                    report.time);

	return report;
}
