//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include "printer/printer.hpp"
#include "common/utils/logger.hpp"
#include "common/data/instances.hpp"
#include "common/utils/timer.hpp"
#include "common/utils/utils.hpp"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <ctime>
#include <iterator>
#include <functional>

namespace
{
	struct greedy_result final
	{
		bool exist = false;
		size_t value = 0;
		double time = 0;
	};
	void to_json(nlohmann::json& j, const greedy_result& serial);

	struct rwls_result final
	{
		bool exist = false;
		size_t best = 0;
		double average = 0;
		size_t best_number = 0;
		size_t total_number = 0;
		double steps = 0;
		double time = 0;
	};
	void to_json(nlohmann::json& j, const rwls_result& serial);

	struct memetic_result final
	{
		bool exist = false;
		size_t best = 0;
		double average = 0;
		size_t best_number = 0;
		size_t total_number = 0;
		double generations = 0;
		double steps = 0;
		double time = 0;
	};
	void to_json(nlohmann::json& j, const memetic_result& serial);

	struct instance_info final
	{
		std::string name;
		size_t bks = 0;
	};
	void to_json(nlohmann::json& j, const instance_info& serial);

	struct instance_result final
	{
		instance_info instance;
		greedy_result greedy;
		rwls_result rwls;
		memetic_result memetic;
	};
	void to_json(nlohmann::json& j, const instance_result& serial);

	struct rwls_stat final
	{
		instance_info instance;
		bool exist = false;
		double initial = 0;
		double final = 0;
		double kept = 0;
		double proximity = 0;
		double steps = 0;
		double time = 0;
		size_t repetitions = 0;
	};
	void to_json(nlohmann::json& j, const rwls_stat& serial);

	struct memetic_config_result final
	{
		uscp::memetic::config_serial config;
		std::string crossover_operator;
		memetic_result result;
	};
	void to_json(nlohmann::json& j, const memetic_config_result& serial);

	struct memetic_comparison final
	{
		instance_info instance;
		bool exist = false;
		std::vector<memetic_config_result> results;
	};
	void to_json(nlohmann::json& j, const memetic_comparison& serial);

	void to_json(nlohmann::json& j, const greedy_result& serial)
	{
		j = nlohmann::json{
		  {"exist", serial.exist},
		  {"value", serial.value},
		  {"time", serial.time},
		};
	}

	void to_json(nlohmann::json& j, const rwls_result& serial)
	{
		j = nlohmann::json{
		  {"exist", serial.exist},
		  {"best", serial.best},
		  {"average", serial.average},
		  {"best_number", serial.best_number},
		  {"total_number", serial.total_number},
		  {"steps", serial.steps},
		  {"time", serial.time},
		};
	}

	void to_json(nlohmann::json& j, const memetic_result& serial)
	{
		j = nlohmann::json{
		  {"exist", serial.exist},
		  {"best", serial.best},
		  {"average", serial.average},
		  {"best_number", serial.best_number},
		  {"total_number", serial.total_number},
		  {"generations", serial.generations},
		  {"steps", serial.steps},
		  {"time", serial.time},
		};
	}

	void to_json(nlohmann::json& j, const instance_info& serial)
	{
		j = nlohmann::json{
		  {"name", serial.name},
		  {"bks", serial.bks},
		};
	}

	void to_json(nlohmann::json& j, const instance_result& serial)
	{
		j = nlohmann::json{
		  {"instance", serial.instance},
		  {"greedy", serial.greedy},
		  {"rwls", serial.rwls},
		  {"memetic", serial.memetic},
		};
	}

	void to_json(nlohmann::json& j, const rwls_stat& serial)
	{
		j = nlohmann::json{
		  {"instance", serial.instance},
		  {"exist", serial.exist},
		  {"initial", serial.initial},
		  {"final", serial.final},
		  {"kept", serial.kept},
		  {"proximity", serial.proximity},
		  {"steps", serial.steps},
		  {"time", serial.time},
		  {"repetitions", serial.repetitions},
		};
	}

