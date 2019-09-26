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
#include "algorithms/rwls.hpp"
#include "data/instances.hpp"
#include "utils/printer.hpp"
#include "git_info.hpp"

#include <cxxopts.hpp>

#include <cstdlib>
#include <vector>
#include <limits>
#include <sstream>

namespace
{
	constexpr bool CHECK_INSTANCES = false;
	constexpr bool GENERATE_PROBLEM = false;
	constexpr const uscp::problem::instance_info& INSTANCE_INFO = uscp::problem::instances[2];

	bool benchmark_greedy()
	{
		std::vector<uscp::problem::instance> instances;
		printer printer;

		//		uscp::random_engine g_problem(314159);
		//		static constexpr uscp::problem::instance_info info{
		//		  "", "generated-1", 32, 12, 0., 1, 100, 0};
		//		uscp::problem::instance generated_instance = uscp::problem::generate(32, 12, g_problem);
		//		generated_instance.info = &info;
		//		instances.push_back(generated_instance);
		//		printer.add_instance("Generated", generated_instance);
		//		LOGGER->info("Generated problem: {}", generated_instance);

		for(const uscp::problem::instance_info& instance_info: uscp::problem::instances)
		{
			if(instance_info.name.substr(0, 3) == "NRE")
			{
				break;
			}
			uscp::problem::instance instance;
			if(!uscp::problem::read(instance_info, instance))
			{
				LOGGER->warn("Failed to read problem {}", instance_info);
				return false;
			}
			instances.push_back(instance);
			printer.add_instance("OR-Library", instance_info);
			//break;
		}

		for(const uscp::problem::instance& instance: instances)
		{
			uscp::solution greedy_solution = uscp::greedy::solve(instance);
			printer.add_solution("greedy", greedy_solution);
			//LOGGER->info("Greedy solution to {}: {}", instance.name, greedy_solution);

			uscp::random_engine g(314159);
			uscp::rwls::stop stopping_criterion;
			stopping_criterion.steps = 10000;
			uscp::rwls::report rwls_report =
			  uscp::rwls::improve_report(greedy_solution, g, stopping_criterion);
			printer.add_solution("rwls", rwls_report.solution_final);
			//LOGGER->info("RWLS solution to {}: {}", instance.name, rwls_solution);
			nlohmann::json rwls_solution_json = rwls_report.serialize();
			LOGGER->info("RWLS report: {}", rwls_solution_json.dump(4));
		}
		printer.generate_results();
		return true;
	}
} // namespace

int main(int argc, char* argv[])
{
	std::vector<std::string> instances;
	std::string output_prefix = "solver_out_";
	size_t repetitions = 1;
	bool greedy = false;
	bool rwls = false;
	uscp::rwls::stop rwls_stop;
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
		options.add_option(
		  "",
		  cxxopts::Option("greedy",
		                  "Solve with greedy algorithm (no repetition as it is determinist)",
		                  cxxopts::value<bool>(greedy)->default_value("false")));
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

		if(!greedy && !rwls)
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
		uscp::random_engine generator(std::random_device{}());

		// check instances
		if constexpr(CHECK_INSTANCES)
		{
			uscp::problem::check_instances();
		}

		//benchmark_greedy();

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

			uscp::problem::instance instance;
			if(!uscp::problem::read(*instance_it, instance))
			{
				LOGGER->error("Failed to read problem {}", *instance_it);
				continue;
			}
			nlohmann::json data_instance;
			data_instance["instance"] = instance.serialize();

			if(greedy && !rwls)
			{
				uscp::greedy::report greedy_report = uscp::greedy::solve_report(instance);
				data_instance["greedy"] = greedy_report.serialize();
			}
			if(rwls)
			{
				uscp::greedy::report greedy_report = uscp::greedy::solve_report(instance);
				if(greedy)
				{
					data_instance["greedy"] = greedy_report.serialize();
				}
				std::vector<nlohmann::json> data_rwls;
				for(size_t repetition = 0; repetition < repetitions; ++repetition)
				{
					uscp::rwls::report rwls_report = uscp::rwls::improve_report(
					  greedy_report.solution_final, generator, rwls_stop);
					data_rwls.emplace_back(rwls_report.serialize());
				}
				data_instance["rwls"] = std::move(data_rwls);
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
