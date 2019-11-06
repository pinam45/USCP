//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include "solver/data/instance.hpp"
#include "solver/algorithms/greedy.hpp"
#include "solver/algorithms/rwls.hpp"
#include "solver/algorithms/memetic.hpp"
#include "solver/algorithms/crossovers/merge.hpp"
#include "solver/data/instances.hpp"
#include "common/utils/logger.hpp"
#include "common/utils/random.hpp"
#include "common/data/instance.hpp"
#include "common/data/instances.hpp"
#include "common/data/solution.hpp"
#include "git_info.hpp"

#include <cxxopts.hpp>
#include <nlohmann/json.hpp>

#include <cstdlib>
#include <vector>
#include <limits>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <string>
#include <string_view>
#include <utility>

namespace
{
	constexpr bool CHECK_INSTANCES = false;
} // namespace

int main(int argc, char* argv[])
{
	std::vector<std::string> instances;
	std::string output_prefix = "solver_out_";
	size_t repetitions = 1;
	bool greedy = false;
	bool rwls = false;
	uscp::rwls::position rwls_stop;
	bool memetic = false;
	uscp::memetic::config memetic_config;
	std::string memetic_crossover;
	try
	{
		std::ostringstream help_txt;
		help_txt << "Unicost Set Cover Problem Solver for OR-Library and STS instances";
		help_txt << "\n";
		help_txt << "Build commit: ";
		help_txt << git_info::head_sha1;
		if(git_info::is_dirty)
		{
			help_txt << " (with uncommitted changes)";
		}
		help_txt << "\n";
		cxxopts::Options options("solver", help_txt.str());
		options.add_option("", cxxopts::Option("help", "Print help"));
		options.add_option("",
		                   cxxopts::Option("i,instances",
		                                   "Instances to process",
		                                   cxxopts::value<std::vector<std::string>>(instances),
		                                   "NAME"));
		options.add_option(
		  "",
		  cxxopts::Option("o,output_prefix",
		                  "Output file prefix",
		                  cxxopts::value<std::string>(output_prefix)->default_value("solver_out_"),
		                  "PREFIX"));
		options.add_option("",
		                   cxxopts::Option("r,repetitions",
		                                   "Repetitions number",
		                                   cxxopts::value<size_t>(repetitions)->default_value("1"),
		                                   "N"));

		// Greedy
		options.add_option(
		  "",
		  cxxopts::Option("greedy",
		                  "Solve with greedy algorithm (no repetition as it is determinist)",
		                  cxxopts::value<bool>(greedy)->default_value("false")));

		// RWLS
		options.add_option("",
		                   cxxopts::Option("rwls",
		                                   "Improve with RWLS algorithm (start with a greedy)",
		                                   cxxopts::value<bool>(rwls)->default_value("false")));
		options.add_option(
		  "",
		  cxxopts::Option("rwls_steps",
		                  "RWLS steps limit",
		                  cxxopts::value<size_t>(rwls_stop.steps)->default_value("30000000"),
		                  "N"));
		options.add_option(
		  "",
		  cxxopts::Option("rwls_time",
		                  "RWLS time (seconds) limit",
		                  cxxopts::value<double>(rwls_stop.time)
		                    ->default_value(std::to_string(std::numeric_limits<size_t>::max())),
		                  "N"));

		// Memetic
		options.add_option("",
		                   cxxopts::Option("memetic",
		                                   "Solve with memetic algorithm",
		                                   cxxopts::value<bool>(memetic)->default_value("false")));
		options.add_option(
		  "",
		  cxxopts::Option("memetic_generations",
		                  "Memetic generations number limit",
		                  cxxopts::value<size_t>(memetic_config.stopping_criterion.generation)
		                    ->default_value("100"),
		                  "N"));
		options.add_option(
		  "",
		  cxxopts::Option(
		    "memetic_cumulative_rwls_steps",
		    "Memetic cumulative RWLS steps limit",
		    cxxopts::value<size_t>(memetic_config.stopping_criterion.rwls_cumulative_position.steps)
		      ->default_value(std::to_string(std::numeric_limits<size_t>::max())),
		    "N"));
		options.add_option(
		  "",
		  cxxopts::Option(
		    "memetic_cumulative_rwls_time",
		    "Memetic cumulative RWLS time (seconds) limit",
		    cxxopts::value<double>(memetic_config.stopping_criterion.rwls_cumulative_position.time)
		      ->default_value(std::to_string(std::numeric_limits<size_t>::max())),
		    "N"));
		options.add_option(
		  "",
		  cxxopts::Option("memetic_time",
		                  "Memetic time limit",
		                  cxxopts::value<double>(memetic_config.stopping_criterion.time)
		                    ->default_value(std::to_string(std::numeric_limits<size_t>::max())),
		                  "N"));
		options.add_option(
		  "",
		  cxxopts::Option("memetic_rwls_steps",
		                  "Memetic RWLS steps limit",
		                  cxxopts::value<size_t>(memetic_config.rwls_stopping_criterion.steps)
		                    ->default_value("300000"),
		                  "N"));
		options.add_option(
		  "",
		  cxxopts::Option("memetic_rwls_time",
		                  "Memetic RWLS time (seconds) limit",
		                  cxxopts::value<double>(memetic_config.rwls_stopping_criterion.time)
		                    ->default_value(std::to_string(std::numeric_limits<size_t>::max())),
		                  "N"));
		options.add_option(
		  "",
		  cxxopts::Option("memetic_crossover",
		                  "Memetic crossover operator",
		                  cxxopts::value<std::string>(memetic_crossover)->default_value("merge"),
		                  "OPERATOR"));
		cxxopts::ParseResult result = options.parse(argc, argv);

		if(result.count("help"))
		{
			std::cout << options.help({"", "Group"}) << std::endl;
			return EXIT_SUCCESS;
		}

		if(instances.empty())
		{
			std::cout << "No instances specified, nothing to do" << std::endl;
			return EXIT_SUCCESS;
		}

		if(!greedy && !rwls && !memetic)
		{
			std::cout << "No algorithm specified, nothing to do" << std::endl;
			return EXIT_SUCCESS;
		}

		if(repetitions == 0)
		{
			std::cout << "0 repetitions, nothing to do" << std::endl;
			return EXIT_SUCCESS;
		}
	}
	catch(const std::exception& e)
	{
		std::cout << "error parsing options: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	catch(...)
	{
		std::cerr << "unknown error parsing options" << std::endl;
		return EXIT_FAILURE;
	}

	if(!init_logger())
	{
		return EXIT_FAILURE;
	}
	LOGGER->info("START");
	{
		if(git_info::is_dirty)
		{
			LOGGER->info("Commit: {} (with uncommitted changes)", git_info::head_sha1);
		}
		else
		{
			LOGGER->info("Commit: {}", git_info::head_sha1);
		}

		// check instances
		if constexpr(CHECK_INSTANCES)
		{
			uscp::problem::check_instances();
		}

		/*for(const uscp::problem::instance_info& instance_info: uscp::problem::instances)
		{
			if(instance_info.name.substr(0, 3) != "STS")
			{
				continue;
			}
			uscp::problem::instance instance;
			if(!uscp::problem::read(instance_info, instance))
			{
				LOGGER->error("Failed to read problem {}", instance_info);
				continue;
			}
			uscp::problem::instance instance_reduced = uscp::problem::reduce(instance);
			if(!uscp::problem::has_solution(instance_reduced))
			{
				LOGGER->error("Invalid reduced instance");
			}
			LOGGER->error("{}: ({}, {}) -> ({}, {}) {}",
			              instance.name,
			              instance.points_number,
			              instance.subsets_number,
			              instance_reduced.points_number,
			              instance_reduced.subsets_number,
			              instance_reduced.points_number != instance.points_number
			                || instance_reduced.subsets_number != instance.subsets_number);
		}
		return EXIT_SUCCESS;*/

		uscp::random_engine generator(std::random_device{}());
		nlohmann::json data;
		data["git"]["retrieved_state"] = git_info::retrieved_state;
		data["git"]["head_sha1"] = git_info::head_sha1;
		data["git"]["is_dirty"] = git_info::is_dirty;
		std::ostringstream now_txt;
		std::time_t t = std::time(nullptr);
		now_txt << std::put_time(std::localtime(&t), "%FT%TZ");
		data["date"] = now_txt.str();
		std::vector<nlohmann::json> data_instances;
		for(const std::string& instance_name: instances)
		{
			const auto instance_it =
			  std::find_if(std::cbegin(uscp::problem::instances),
			               std::cend(uscp::problem::instances),
			               [&](const uscp::problem::instance_info& instance_info) {
				               return instance_info.name == instance_name;
			               });
			if(instance_it == std::cend(uscp::problem::instances))
			{
				LOGGER->error("No instance named {} exist", instance_name);
				continue;
			}
			LOGGER->info("Current instance information: {}", *instance_it);

			uscp::problem::instance instance_base;
			if(!uscp::problem::read(*instance_it, instance_base))
			{
				LOGGER->error("Failed to read problem {}", *instance_it);
				continue;
			}
			const bool reduced = instance_it->can_reduce;
			uscp::problem::instance instance;
			if(reduced)
			{
				instance = uscp::problem::reduce(instance_base);
				if(!uscp::problem::has_solution(instance))
				{
					LOGGER->error("Invalid reduced instance");
				}
			}
			else
			{
				instance = instance_base;
			}

			nlohmann::json data_instance;
			data_instance["instance"] = instance_base.serialize();
			if(greedy && !rwls)
			{
				uscp::greedy::report greedy_report = uscp::greedy::solve_report(instance);
				if(reduced)
				{
					uscp::greedy::report expanded_greedy_report =
					  uscp::greedy::expand(greedy_report);
					if(!expanded_greedy_report.solution_final.cover_all_points)
					{
						LOGGER->error("Expanded greedy solution doesn't cover all points");
						return EXIT_FAILURE;
					}
					LOGGER->info("({}) Expanded greedy solution to {} subsets",
					             instance_base.name,
					             expanded_greedy_report.solution_final.selected_subsets.count());
					LOGGER->info("({}) Greedy found solution with {} subsets",
					             instance_base.name,
					             expanded_greedy_report.solution_final.selected_subsets.count());
					data_instance["greedy"] = expanded_greedy_report.serialize();
				}
				else
				{
					LOGGER->info("({}) Greedy found solution with {} subsets",
					             instance_base.name,
					             greedy_report.solution_final.selected_subsets.count());
					data_instance["greedy"] = greedy_report.serialize();
				}
			}
			if(rwls)
			{
				uscp::greedy::report greedy_report = uscp::greedy::solve_report(instance);
				if(greedy)
				{
					if(reduced)
					{
						uscp::greedy::report expanded_greedy_report =
						  uscp::greedy::expand(greedy_report);
						if(!expanded_greedy_report.solution_final.cover_all_points)
						{
							LOGGER->error("Expanded greedy solution doesn't cover all points");
							return EXIT_FAILURE;
						}
						LOGGER->info(
						  "({}) Expanded greedy solution to {} subsets",
						  instance_base.name,
						  expanded_greedy_report.solution_final.selected_subsets.count());
						LOGGER->info(
						  "({}) Greedy found solution with {} subsets",
						  instance_base.name,
						  expanded_greedy_report.solution_final.selected_subsets.count());
						data_instance["greedy"] = expanded_greedy_report.serialize();
					}
					else
					{
						LOGGER->info("({}) Greedy found solution with {} subsets",
						             instance_base.name,
						             greedy_report.solution_final.selected_subsets.count());
						data_instance["greedy"] = greedy_report.serialize();
					}
				}
				std::vector<nlohmann::json> data_rwls;
				for(size_t repetition = 0; repetition < repetitions; ++repetition)
				{
					uscp::rwls::report rwls_report = uscp::rwls::improve_report(
					  greedy_report.solution_final, generator, rwls_stop);
					if(reduced)
					{
						uscp::rwls::report expanded_rwls_report = uscp::rwls::expand(rwls_report);
						if(!expanded_rwls_report.solution_final.cover_all_points)
						{
							LOGGER->error("Expanded rwls solution doesn't cover all points");
							return EXIT_FAILURE;
						}
						LOGGER->info("({}) Expanded rwls solution to {} subsets",
						             instance_base.name,
						             expanded_rwls_report.solution_final.selected_subsets.count());
						LOGGER->info("({}) RWLS improved solution from {} subsets to {} subsets",
						             instance_base.name,
						             expanded_rwls_report.solution_initial.selected_subsets.count(),
						             expanded_rwls_report.solution_final.selected_subsets.count());
						data_rwls.emplace_back(expanded_rwls_report.serialize());
					}
					else
					{
						LOGGER->info("({}) RWLS improved solution from {} subsets to {} subsets",
						             instance_base.name,
						             rwls_report.solution_initial.selected_subsets.count(),
						             rwls_report.solution_final.selected_subsets.count());
						data_rwls.emplace_back(rwls_report.serialize());
					}
				}
				data_instance["rwls"] = std::move(data_rwls);
			}
			if(memetic)
			{
				auto process_memetic = [&](auto memetic_alg) -> bool {
					std::vector<nlohmann::json> data_memetic;
					memetic_alg.initialize();
					for(size_t repetition = 0; repetition < repetitions; ++repetition)
					{
						uscp::memetic::report memetic_report =
						  memetic_alg.solve(generator, memetic_config);
						if(reduced)
						{
							uscp::memetic::report expanded_memetic_report =
							  uscp::memetic::expand(memetic_report);
							if(!expanded_memetic_report.solution_final.cover_all_points)
							{
								LOGGER->error("Expanded memetic solution doesn't cover all points");
								return false;
							}
							LOGGER->info(
							  "({}) Expanded memetic solution to {} subsets",
							  instance_base.name,
							  expanded_memetic_report.solution_final.selected_subsets.count());
							LOGGER->info(
							  "({}) Memetic found solution with {} subsets",
							  instance_base.name,
							  expanded_memetic_report.solution_final.selected_subsets.count());
							data_memetic.emplace_back(expanded_memetic_report.serialize());
						}
						else
						{
							LOGGER->info("({}) Memetic found solution with {} subsets",
							             instance_base.name,
							             memetic_report.solution_final.selected_subsets.count());
							data_memetic.emplace_back(memetic_report.serialize());
						}
					}
					data_instance["memetic"] = std::move(data_memetic);
					return true;
				};
				if(memetic_crossover == "merge")
				{
					uscp::memetic::memetic<uscp::crossover::merge> memetic_alg_(instance);
					if(!process_memetic(memetic_alg_))
					{
						return EXIT_FAILURE;
					}
				}
				else
				{
					LOGGER->error("No crossover operator named \"{}\" exist", memetic_crossover);
					return EXIT_FAILURE;
				}
			}
			data_instances.push_back(data_instance);
		}
		data["instances"] = std::move(data_instances);

		// save data
		std::ostringstream file_data_stream;
		file_data_stream << output_prefix;
		file_data_stream << std::put_time(std::localtime(&t), "%Y-%m-%d-%H-%M-%S");
		file_data_stream << "_";
		file_data_stream << generator();
		file_data_stream << ".json";
		const std::string file_data = file_data_stream.str();
		std::ofstream data_stream(file_data, std::ios::out | std::ios::trunc);
		if(!data_stream)
		{
			SPDLOG_LOGGER_DEBUG(LOGGER, "std::ofstream constructor failed");
			LOGGER->error("Failed to write file {}", file_data);
			return EXIT_FAILURE;
		}
		data_stream << data.dump(4);
	}
	LOGGER->info("END");
	return EXIT_SUCCESS;
}