	void to_json(nlohmann::json& j, const memetic_config_result& serial)
	{
		j = nlohmann::json{
		  {"config", serial.config},
		  {"crossover_operator", serial.crossover_operator},
		  {"result", serial.result},
		};
	}

	void to_json(nlohmann::json& j, const memetic_comparison& serial)
	{
		j = nlohmann::json{
		  {"instance", serial.instance},
		  {"exist", serial.exist},
		  {"results", serial.results},
		};
	}

	void replace(std::string& data, std::string_view search, std::string_view replace)
	{
		size_t pos = data.find(search);
		while(pos != std::string::npos)
		{
			data.replace(pos, search.size(), replace);
			pos = data.find(search, pos + replace.size());
		}
	}
} // namespace

namespace
{
	struct greedy_report_less_t
	{
		bool operator()(const uscp::greedy::report_serial& a,
		                const uscp::greedy::report_serial& b) const
		{
			return a.solution_final.problem.name < b.solution_final.problem.name;
		}
		bool operator()(const std::string_view& a, const uscp::greedy::report_serial& b) const
		{
			return a < b.solution_final.problem.name;
		}
		bool operator()(const uscp::greedy::report_serial& a, const std::string_view& b) const
		{
			return a.solution_final.problem.name < b;
		}
	};
	const greedy_report_less_t greedy_report_less;

	struct rwls_report_less_t
	{
		bool operator()(const uscp::rwls::report_serial& a,
		                const uscp::rwls::report_serial& b) const
		{
			return a.solution_final.problem.name < b.solution_final.problem.name;
		}
		bool operator()(const std::string_view& a, const uscp::rwls::report_serial& b) const
		{
			return a < b.solution_final.problem.name;
		}
		bool operator()(const uscp::rwls::report_serial& a, const std::string_view& b) const
		{
			return a.solution_final.problem.name < b;
		}
	};
	const rwls_report_less_t rwls_report_less;

	struct memetic_report_less_t
	{
		bool operator()(const uscp::memetic::report_serial& a,
		                const uscp::memetic::report_serial& b) const
		{
			return a.solution_final.problem.name < b.solution_final.problem.name;
		}
		bool operator()(const std::string_view& a, const uscp::memetic::report_serial& b) const
		{
			return a < b.solution_final.problem.name;
		}
		bool operator()(const uscp::memetic::report_serial& a, const std::string_view& b) const
		{
			return a.solution_final.problem.name < b;
		}
	};
	const memetic_report_less_t memetic_report_less;
} // namespace

printer::printer(std::string_view output_prefix) noexcept
  : output_folder(generate_output_folder_name(output_prefix))
  , tables_output_folder(output_folder + std::string(config::partial::TABLES_TEMPLATE_SUBFOLDER))
  , memetic_comparisons_tables_output_folder(
      tables_output_folder
      + std::string(config::partial::MEMETIC_COMPARISONS_TABLES_TEMPLATE_SUBFOLDER))
  , template_folder(
      std::string(config::partial::RESOURCES_FOLDER).append(config::partial::TEMPLATE_SUBFOLDER))
  , tables_template_folder(template_folder
                           + std::string(config::partial::TABLES_TEMPLATE_SUBFOLDER))
  , memetic_comparisons_tables_template_folder(
      tables_template_folder
      + std::string(config::partial::MEMETIC_COMPARISONS_TABLES_TEMPLATE_SUBFOLDER))
  , m_environment()
  , m_greedy_reports()
  , m_rwls_reports()
  , m_memetic_reports()
  , m_generate_rwls_stats(false)
  , m_generate_memetic_comparisons(false)
{
	m_environment.set_statement(std::string(config::inja::STATEMENT_OPEN),
	                            std::string(config::inja::STATEMENT_CLOSE));
	m_environment.set_expression(std::string(config::inja::EXPRESSION_OPEN),
	                             std::string(config::inja::EXPRESSION_CLOSE));

	m_environment.set_comment(std::string(config::inja::COMMENT_OPEN),
	                          std::string(config::inja::COMMENT_CLOSE));
	m_environment.set_line_statement(std::string(config::inja::LINE_STATEMENT_START));
	m_environment.set_trim_blocks(true);
	m_environment.set_lstrip_blocks(true);

	m_environment.add_callback("add", 2, [](inja::Arguments& args) {
		return args.at(0)->get<int>() + args.at(1)->get<int>();
	});

	m_environment.add_callback(
	  "round", 1, [](inja::Arguments& args) { return std::llround(args.at(0)->get<double>()); });

	auto percent_callback = [](inja::Arguments& args) {
		std::ostringstream txt;
		txt << std::fixed;
		if(args.size() > 1)
		{
			txt << std::setprecision(args.at(1)->get<size_t>());
		}
		else
		{
			txt << std::setprecision(2);
		}
		txt << (args.at(0)->get<double>() * 100);
		return txt.str();
	};

	m_environment.add_callback("percent", 1, percent_callback);
	m_environment.add_callback("percent", 2, percent_callback);
}

