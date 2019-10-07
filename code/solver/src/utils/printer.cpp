//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include "utils/printer.hpp"

#include <common/utils/logger.hpp>

#include <nlohmann/json.hpp>

#include <filesystem>
#include <ctime>
#include <sstream>

printer::printer() noexcept
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

void printer::add_instance(std::string_view instance_group,
                           const uscp::problem::instance_info& instance)
{
	assert(!instance_group.empty());
	assert(!instance.name.empty());
	// find or add group
	auto it = std::find_if(
	  std::begin(m_instances),
	  std::end(m_instances),
	  [&](const std::pair<std::string, std::vector<uscp::problem::instance_info>>& group) {
		  return group.first == instance_group;
	  });
	if(it == std::end(m_instances))
	{
		m_instances.emplace_back(instance_group, std::vector<uscp::problem::instance_info>());
		it = --std::end(m_instances);
	}

	// find or add instance
	if(std::find_if(std::begin(it->second),
	                std::end(it->second),
	                [&](const uscp::problem::instance_info& list_instance) {
		                return list_instance.name == instance.name;
	                })
	   == std::end(it->second))
	{
		it->second.push_back(instance);
	}
}

void printer::add_solution(std::string_view solutions_group, const uscp::solution& solution)
{
	m_solutions[std::string(solutions_group)].push_back(solution);
}

bool printer::generate_results() noexcept
{
	const std::string output_folder = generate_output_folder_name();
	const std::string tables_output_folder =
	  output_folder + std::string(config::partial::TABLES_TEMPLATE_SUBFOLDER);
	const std::string template_folder =
	  std::string(config::partial::RESOURCES_FOLDER).append(config::partial::TEMPLATE_SUBFOLDER);
	const std::string tables_template_folder =
	  template_folder + std::string(config::partial::TABLES_TEMPLATE_SUBFOLDER);

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

	// Generate data
	nlohmann::json data_document;
	data_document["title"] = config::info::DOCUMENT_TITLE;
	data_document["author"] = config::info::DOCUMENT_AUTHOR;

	std::vector<std::string> solutions_groups_names;
	for(const std::pair<const std::string, std::vector<uscp::solution>>& solutions_group:
	    m_solutions)
	{
		solutions_groups_names.push_back(solutions_group.first);
	}

	std::vector<nlohmann::json> data_document_instances_groups;
	for(const std::pair<std::string, std::vector<uscp::problem::instance_info>>& instances_group:
	    m_instances)
	{
		nlohmann::json data_instances_group;
		data_instances_group["name"] = instances_group.first;
		data_instances_group["solutions_groups_names"] = solutions_groups_names;
		std::vector<nlohmann::json> data_instances;
		for(const uscp::problem::instance_info& instance: instances_group.second)
		{
			nlohmann::json data_instance;
			data_instance["info"] = instance;
			std::vector<nlohmann::json> data_solutions_groups;
			for(const std::pair<const std::string, std::vector<uscp::solution>>& solutions_group:
			    m_solutions)
			{
				nlohmann::json data_solutions_group;
				data_solutions_group["name"] = solutions_group.first;
				std::vector<uscp::solution> solutions;
				for(const uscp::solution& solution: solutions_group.second)
				{
					if(solution.problem.name == instance.name)
					{
						solutions.push_back(solution);
					}
				}
				data_solutions_group["solutions"] = std::move(solutions);
				data_solutions_groups.push_back(std::move(data_solutions_group));
			}
			data_instance["solutions_groups"] = std::move(data_solutions_groups);
			data_instances.push_back(std::move(data_instance));
		}
		data_instances_group["instances"] = std::move(data_instances);

		// generate instances table
		std::string instances_input_file =
		  tables_template_folder + std::string(config::partial::INSTANCES_TABLE_TEMPLATE_FILE);
		std::string instances_output_filename =
		  std::string(config::partial::INSTANCES_TABLE_OUTPUT_FILE_PREFIX) + instances_group.first
		  + std::string(config::partial::INSTANCES_TABLE_OUTPUT_FILE_POSTFIX);
		std::string instances_output_file = tables_output_folder + instances_output_filename;
		m_environment.write(instances_input_file, data_instances_group, instances_output_file);

		// generate results table
		std::string reults_input_file =
		  tables_template_folder + std::string(config::partial::RESULT_TABLE_TEMPLATE_FILE);
		std::string reults_output_filename =
		  std::string(config::partial::RESULT_TABLE_OUTPUT_FILE_PREFIX) + instances_group.first
		  + std::string(config::partial::RESULT_TABLE_OUTPUT_FILE_POSTFIX);
		std::string reults_output_file = tables_output_folder + reults_output_filename;
		m_environment.write(reults_input_file, data_instances_group, reults_output_file);

		// save data
		const std::string file_data = tables_output_folder + instances_group.first + ".json";
		std::ofstream data_stream(file_data, std::ios::out | std::ios::trunc);
		if(!data_stream)
		{
			SPDLOG_LOGGER_DEBUG(LOGGER, "std::ofstream constructor failed");
			LOGGER->warn("Failed to write file {}", file_data);
			return false;
		}
		data_stream << data_instances_group.dump(4);

		// set document data
		nlohmann::json data_document_instances_group;
		data_document_instances_group["name"] = instances_group.first;
		data_document_instances_group["instances_table_path"] = instances_output_filename;
		data_document_instances_group["results_table_path"] = reults_output_filename;
		data_document_instances_groups.push_back(std::move(data_document_instances_group));
	}
	data_document["instances_groups"] = std::move(data_document_instances_groups);

	// generate document
	m_environment.write(template_folder + std::string(config::partial::DOCUMENT_TEMPLATE_FILE),
	                    data_document,
	                    output_folder + std::string(config::partial::DOCUMENT_TEMPLATE_FILE));

	// save data
	const std::string file_data = output_folder + "document.json";
	std::ofstream data_stream(file_data, std::ios::out | std::ios::trunc);
	if(!data_stream)
	{
		SPDLOG_LOGGER_DEBUG(LOGGER, "std::ofstream constructor failed");
		LOGGER->warn("Failed to write file {}", file_data);
		return false;
	}
	data_stream << data_document.dump(4);
	return true;
}

std::string printer::generate_output_folder_name() const noexcept
{
	std::ostringstream output_folder_stream;
	output_folder_stream << config::partial::OUTPUT_FOLDER_PREFIX;
	std::time_t t = std::time(nullptr);
	output_folder_stream << std::put_time(std::localtime(&t), "-%Y-%m-%d--%H-%M-%S");
	output_folder_stream << config::partial::OUTPUT_FOLDER_POSTFIX;
	return output_folder_stream.str();
}
