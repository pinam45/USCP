//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include "utils/logger.hpp"
#include "utils/random.hpp"
#include "data/instance.hpp"
#include "algorithms/exhaustive.hpp"
#include "algorithms/greedy.hpp"
#include "data/orlibrary.hpp"
#include "data/sts.hpp"

#include <cstdlib>

namespace
{
	constexpr bool CHECK_INSTANCES = false;
	constexpr bool GENERATE_PROBLEM = false;
	//constexpr const char* PROBLEM_FILE_PATH = "resources/OR-Library/scp41.txt";
	constexpr const char* PROBLEM_FILE_PATH = "resources/steiner-triple-covering/data.27";

	void check_instances() noexcept
	{
		LOGGER->info("OR-Library instances validation start");
		if(uscp::problem::orlibrary::check_instances())
		{
			LOGGER->info("OR-Library instances validation succeeded");
		}
		else
		{
			LOGGER->error("OR-Library instances validation failed");
		}
		LOGGER->info("STS instances validation start");
		if(uscp::problem::sts::check_instances())
		{
			LOGGER->info("STS instances validation succeeded");
		}
		else
		{
			LOGGER->error("STS instances validation failed");
		}
	}
} // namespace

int main()
{
	if(!init_logger())
	{
		return EXIT_FAILURE;
	}
	LOGGER->info("START");
	{
		uscp::random_engine g(std::random_device{}());

		// check instances
		if constexpr(CHECK_INSTANCES)
		{
			check_instances();
		}

		// read problem
		uscp::problem::instance problem_instance;
		if constexpr(GENERATE_PROBLEM)
		{
			problem_instance = uscp::problem::generate(30, 100, g);
		}
		else
		{
			if(!uscp::problem::sts::read(PROBLEM_FILE_PATH, problem_instance))
			{
				LOGGER->error("Failed to read problem");
				return EXIT_FAILURE;
			}
		}

		// save problem
		if(!uscp::problem::orlibrary::write(problem_instance, "last_problem.txt", true))
		{
			LOGGER->error("Failed to write problem instance");
			return EXIT_FAILURE;
		}

		// print problem
		LOGGER->info("Problem: {}", problem_instance);

		// check if the problem have a solution
		if(!uscp::problem::has_solution(problem_instance))
		{
			LOGGER->error(
			  "Unsolvable problem (some elements cannot be covered using provided subsets).");
			return EXIT_FAILURE;
		}

		// solve
		uscp::solution greedy_solution = uscp::greedy::solve(problem_instance);
		LOGGER->info("Greedy solution: {}", greedy_solution);

		//		uscp::solution optimal_solution = uscp::exhaustive::solve(problem_instance);
		//		LOGGER->info("Optimal solution: {}", optimal_solution);
	}
	LOGGER->info("END");
	return EXIT_SUCCESS;
}