void printer::add(const uscp::greedy::report_serial& report) noexcept
{
	m_greedy_reports.push_back(report);
}

void printer::add(const uscp::rwls::report_serial& report) noexcept
{
	m_rwls_reports.push_back(report);
}

void printer::add(const uscp::memetic::report_serial& report) noexcept
{
	m_memetic_reports.push_back(report);
}

void printer::generate_rwls_stats(bool enable) noexcept
{
	m_generate_rwls_stats = enable;
}

void printer::generate_memetic_comparisons(bool enable) noexcept
{
	m_generate_memetic_comparisons = enable;
}

bool printer::generate_document() noexcept
{
	LOGGER->info("Started generating document");
	const timer timer;

	if(!create_output_folders())
	{
		LOGGER->warn("Failed to create output folders");
		return false;
	}
	LOGGER->info("Created output folders");

	if(!copy_instances_tables())
	{
		LOGGER->warn("Failed to copy instances tables");
		return false;
	}
	LOGGER->info("Copied instances tables");

	if(!generate_results_table())
	{
		LOGGER->warn("Failed to generate result table");
		return false;
	}
	LOGGER->info("Generated result table");

	if(m_generate_rwls_stats)
	{
		if(!generate_rwls_stats_table())
		{
			LOGGER->warn("Failed to generate rwls stats table");
			return false;
		}
		LOGGER->info("Generated rwls stats table");
	}

	std::vector<std::string> memetic_comparisons_tables_files;
	if(m_generate_memetic_comparisons)
	{
		if(!generate_memetic_comparisons_tables(memetic_comparisons_tables_files))
		{
			LOGGER->warn("Failed to generate memetic comparison tables");
			return false;
		}
		LOGGER->info("Generated memetic comparison tables");
	}

	// generate data
	nlohmann::json data;
	data["title"] = config::info::DOCUMENT_TITLE;
	data["author"] = config::info::DOCUMENT_AUTHOR;
	std::ostringstream now_txt;
	std::time_t t = std::time(nullptr);
	now_txt << std::put_time(std::localtime(&t), "%FT%TZ");
	data["date"] = now_txt.str();
	data["rwls_stats"] = m_generate_rwls_stats;
	data["memetic_comparisons"] = m_generate_memetic_comparisons;
	data["memetic_comparisons_tables_files"] = memetic_comparisons_tables_files;

	// generate document
	try
	{
		m_environment.write(template_folder + std::string(config::partial::DOCUMENT_TEMPLATE_FILE),
		                    data,
		                    output_folder + std::string(config::partial::DOCUMENT_TEMPLATE_FILE));
		LOGGER->info("Generated main document");
	}
	catch(const std::exception& e)
	{
		LOGGER->warn("error writing document: {}", e.what());
		return false;
	}
	catch(...)
	{
		LOGGER->warn("unknown error writing document");
		return false;
	}

	// save data
	const std::string file_data = output_folder + "main.json";
	std::ofstream data_stream(file_data, std::ios::out | std::ios::trunc);
	if(!data_stream)
	{
		SPDLOG_LOGGER_DEBUG(LOGGER, "std::ofstream constructor failed");
		LOGGER->warn("Failed to write file {}", file_data);
		return false;
	}
	data_stream << data.dump(4);

	LOGGER->info("Generated full document in {}s", timer.elapsed());
	return true;
}

