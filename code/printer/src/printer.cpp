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
		double time = 0;
	};
	void to_json(nlohmann::json& j, const rwls_result& serial);

	struct instance_result final
	{
		std::string name;
		size_t bks = 0;
		greedy_result greedy;
		rwls_result rwls;
	};
	void to_json(nlohmann::json& j, const instance_result& serial);

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
		  {"time", serial.time},
		};
	}

	void to_json(nlohmann::json& j, const instance_result& serial)
	{
		j = nlohmann::json{
		  {"name", serial.name},
		  {"bks", serial.bks},
		  {"greedy", serial.greedy},
		  {"rwls", serial.rwls},
		};
	}

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
} // namespace

printer::printer(std::string_view output_prefix) noexcept
  : output_folder(generate_output_folder_name(output_prefix))
  , tables_output_folder(output_folder + std::string(config::partial::TABLES_TEMPLATE_SUBFOLDER))
  , template_folder(
      std::string(config::partial::RESOURCES_FOLDER).append(config::partial::TEMPLATE_SUBFOLDER))
  , tables_template_folder(template_folder
                           + std::string(config::partial::TABLES_TEMPLATE_SUBFOLDER))
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
}

void printer::add(const uscp::greedy::report_serial& report) noexcept
{
	m_greedy_reports.push_back(report);
}

void printer::add(const uscp::rwls::report_serial& report) noexcept
{
	m_rwls_reports.push_back(report);
}

bool printer::generate_document() noexcept
{
	if(!create_output_folders())
	{
		LOGGER->warn("Failed to create output folders");
		return false;
	}

	if(!copy_instances_tables())
	{
		LOGGER->warn("Failed to copy instances tables");
		return false;
	}

	if(!generate_results_table())
	{
		LOGGER->warn("Failed to generate result table");
		return false;
	}

	// generate data
	nlohmann::json data;
	data["title"] = config::info::DOCUMENT_TITLE;
	data["author"] = config::info::DOCUMENT_AUTHOR;

	// generate document
	try
	{
		m_environment.write(template_folder + std::string(config::partial::DOCUMENT_TEMPLATE_FILE),
		                    data,
		                    output_folder + std::string(config::partial::DOCUMENT_TEMPLATE_FILE));
	}
	catch(const std::exception& e)
	{
		std::cout << "error writing document: " << e.what() << std::endl;
		return false;
	}
	catch(...)
	{
		std::cerr << "unknown error writing document" << std::endl;
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

	std::vector<instance_result> results;
	for(const uscp::problem::instance_info& instance: uscp::problem::instances)
	{
		instance_result result;
		result.name = instance.name;
		result.bks = instance.bks;

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
				LOGGER->warn("Multiple greedy results for {}, only the first will be used",
				             instance.name);
			}
		}

		const auto [rwls_begin, rwls_end] = std::equal_range(
		  std::cbegin(m_rwls_reports), std::cend(m_rwls_reports), instance.name, rwls_report_less);
		if(greedy_begin != greedy_end)
		{
			for(auto it = rwls_begin; it < rwls_end; ++it)
			{
				result.rwls.exist = true;
				++result.rwls.total_number;
				result.rwls.average +=
				  (1.0 / result.rwls.total_number)
				  * (static_cast<double>(it->solution_final.selected_subsets.size())
				     - result.rwls.average);

				if(result.rwls.best == 0
				   || it->solution_final.selected_subsets.size() < result.rwls.best)
				{
					result.rwls.best = it->solution_final.selected_subsets.size();
					result.rwls.best_number = 1;
					result.rwls.time = it->time;
				}
				else if(it->solution_final.selected_subsets.size() == result.rwls.best)
				{
					++result.rwls.best_number;
					result.rwls.time +=
					  (1.0 / result.rwls.best_number) * (it->time - result.rwls.time);
				}
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
		std::cout << "error writing result table: " << e.what() << std::endl;
		return false;
	}
	catch(...)
	{
		std::cerr << "unknown error writing result table" << std::endl;
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

std::string printer::generate_output_folder_name(std::string_view output_prefix) const noexcept
{
	std::ostringstream output_folder_stream;
	output_folder_stream << output_prefix;
	std::time_t t = std::time(nullptr);
	output_folder_stream << std::put_time(std::localtime(&t), "%Y-%m-%d-%H-%M-%S");
	output_folder_stream << config::partial::OUTPUT_FOLDER_POSTFIX;
	return output_folder_stream.str();
}
