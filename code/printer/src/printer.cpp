//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include "printer/printer.hpp"
#include "common/utils/logger.hpp"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <ctime>
#include <iterator>

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

std::string printer::generate_output_folder_name() const noexcept
{
	std::ostringstream output_folder_stream;
	output_folder_stream << config::partial::OUTPUT_FOLDER_PREFIX;
	std::time_t t = std::time(nullptr);
	output_folder_stream << std::put_time(std::localtime(&t), "-%Y-%m-%d--%H-%M-%S");
	output_folder_stream << config::partial::OUTPUT_FOLDER_POSTFIX;
	return output_folder_stream.str();
}

void printer::add(const uscp::greedy::report_serial& report) noexcept
{
	//TODO
}

void printer::add(const uscp::rwls::report_serial& report) noexcept
{
	//TODO
}