bool printer::create_output_folders() noexcept
{
	std::error_code error;
	std::filesystem::create_directories(output_folder, error);
	if(error)
	{
		SPDLOG_LOGGER_DEBUG(
		  LOGGER, "std::filesystem::create_directories failed: {}", error.message());
		LOGGER->warn("Directory creation failed for: {}", output_folder);
		return false;
	}
	std::filesystem::create_directories(tables_output_folder, error);
	if(error)
	{
		SPDLOG_LOGGER_DEBUG(
		  LOGGER, "std::filesystem::create_directories failed: {}", error.message());
		LOGGER->warn("Directory creation failed for: {}", tables_output_folder);
		return false;
	}
	std::filesystem::create_directories(memetic_comparisons_tables_output_folder, error);
	if(error)
	{
		SPDLOG_LOGGER_DEBUG(
		  LOGGER, "std::filesystem::create_directories failed: {}", error.message());
		LOGGER->warn("Directory creation failed for: {}", memetic_comparisons_tables_output_folder);
		return false;
	}
	return true;
}

bool printer::copy_instances_tables() noexcept
{
	for(const std::string_view& instance_table_file: config::partial::INSTANCES_TABLES_FILES)
	{
		std::error_code error;
		std::filesystem::copy(tables_template_folder + std::string(instance_table_file),
		                      tables_output_folder + std::string(instance_table_file),
		                      std::filesystem::copy_options::skip_existing,
		                      error);
		if(error)
		{
			SPDLOG_LOGGER_DEBUG(LOGGER, "std::filesystem::copy failed: {}", error.message());
			LOGGER->warn("File copy failed for: {}", instance_table_file);
			return false;
		}
	}
	return true;
}

