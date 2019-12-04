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
#include "common/algorithms/greedy.hpp"
#include "common/algorithms/rwls.hpp"
#include "common/algorithms/memetic.hpp"

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
			static constexpr const std::string_view STATEMENT_OPEN = "<*";
			static constexpr const std::string_view STATEMENT_CLOSE = "*>";
			static constexpr const std::string_view EXPRESSION_OPEN = "<<";
			static constexpr const std::string_view EXPRESSION_CLOSE = ">>";
			static constexpr const std::string_view COMMENT_OPEN = "<#";
			static constexpr const std::string_view COMMENT_CLOSE = "#>";
			static constexpr const std::string_view LINE_STATEMENT_START = "##";
		};

		struct partial
		{
			// Folders
			static constexpr const std::string_view RESOURCES_FOLDER = "./resources/";
			static constexpr const std::string_view OUTPUT_FOLDER_PREFIX = "./printer_out_";
			static constexpr const std::string_view OUTPUT_FOLDER_POSTFIX = "/";
			static constexpr const std::string_view TEMPLATE_SUBFOLDER = "templates/";
			static constexpr const std::string_view TABLES_TEMPLATE_SUBFOLDER = "tables/";
			static constexpr const std::string_view MEMETIC_COMPARISONS_TABLES_TEMPLATE_SUBFOLDER =
			  "memetic_comparisons/";
			static constexpr const std::string_view PLOTS_TEMPLATE_SUBFOLDER = "plots/";

			// Template files
			static constexpr const std::string_view DOCUMENT_TEMPLATE_FILE = "main.tex";
			static constexpr const std::string_view RESULT_TABLE_TEMPLATE_FILE = "results.tex";
			static constexpr const std::string_view RWLS_STATS_TABLE_TEMPLATE_FILE =
			  "rwls_stats.tex";
			static constexpr const std::string_view RWLS_WEIGHTS_PLOT_TEMPLATE_FILE = "weights.tex";
			static constexpr const std::string_view RWLS_WEIGHTS_PLOT_IMPORT_TEMPLATE_FILE =
			  "weights_import.tex";
			static constexpr const std::string_view MEMETIC_COMPARISON_TABLE_TEMPLATE_FILE =
			  "comparison.tex";

			// Output files
			static constexpr const std::string_view RWLS_WEIGHTS_CSV_OUTPUT_FILE_PREFIX =
			  "rwls_weights_";
			static constexpr const std::string_view RWLS_WEIGHTS_CSV_OUTPUT_FILE_POSTFIX = ".csv";
			static constexpr const std::string_view RWLS_WEIGHTS_PLOT_OUTPUT_FILE_PREFIX =
			  "rwls_weights_";
			static constexpr const std::string_view RWLS_WEIGHTS_PLOT_OUTPUT_FILE_POSTFIX = ".tex";
			static constexpr const std::string_view RWLS_WEIGHTS_PLOT_IMPORT_OUTPUT_FILE_PREFIX =
			  "rwls_weights_import_";
			static constexpr const std::string_view RWLS_WEIGHTS_PLOT_IMPORT_OUTPUT_FILE_POSTFIX =
			  ".tex";
			static constexpr const std::string_view MEMETIC_COMPARISON_TABLE_OUTPUT_FILE_PREFIX =
			  "comparison_";
			static constexpr const std::string_view MEMETIC_COMPARISON_TABLE_OUTPUT_FILE_POSTFIX =
			  ".tex";

			// Copied files
			static constexpr const std::string_view LATEXMKRC_FILE = ".latexmkrc";
			static constexpr const std::array<std::string_view, 4> INSTANCES_TABLES_FILES = {
			  "orlibrary_instances_base.tex",
			  "orlibrary_instances_cyc_clr.tex",
			  "orlibrary_instances_rail.tex",
			  "sts_instances.tex"};
		};

		struct info
		{
			static constexpr const std::string_view DOCUMENT_TITLE = "USCP results";
			static constexpr const std::string_view DOCUMENT_AUTHOR =
			  "\\url{https://github.com/pinam45/USCP}";
		};
	};

	explicit printer(
	  std::string_view output_prefix = config::partial::OUTPUT_FOLDER_PREFIX) noexcept;
	printer(const printer&) = delete;
	printer(printer&&) = default;
	printer& operator=(const printer&) = delete;
	printer& operator=(printer&&) noexcept = default;

	void add(const uscp::greedy::report_serial& report) noexcept;

	void add(const uscp::rwls::report_serial& report) noexcept;

	void add(const uscp::memetic::report_serial& report) noexcept;

	void generate_rwls_stats(bool enable) noexcept;
	void generate_rwls_weights(bool enable) noexcept;
	void generate_memetic_comparisons(bool enable) noexcept;

	bool generate_document() noexcept;

private:
	std::string output_folder;
	std::string tables_output_folder;
	std::string memetic_comparisons_tables_output_folder;
	std::string plots_output_folder;

	std::string template_folder;
	std::string tables_template_folder;
	std::string memetic_comparisons_tables_template_folder;
	std::string plots_template_folder;

	inja::Environment m_environment;
	std::vector<uscp::greedy::report_serial> m_greedy_reports;
	std::vector<uscp::rwls::report_serial> m_rwls_reports;
	std::vector<uscp::memetic::report_serial> m_memetic_reports;
	bool m_generate_rwls_stats;
	bool m_generate_rwls_weights;
	bool m_generate_memetic_comparisons;

	bool create_output_folders() noexcept;

	bool copy_latexmkrc() noexcept;
	bool copy_instances_tables() noexcept;

	bool generate_results_table() noexcept;
	bool generate_rwls_stats_table() noexcept;
	bool generate_rwls_weights_plots(std::vector<std::string>& generated_plots_files) noexcept;
	bool generate_memetic_comparisons_tables(
	  std::vector<std::string>& generated_tables_files) noexcept;

	[[nodiscard]] std::string generate_output_folder_name(std::string_view output_prefix) const
	  noexcept;
};

#endif //USCP_PRINTER_HPP
