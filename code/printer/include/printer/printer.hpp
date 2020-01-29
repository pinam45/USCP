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
#include "common/data/instances.hpp"
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
			  "figure_weights.tex";
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
			  "figure_rwls_weights_";
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
	struct greedy_result final
	{
		bool exist = false;
		size_t value = 0;
		double time = 0;
	};
	friend void to_json(nlohmann::json& j, const printer::greedy_result& serial);

	struct rwls_result final
	{
		bool exist = false;
		size_t best = 0;
		double average = 0;
		size_t best_number = 0;
		size_t total_number = 0;
		double steps = 0;
		double time = 0;
		std::vector<size_t> top_count = {0};
	};
	friend void to_json(nlohmann::json& j, const printer::rwls_result& serial);

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
		std::vector<size_t> top_count = {0};
	};
	friend void to_json(nlohmann::json& j, const printer::memetic_result& serial);

	struct instance_info final
	{
		std::string name;
		size_t points_number = 0;
		size_t subsets_number = 0;
		size_t bks = 0;
	};
	friend void to_json(nlohmann::json& j, const printer::instance_info& serial);

	struct instance_result final
	{
		instance_info instance;
		greedy_result greedy;
		rwls_result rwls;
		memetic_result memetic;
	};
	friend void to_json(nlohmann::json& j, const printer::instance_result& serial);

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
	friend void to_json(nlohmann::json& j, const printer::rwls_stat& serial);

	struct memetic_config_result final
	{
		uscp::memetic::config_serial config;
		std::string crossover_operator;
		std::string wcrossover_operator;
		memetic_result result;
	};
	friend void to_json(nlohmann::json& j, const printer::memetic_config_result& serial);

	struct memetic_comparison final
	{
		instance_info instance;
		bool exist = false;
		std::vector<memetic_config_result> results;
	};
	friend void to_json(nlohmann::json& j, const printer::memetic_comparison& serial);

	struct rwls_weights_stats final
	{
		instance_info instance;
		bool exist = false;
		//uscp::rwls::position_serial stopping_criterion;
		size_t repetitions = 0;
		float weights_mean_mean = 0;
		std::string data_file; //csv
		std::string plot_file; //tex
	};
	friend void to_json(nlohmann::json& j, const printer::rwls_weights_stats& serial);

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

	template<typename... Report>
	friend std::vector<instance_info> gather_instances_infos_(
	  const std::vector<Report>&... reports) noexcept;
	std::vector<instance_info> gather_instances_infos() noexcept;

	bool create_output_folders() noexcept;

	bool copy_latexmkrc() noexcept;
	bool copy_instances_tables() noexcept;

	bool generate_results_table(const std::vector<instance_info>& instances) noexcept;
	bool generate_rwls_stats_table(const std::vector<instance_info>& instances) noexcept;
	bool generate_rwls_weights_plots(const std::vector<instance_info>& instances,
	                                 std::vector<std::string>& generated_plots_files) noexcept;
	bool generate_memetic_comparisons_tables(
	  const std::vector<instance_info>& instances,
	  std::vector<std::string>& generated_tables_files) noexcept;

	[[nodiscard]] std::string generate_output_folder_name(std::string_view output_prefix) const
	  noexcept;

	bool write_and_save(const std::string& template_file,
	                    const nlohmann::json& data,
	                    const std::string& output_file) noexcept;
};

#endif //USCP_PRINTER_HPP