bool printer::generate_results_table() noexcept
{
	// generate data
	std::sort(std::begin(m_greedy_reports), std::end(m_greedy_reports), greedy_report_less);
	std::sort(std::begin(m_rwls_reports), std::end(m_rwls_reports), rwls_report_less);
	std::sort(std::begin(m_memetic_reports), std::end(m_memetic_reports), memetic_report_less);

	std::vector<instance_result> results;
	for(const uscp::problem::instance_info& instance: uscp::problem::instances)
	{
		instance_result result;
		result.instance.name = instance.name;
		result.instance.bks = instance.bks;

		const auto [greedy_begin, greedy_end] = std::equal_range(std::cbegin(m_greedy_reports),
		                                                         std::cend(m_greedy_reports),
		                                                         instance.name,
		                                                         greedy_report_less);
		if(greedy_begin != greedy_end)
		{
			result.greedy.exist = true;
			result.greedy.value = greedy_begin->solution_final.selected_subsets.size();
			result.greedy.time = greedy_begin->time;
			if(std::distance(greedy_begin, greedy_end) > 1)
			{
				LOGGER->info("Multiple greedy results for {}, only the first will be used",
				             instance.name);
			}
		}

		const auto [rwls_begin, rwls_end] = std::equal_range(
		  std::cbegin(m_rwls_reports), std::cend(m_rwls_reports), instance.name, rwls_report_less);
		for(auto it = rwls_begin; it < rwls_end; ++it)
		{
			const uscp::rwls::report_serial& rwls = *it;

			result.rwls.exist = true;
			++result.rwls.total_number;
			result.rwls.average +=
			  (1.0 / result.rwls.total_number)
			  * (static_cast<double>(rwls.solution_final.selected_subsets.size())
			     - result.rwls.average);

			if(result.rwls.best == 0
			   || rwls.solution_final.selected_subsets.size() < result.rwls.best)
			{
				result.rwls.best = rwls.solution_final.selected_subsets.size();
				result.rwls.best_number = 1;
				result.rwls.steps = static_cast<double>(rwls.found_at.steps);
				result.rwls.time = rwls.found_at.time;
			}
			else if(rwls.solution_final.selected_subsets.size() == result.rwls.best)
			{
				++result.rwls.best_number;
				result.rwls.steps +=
				  (1.0 / result.rwls.best_number)
				  * (static_cast<double>(rwls.found_at.steps) - result.rwls.steps);
				result.rwls.time +=
				  (1.0 / result.rwls.best_number) * (rwls.found_at.time - result.rwls.time);
			}
		}

		const auto [memetic_begin, memetic_end] = std::equal_range(std::cbegin(m_memetic_reports),
		                                                           std::cend(m_memetic_reports),
		                                                           instance.name,
		                                                           memetic_report_less);
		for(auto it = memetic_begin; it < memetic_end; ++it)
		{
			const uscp::memetic::report_serial& memetic = *it;

			result.memetic.exist = true;
			++result.memetic.total_number;
			result.memetic.average +=
			  (1.0 / result.memetic.total_number)
			  * (static_cast<double>(memetic.solution_final.selected_subsets.size())
			     - result.memetic.average);

			if(result.memetic.best == 0
			   || memetic.solution_final.selected_subsets.size() < result.memetic.best)
			{
				result.memetic.best = memetic.solution_final.selected_subsets.size();
				result.memetic.best_number = 1;
				result.memetic.generations = static_cast<double>(memetic.found_at.generation);
				result.memetic.steps =
				  static_cast<double>(memetic.found_at.rwls_cumulative_position.steps);
				result.memetic.time = memetic.found_at.time;
			}
			else if(memetic.solution_final.selected_subsets.size() == result.memetic.best)
			{
				++result.memetic.best_number;
				result.memetic.generations +=
				  (1.0 / result.memetic.best_number)
				  * (static_cast<double>(memetic.found_at.generation) - result.memetic.generations);
				result.memetic.steps +=
				  (1.0 / result.memetic.best_number)
				  * (static_cast<double>(memetic.found_at.rwls_cumulative_position.steps)
				     - result.memetic.steps);
				result.memetic.time += (1.0 / result.memetic.best_number)
				                       * (memetic.found_at.time - result.memetic.time);
			}
		}

		results.push_back(std::move(result));
	}
	nlohmann::json data;
	data["results"] = std::move(results);

	// generate table
	try
	{
		m_environment.write(
		  tables_template_folder + std::string(config::partial::RESULT_TABLE_TEMPLATE_FILE),
		  data,
		  tables_output_folder + std::string(config::partial::RESULT_TABLE_TEMPLATE_FILE));
	}
	catch(const std::exception& e)
	{
		LOGGER->warn("error writing result table: {}", e.what());
		return false;
	}
	catch(...)
	{
		LOGGER->warn("unknown error writing result table");
		return false;
	}

	// save data
	const std::string file_data = tables_output_folder + "results.json";
	std::ofstream data_stream(file_data, std::ios::out | std::ios::trunc);
	if(!data_stream)
	{
		SPDLOG_LOGGER_DEBUG(LOGGER, "std::ofstream constructor failed");
		LOGGER->warn("Failed to write file {}", file_data);
		return false;
	}
	data_stream << data.dump(4);
	return true;
}

