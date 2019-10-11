//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_PRINTER_HPP
#define USCP_PRINTER_HPP

#include "common/data/solution.hpp"

#include <inja/inja.hpp>

#include <string_view>
#include <string>
#include <vector>
#include <unordered_map>

class printer final
{
public:
	struct config
	{
		struct inja
		{
			static constexpr std::string_view STATEMENT_OPEN = "<*";
			static constexpr std::string_view STATEMENT_CLOSE = "*>";
			static constexpr std::string_view EXPRESSION_OPEN = "<<";
			static constexpr std::string_view EXPRESSION_CLOSE = ">>";
			static constexpr std::string_view COMMENT_OPEN = "<#";
			static constexpr std::string_view COMMENT_CLOSE = "#>";
			static constexpr std::string_view LINE_STATEMENT_START = "##";
		};

		struct partial
		{
			static constexpr std::string_view RESOURCES_FOLDER = "./resources/";
			static constexpr std::string_view OUTPUT_FOLDER_PREFIX = "./generated-";
			static constexpr std::string_view OUTPUT_FOLDER_POSTFIX = "/";

			static constexpr std::string_view TEMPLATE_SUBFOLDER = "templates/";
			static constexpr std::string_view TABLES_TEMPLATE_SUBFOLDER = "tables/";

			static constexpr std::string_view DOCUMENT_TEMPLATE_FILE = "main.tex";
			static constexpr std::string_view INSTANCES_TABLE_TEMPLATE_FILE = "instances.tex";
			static constexpr std::string_view INSTANCES_TABLE_OUTPUT_FILE_PREFIX = "";
			static constexpr std::string_view INSTANCES_TABLE_OUTPUT_FILE_POSTFIX =
			  "_instances.tex";
			static constexpr std::string_view RESULT_TABLE_TEMPLATE_FILE = "results.tex";
			static constexpr std::string_view RESULT_TABLE_OUTPUT_FILE_PREFIX = "";
			static constexpr std::string_view RESULT_TABLE_OUTPUT_FILE_POSTFIX = "_results.tex";
		};

		struct info
		{
			static constexpr std::string_view DOCUMENT_TITLE = "USCP results";
			static constexpr std::string_view DOCUMENT_AUTHOR =
			  "\\url{https://github.com/pinam45/USCP}";
		};
	};

	printer() noexcept;
	printer(const printer&) = delete;
	printer(printer&&) noexcept = default;
	printer& operator=(const printer&) = delete;
	printer& operator=(printer&&) noexcept = default;

private:
	inja::Environment m_environment;

	[[nodiscard]] std::string generate_output_folder_name() const noexcept;
};

#endif //USCP_PRINTER_HPP