bool printer::generate_rwls_stats_table() noexcept
{
	// generate data
	std::sort(std::begin(m_greedy_reports), std::end(m_greedy_reports), greedy_report_less);
	std::sort(std::begin(m_rwls_reports), std::end(m_rwls_reports), rwls_report_less);

	std::vector<rwls_stat> stats;
	for(const uscp::problem::instance_info& instance: uscp::problem::instances)
	{
		rwls_stat stat;
		stat.instance.name = instance.name;
		stat.instance.bks = instance.bks;

		const auto [rwls_begin, rwls_end] = std::equal_range(
		  std::begin(m_rwls_reports), std::end(m_rwls_reports), instance.name, rwls_report_less);
		for(auto it = rwls_begin; it < rwls_end; ++it)
		{
			std::sort(std::begin(it->solution_initial.selected_subsets),
			          std::end(it->solution_initial.selected_subsets));
			std::sort(std::begin(it->solution_final.selected_subsets),
			          std::end(it->solution_final.selected_subsets));
		}
		for(auto it = rwls_begin; it < rwls_end; ++it)
		{
			const uscp::rwls::report_serial& rwls = *it;

			stat.exist = true;
			++stat.repetitions;
			stat.initial +=
			  (1.0 / stat.repetitions)
			  * (static_cast<double>(rwls.solution_initial.selected_subsets.size()) - stat.initial);
			stat.final +=
			  (1.0 / stat.repetitions)
			  * (static_cast<double>(rwls.solution_final.selected_subsets.size()) - stat.final);
			stat.steps +=
			  (1.0 / stat.repetitions) * (static_cast<double>(rwls.found_at.steps) - stat.steps);
			stat.time += (1.0 / stat.repetitions) * (rwls.found_at.time - stat.time);

			const size_t same_count = count_common_elements_sorted(
			  rwls.solution_initial.selected_subsets, rwls.solution_final.selected_subsets);
			stat.kept =
			  static_cast<double>(same_count) / rwls.solution_final.selected_subsets.size();

			stat.proximity = 0;
			double common_mean = 0;
			size_t count_other = 0;
			for(auto it_other = rwls_begin; it_other < rwls_end; ++it_other)
			{
				if(it_other == it)
				{
					continue;
				}
				const uscp::rwls::report_serial& rwls_other = *it_other;
				++count_other;
				common_mean += (1.0 / count_other)
				               * (static_cast<double>(count_common_elements_sorted(
				                    rwls.solution_final.selected_subsets,
				                    rwls_other.solution_final.selected_subsets))
				                  - common_mean);
			}
			stat.proximity = common_mean / rwls.solution_final.selected_subsets.size();
		}

		stats.push_back(std::move(stat));
	}
	nlohmann::json data;
	data["stats"] = std::move(stats);

	// generate table
	try
	{
		m_environment.write(
		  tables_template_folder + std::string(config::partial::RWLS_STATS_TABLE_TEMPLATE_FILE),
		  data,
		  tables_output_folder + std::string(config::partial::RWLS_STATS_TABLE_TEMPLATE_FILE));
	}
	catch(const std::exception& e)
	{
		LOGGER->warn("error writing rwls stats table: {}", e.what());
		return false;
	}
	catch(...)
	{
		LOGGER->warn("unknown error writing rwls stats table");
		return false;
	}

	// save data
	const std::string file_data = tables_output_folder + "rwls_stats.json";
	std::ofstream data_stream(file_data, std::ios::out | std::ios::trunc);
	if(!data_stream)
	{
		SPDLOG_LOGGER_DEBUG(LOGGER, "std::ofstream constructor failed");
		LOGGER->warn("Failed to write file {}", file_data);
		return false;
	}
	data_stream << data.dump(4);
	return true;
}

bool printer::generate_memetic_comparisons_tables(
  std::vector<std::string>& generated_tables_files) noexcept
{
	// generate data
	std::sort(std::begin(m_memetic_reports), std::end(m_memetic_reports), memetic_report_less);

	std::vector<memetic_comparison> comparisons;
	for(const uscp::problem::instance_info& instance: uscp::problem::instances)
	{
		memetic_comparison comparison;
		comparison.instance.name = instance.name;
		comparison.instance.bks = instance.bks;

		const auto [memetic_begin, memetic_end] = std::equal_range(std::cbegin(m_memetic_reports),
		                                                           std::cend(m_memetic_reports),
		                                                           instance.name,
		                                                           memetic_report_less);
		for(auto it = memetic_begin; it < memetic_end; ++it)
		{
			const uscp::memetic::report_serial& memetic = *it;

			comparison.exist = true;
			auto result_it =
			  std::find_if(std::begin(comparison.results),
			               std::end(comparison.results),
			               [&](const memetic_config_result& result) {
				               return result.crossover_operator == memetic.crossover_operator
				                      && result.config.stopping_criterion.generation
				                           == memetic.solve_config.stopping_criterion.generation;
			               });
			memetic_config_result* result;
			if(result_it != std::end(comparison.results))
			{
				result = result_it.base();
			}
			else
			{
				comparison.results.emplace_back();
				result = &comparison.results.back();
				result->config = memetic.solve_config;
				result->crossover_operator = memetic.crossover_operator;
				result->result.exist = true;
			}
			++result->result.total_number;
			result->result.average +=
			  (1.0 / result->result.total_number)
			  * (static_cast<double>(memetic.solution_final.selected_subsets.size())
			     - result->result.average);

			if(result->result.best == 0
			   || memetic.solution_final.selected_subsets.size() < result->result.best)
			{
				result->result.best = memetic.solution_final.selected_subsets.size();
				result->result.best_number = 1;
				result->result.generations = static_cast<double>(memetic.found_at.generation);
				result->result.steps =
				  static_cast<double>(memetic.found_at.rwls_cumulative_position.steps);
				result->result.time = memetic.found_at.time;
			}
			else if(memetic.solution_final.selected_subsets.size() == result->result.best)
			{
				++result->result.best_number;
				result->result.generations +=
				  (1.0 / result->result.best_number)
				  * (static_cast<double>(memetic.found_at.generation) - result->result.generations);
				result->result.steps +=
				  (1.0 / result->result.best_number)
				  * (static_cast<double>(memetic.found_at.rwls_cumulative_position.steps)
				     - result->result.steps);
				result->result.time += (1.0 / result->result.best_number)
				                       * (memetic.found_at.time - result->result.time);
			}
		}

		comparisons.push_back(std::move(comparison));
	}

	for(memetic_comparison& comparison: comparisons)
	{
		if(!comparison.exist)
		{
			continue;
		}
		std::sort(std::begin(comparison.results),
		          std::end(comparison.results),
		          [](const memetic_config_result& a, const memetic_config_result& b) {
			          return std::make_tuple(a.crossover_operator,
			                                 a.config.stopping_criterion.generation)
			                 < std::make_tuple(b.crossover_operator,
			                                   b.config.stopping_criterion.generation);
		          });
		std::for_each(
		  std::begin(comparison.results),
		  std::end(comparison.results),
		  [](memetic_config_result& result) { replace(result.crossover_operator, "_", "\\_"); });

		const std::string template_file =
		  memetic_comparisons_tables_template_folder
		  + std::string(config::partial::MEMETIC_COMPARISON_TABLE_TEMPLATE_FILE);
		const std::string output_file =
		  std::string(config::partial::MEMETIC_COMPARISON_TABLE_OUTPUT_FILE_PREFIX)
		  + comparison.instance.name
		  + std::string(config::partial::MEMETIC_COMPARISON_TABLE_OUTPUT_FILE_POSTFIX);
		const std::string output_file_full = memetic_comparisons_tables_output_folder + output_file;

		nlohmann::json data;
		data["comparison"] = comparison;

		// generate table
		try
		{
			m_environment.write(template_file, data, output_file_full);
		}
		catch(const std::exception& e)
		{
			LOGGER->warn("error writing memetic comparison table table: {}", e.what());
			return false;
		}
		catch(...)
		{
			LOGGER->warn("unknown error writing memetic comparison table table");
			return false;
		}

		// save data
		const std::string file_data =
		  memetic_comparisons_tables_output_folder
		  + std::string(config::partial::MEMETIC_COMPARISON_TABLE_OUTPUT_FILE_PREFIX)
		  + comparison.instance.name + ".json";
		std::ofstream data_stream(file_data, std::ios::out | std::ios::trunc);
		if(!data_stream)
		{
			SPDLOG_LOGGER_DEBUG(LOGGER, "std::ofstream constructor failed");
			LOGGER->warn("Failed to write file {}", file_data);
			return false;
		}
		data_stream << data.dump(4);

		generated_tables_files.push_back(output_file);
	}

	return true;
}

std::string printer::generate_output_folder_name(std::string_view output_prefix) const noexcept
{
	std::ostringstream output_folder_stream;
	output_folder_stream << output_prefix;
	std::time_t t = std::time(nullptr);
	output_folder_stream << std::put_time(std::localtime(&t), "%Y-%m-%d-%H-%M-%S");
	output_folder_stream << config::partial::OUTPUT_FOLDER_POSTFIX;
	return output_folder_stream.str();
}
