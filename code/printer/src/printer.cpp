//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include "printer/printer.hpp"
#include "common/utils/logger.hpp"
#include "common/utils/timer.hpp"
#include "common/utils/utils.hpp"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <ctime>
#include <iterator>
#include <functional>

namespace
{
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
  , tables_output_folder(concat(output_folder, config::partial::TABLES_TEMPLATE_SUBFOLDER))
  , memetic_comparisons_tables_output_folder(
      concat(tables_output_folder, config::partial::MEMETIC_COMPARISONS_TABLES_TEMPLATE_SUBFOLDER))
  , plots_output_folder(concat(output_folder, config::partial::PLOTS_TEMPLATE_SUBFOLDER))
  , template_folder(concat(config::partial::RESOURCES_FOLDER, config::partial::TEMPLATE_SUBFOLDER))
  , tables_template_folder(concat(template_folder, config::partial::TABLES_TEMPLATE_SUBFOLDER))
  , memetic_comparisons_tables_template_folder(
      concat(tables_template_folder,
             config::partial::MEMETIC_COMPARISONS_TABLES_TEMPLATE_SUBFOLDER))
  , plots_template_folder(concat(template_folder, config::partial::PLOTS_TEMPLATE_SUBFOLDER))
  , m_environment()
  , m_greedy_reports()
  , m_rwls_reports()
  , m_memetic_reports()
  , m_generate_rwls_stats(false)
  , m_generate_rwls_weights(false)
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

	m_environment.add_callback("fround", 2, [](inja::Arguments& args) -> std::string {
		std::ostringstream txt;
		txt << std::fixed;
		txt << std::setprecision(args.at(1)->get<int>());
		txt << args.at(0)->get<double>();
		return txt.str();
	});

	auto percent_callback = [](inja::Arguments& args) -> std::string {
		std::ostringstream txt;
		txt << std::fixed;
		if(args.size() > 1)
		{
			txt << std::setprecision(args.at(1)->get<int>());
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

	m_environment.add_callback("sep", 1, [](inja::Arguments& args) {
		constexpr char separator = ' ';
		std::string value = std::to_string(args.at(0)->get<size_t>());
		size_t len = value.length();
		size_t dlen = 3;

		while(len > dlen)
		{
			value.insert(len - dlen, 1, separator);
			dlen += 4;
			len += 1;
		}
		return value;
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

void printer::add(const uscp::memetic::report_serial& report) noexcept
{
	m_memetic_reports.push_back(report);
}

void printer::generate_rwls_stats(bool enable) noexcept
{
	m_generate_rwls_stats = enable;
}

void printer::generate_rwls_weights(bool enable) noexcept
{
	m_generate_rwls_weights = enable;
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

	if(!copy_latexmkrc())
	{
		LOGGER->warn("Failed to copy .latexmkrc");
		return false;
	}

	if(!copy_instances_tables())
	{
		LOGGER->warn("Failed to copy instances tables");
		return false;
	}

	// generate tables
	std::sort(std::begin(m_greedy_reports), std::end(m_greedy_reports), greedy_report_less);
	std::sort(std::begin(m_rwls_reports), std::end(m_rwls_reports), rwls_report_less);
	std::sort(std::begin(m_memetic_reports), std::end(m_memetic_reports), memetic_report_less);
	std::vector<instance_info> instances = gather_instances_infos();

	if(!generate_results_table(instances))
	{
		LOGGER->warn("Failed to generate result table");
		return false;
	}

	if(m_generate_rwls_stats)
	{
		if(!generate_rwls_stats_table(instances))
		{
			LOGGER->warn("Failed to generate rwls stats table");
			return false;
		}
	}

	std::vector<std::string> rwls_weights_plots_files;
	if(m_generate_rwls_weights)
	{
		if(!generate_rwls_weights_plots(instances, rwls_weights_plots_files))
		{
			LOGGER->warn("Failed to generate rwls weights plots");
			return false;
		}
	}

	std::vector<std::string> memetic_comparisons_tables_files;
	if(m_generate_memetic_comparisons)
	{
		if(!generate_memetic_comparisons_tables(instances, memetic_comparisons_tables_files))
		{
			LOGGER->warn("Failed to generate memetic comparison tables");
			return false;
		}
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
	data["rwls_weights"] = m_generate_rwls_weights;
	data["memetic_comparisons_tables_files"] = memetic_comparisons_tables_files;
	data["rwls_weights_plots_files"] = rwls_weights_plots_files;

	// generate document
	const std::string template_file =
	  concat(template_folder, config::partial::DOCUMENT_TEMPLATE_FILE);
	const std::string output_file = concat(output_folder, config::partial::DOCUMENT_TEMPLATE_FILE);
	if(!write_and_save(template_file, data, output_file))
	{
		LOGGER->error("Failed to generate full document");
		return false;
	}

	LOGGER->info("Generated full document in {}s", timer.elapsed());
	return true;
}

template<typename... Report>
std::vector<printer::instance_info> gather_instances_infos_(
  const std::vector<Report>&... reports) noexcept
{
	std::vector<printer::instance_info> instances;
	instances.reserve(uscp::problem::instances.size());
	for(const uscp::problem::instance_info& instance_full_info: uscp::problem::instances)
	{
		printer::instance_info instance;
		instance.name = instance_full_info.name;
		instance.bks = instance_full_info.bks;
		instance.subsets_number = instance_full_info.subsets;
		instance.points_number = instance_full_info.points;

		instances.push_back(std::move(instance));
	}

	auto process_report = [&](const auto& report) noexcept
	{
		auto it = std::find_if(
		  std::cbegin(instances),
		  std::cend(instances),
		  [&](const printer::instance_info& instance) noexcept {
			  return instance.name == report.solution_final.problem.name;
		  });
		if(it == std::cend(instances))
		{
			printer::instance_info instance;
			instance.name = report.solution_final.problem.name;
			instance.bks = 0;
			instance.subsets_number = report.solution_final.problem.subsets_number;
			instance.points_number = report.solution_final.problem.points_number;
			instances.push_back(std::move(instance));
		}
	};
	auto process_reports = [&](const auto& reports_) noexcept
	{
		for(const auto& report: reports_)
		{
			process_report(report);
		}
	};
	(process_reports(reports), ...);

	return instances;
}

std::vector<printer::instance_info> printer::gather_instances_infos() noexcept
{
	return gather_instances_infos_(m_greedy_reports, m_rwls_reports, m_memetic_reports);
}

bool printer::create_output_folders() noexcept
{
	std::error_code error;
	for(const std::string& directory: {output_folder,
	                                   tables_output_folder,
	                                   memetic_comparisons_tables_output_folder,
	                                   plots_output_folder})
	{
		std::filesystem::create_directories(directory, error);
		if(error)
		{
			SPDLOG_LOGGER_DEBUG(
			  LOGGER, "std::filesystem::create_directories failed: {}", error.message());
			LOGGER->warn("Directory creation failed for: {}", directory);
			return false;
		}
	}
	LOGGER->info("Created output folders");
	return true;
}

bool printer::copy_latexmkrc() noexcept
{
	std::error_code error;
	std::filesystem::copy(concat(template_folder, config::partial::LATEXMKRC_FILE),
	                      concat(output_folder, config::partial::LATEXMKRC_FILE),
	                      std::filesystem::copy_options::skip_existing,
	                      error);
	if(error)
	{
		SPDLOG_LOGGER_DEBUG(LOGGER, "std::filesystem::copy failed: {}", error.message());
		LOGGER->warn("File copy failed for: {}", config::partial::LATEXMKRC_FILE);
		return false;
	}
	LOGGER->info("Copied .latexmkrc");
	return true;
}

bool printer::copy_instances_tables() noexcept
{
	for(const std::string_view& instance_table_file: config::partial::INSTANCES_TABLES_FILES)
	{
		std::error_code error;
		std::filesystem::copy(concat(tables_template_folder, instance_table_file),
		                      concat(tables_output_folder, instance_table_file),
		                      std::filesystem::copy_options::skip_existing,
		                      error);
		if(error)
		{
			SPDLOG_LOGGER_DEBUG(LOGGER, "std::filesystem::copy failed: {}", error.message());
			LOGGER->warn("File copy failed for: {}", instance_table_file);
			return false;
		}
	}
	LOGGER->info("Copied instances tables");
	return true;
}

bool printer::generate_results_table(const std::vector<instance_info>& instances) noexcept
{
	const timer timer;

	// generate data
	std::vector<instance_result> results;
	for(const instance_info& instance: instances)
	{
		instance_result result;
		result.instance = instance;

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
				SPDLOG_LOGGER_DEBUG(LOGGER,
				                    "Multiple greedy results for {}, only the first will be used",
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
		for(auto it = rwls_begin; it < rwls_end; ++it)
		{
			const uscp::rwls::report_serial& rwls = *it;
			assert(rwls.solution_final.selected_subsets.size() >= result.rwls.best);
			const size_t pos = rwls.solution_final.selected_subsets.size() - result.rwls.best;
			if(pos >= result.rwls.top_count.size())
			{
				result.rwls.top_count.resize(pos + 1);
			}
			++result.rwls.top_count[pos];
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
		for(auto it = memetic_begin; it < memetic_end; ++it)
		{
			const uscp::memetic::report_serial& memetic = *it;
			assert(memetic.solution_final.selected_subsets.size() >= result.memetic.best);
			const size_t pos = memetic.solution_final.selected_subsets.size() - result.memetic.best;
			if(pos >= result.memetic.top_count.size())
			{
				result.memetic.top_count.resize(pos + 1);
			}
			++result.memetic.top_count[pos];
		}

		results.push_back(std::move(result));
	}
	nlohmann::json data;
	data["results"] = std::move(results);

	// generate table
	const std::string template_file =
	  concat(tables_template_folder, config::partial::RESULT_TABLE_TEMPLATE_FILE);
	const std::string output_file =
	  concat(tables_output_folder, config::partial::RESULT_TABLE_TEMPLATE_FILE);
	if(!write_and_save(template_file, data, output_file))
	{
		LOGGER->error("Failed to generate results table");
		return false;
	}

	LOGGER->info("Generated results table in {}", timer.elapsed());
	return true;
}

bool printer::generate_rwls_stats_table(const std::vector<instance_info>& instances) noexcept
{
	const timer timer;

	// generate data
	std::vector<rwls_stat> stats;
	stats.reserve(uscp::problem::instances.size());
	for(const instance_info& instance: instances)
	{
		rwls_stat stat;
		stat.instance = instance;

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
	const std::string template_file =
	  concat(tables_template_folder, config::partial::RWLS_STATS_TABLE_TEMPLATE_FILE);
	const std::string output_file =
	  concat(tables_output_folder, config::partial::RWLS_STATS_TABLE_TEMPLATE_FILE);
	if(!write_and_save(template_file, data, output_file))
	{
		LOGGER->error("Failed to generate RWLS stats table");
		return false;
	}

	LOGGER->info("Generated RWLS stats table in {}", timer.elapsed());
	return true;
}

bool printer::generate_rwls_weights_plots(const std::vector<instance_info>& instances,
                                          std::vector<std::string>& generated_plots_files) noexcept
{
	// generate data
	struct weights_data
	{
		float mean = 0;
		float stddev = 0;
		float min = std::numeric_limits<float>::max();
		float max = std::numeric_limits<float>::min();
	};
	for(const instance_info& instance: instances)
	{
		const timer timer;
		rwls_weights_stats weights_stat;
		weights_stat.instance.name = instance.name;
		weights_stat.instance.bks = instance.bks;
		weights_stat.instance.subsets_number = instance.subsets_number;
		weights_stat.instance.points_number = instance.points_number;
		weights_stat.repetitions = 0;

		std::vector<const uscp::rwls::report_serial*> rwls_reports_with_weights;
		const auto [rwls_begin, rwls_end] = std::equal_range(
		  std::begin(m_rwls_reports), std::end(m_rwls_reports), instance.name, rwls_report_less);
		for(auto it = rwls_begin; it < rwls_end; ++it)
		{
			if(it->points_weights_final.size() != instance.points_number)
			{
				// no final weights information
				continue;
			}
			rwls_reports_with_weights.push_back(&(*it));
			weights_stat.exist = true;
			++weights_stat.repetitions;
		}

		std::vector<weights_data> weights_data;
		weights_data.resize(instance.points_number);
		for(const uscp::rwls::report_serial* rwls_ptr: rwls_reports_with_weights)
		{
			const uscp::rwls::report_serial& rwls = *rwls_ptr;
			for(size_t i = 0; i < instance.points_number; ++i)
			{
				weights_data[i].mean += rwls.points_weights_final[i];
				if(rwls.points_weights_final[i] < weights_data[i].min)
				{
					weights_data[i].min = rwls.points_weights_final[i];
				}
				if(rwls.points_weights_final[i] > weights_data[i].max)
				{
					weights_data[i].max = rwls.points_weights_final[i];
				}
			}
		}
		if(!weights_stat.exist)
		{
			continue;
		}
		for(size_t i = 0; i < instance.points_number; ++i)
		{
			weights_data[i].mean /= weights_stat.repetitions;
			weights_stat.weights_mean_mean += weights_data[i].mean;
		}
		weights_stat.weights_mean_mean /= instance.points_number;
		for(const uscp::rwls::report_serial* rwls_ptr: rwls_reports_with_weights)
		{
			const uscp::rwls::report_serial& rwls = *rwls_ptr;

			for(size_t i = 0; i < instance.points_number; ++i)
			{
				weights_data[i].stddev +=
				  std::pow(rwls.points_weights_final[i] - weights_data[i].mean, 2.f);
			}
		}
		for(size_t i = 0; i < instance.points_number; ++i)
		{
			weights_data[i].stddev /= weights_stat.repetitions;
			weights_data[i].stddev = std::sqrt(weights_data[i].stddev);
		}

		weights_stat.data_file = concat(config::partial::RWLS_WEIGHTS_CSV_OUTPUT_FILE_PREFIX,
		                                instance.name,
		                                config::partial::RWLS_WEIGHTS_CSV_OUTPUT_FILE_POSTFIX);
		{
			const std::string data_file = concat(plots_output_folder, weights_stat.data_file);
			std::ofstream data_stream(data_file, std::ios::out | std::ios::trunc);
			if(!data_stream)
			{
				SPDLOG_LOGGER_DEBUG(LOGGER, "std::ofstream constructor failed");
				LOGGER->warn("Failed to write file {}", data_file);
				return false;
			}
			data_stream << "point, mean, stddev, min, max\n";
			for(size_t i = 0; i < weights_data.size(); ++i)
			{
				data_stream << i << ", " << weights_data[i].mean << ", " << weights_data[i].stddev
				            << ", " << weights_data[i].min << ", " << weights_data[i].max << "\n";
			}
		}

		weights_stat.plot_file = concat(config::partial::RWLS_WEIGHTS_PLOT_OUTPUT_FILE_PREFIX,
		                                instance.name,
		                                config::partial::RWLS_WEIGHTS_PLOT_OUTPUT_FILE_POSTFIX);
		{
			const std::string template_plot_file =
			  concat(plots_template_folder, config::partial::RWLS_WEIGHTS_PLOT_TEMPLATE_FILE);
			const std::string output_plot_file =
			  concat(plots_output_folder, weights_stat.plot_file);

			nlohmann::json data;
			data["weights_stat"] = weights_stat;

			if(!write_and_save(template_plot_file, data, output_plot_file))
			{
				LOGGER->error("Failed to generate RWLS weights plot for instance {}",
				              instance.name);
				return false;
			}
		}

		std::string import_file =
		  concat(config::partial::RWLS_WEIGHTS_PLOT_IMPORT_OUTPUT_FILE_PREFIX,
		         instance.name,
		         config::partial::RWLS_WEIGHTS_PLOT_IMPORT_OUTPUT_FILE_POSTFIX);
		generated_plots_files.push_back(import_file);
		{
			const std::string template_figure_file = concat(
			  plots_template_folder, config::partial::RWLS_WEIGHTS_PLOT_IMPORT_TEMPLATE_FILE);
			const std::string output_figure_file = concat(plots_output_folder, import_file);

			nlohmann::json data;
			data["weights_stat"] = weights_stat;

			if(!write_and_save(template_figure_file, data, output_figure_file))
			{
				LOGGER->error("Failed to generate RWLS weights plot figure for instance {}",
				              instance.name);
				return false;
			}
		}
		LOGGER->info(
		  "Generated RWLS weights plot for instance {} in {}", instance.name, timer.elapsed());
	}

	return true;
}

bool printer::generate_memetic_comparisons_tables(
  const std::vector<instance_info>& instances,
  std::vector<std::string>& generated_tables_files) noexcept
{
	// generate data
	for(const instance_info& instance: instances)
	{
		const timer timer;

		memetic_comparison comparison;
		comparison.instance = instance;

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
				                      && result.wcrossover_operator == memetic.wcrossover_operator
				                      && result.config.stopping_criterion.generation
				                           == memetic.solve_config.stopping_criterion.generation;
			               });
			memetic_config_result* result;
			if(result_it != std::end(comparison.results))
			{
				result = &(*result_it);
			}
			else
			{
				comparison.results.emplace_back();
				result = &comparison.results.back();
				result->config = memetic.solve_config;
				result->crossover_operator = memetic.crossover_operator;
				result->wcrossover_operator = memetic.wcrossover_operator;
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
		if(!comparison.exist)
		{
			continue;
		}
		for(auto it = memetic_begin; it < memetic_end; ++it)
		{
			const uscp::memetic::report_serial& memetic = *it;
			auto result_it =
			  std::find_if(std::begin(comparison.results),
			               std::end(comparison.results),
			               [&](const memetic_config_result& result) {
				               return result.crossover_operator == memetic.crossover_operator
				                      && result.wcrossover_operator == memetic.wcrossover_operator
				                      && result.config.stopping_criterion.generation
				                           == memetic.solve_config.stopping_criterion.generation;
			               });
			if(result_it == std::end(comparison.results))
			{
				continue;
			}
			memetic_config_result& result = *result_it;
			assert(memetic.solution_final.selected_subsets.size() >= result.result.best);
			const size_t pos = memetic.solution_final.selected_subsets.size() - result.result.best;
			if(pos >= result.result.top_count.size())
			{
				result.result.top_count.resize(pos + 1);
			}
			++result.result.top_count[pos];
		}

		std::sort(std::begin(comparison.results),
		          std::end(comparison.results),
		          [](const memetic_config_result& a, const memetic_config_result& b) {
			          return std::make_tuple(
			                   a.result.best,
			                   -(static_cast<float>(a.result.best_number) / a.result.total_number),
			                   a.result.steps,
			                   a.result.time)
			                 < std::make_tuple(
			                   b.result.best,
			                   -(static_cast<float>(b.result.best_number) / b.result.total_number),
			                   b.result.steps,
			                   b.result.time);
		          });
		std::for_each(std::begin(comparison.results),
		              std::end(comparison.results),
		              [](memetic_config_result& result) {
			              replace(result.crossover_operator, "_", "\\_");
			              replace(result.wcrossover_operator, "_", "\\_");
		              });

		nlohmann::json data;
		data["comparison"] = comparison;

		// generate table
		const std::string template_file =
		  concat(memetic_comparisons_tables_template_folder,
		         config::partial::MEMETIC_COMPARISON_TABLE_TEMPLATE_FILE);
		const std::string output_file_relative =
		  concat(config::partial::MEMETIC_COMPARISON_TABLE_OUTPUT_FILE_PREFIX,
		         comparison.instance.name,
		         config::partial::MEMETIC_COMPARISON_TABLE_OUTPUT_FILE_POSTFIX);
		const std::string output_file =
		  concat(memetic_comparisons_tables_output_folder, output_file_relative);
		if(!write_and_save(template_file, data, output_file))
		{
			LOGGER->error("Failed to generate Memetic comparison table for instance {}",
			              instance.name);
			return false;
		}

		LOGGER->info("Generated Memetic comparison table for instance {} in {}s",
		             instance.name,
		             timer.elapsed());
		generated_tables_files.push_back(output_file_relative);
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

bool printer::write_and_save(const std::string& template_file,
                             const nlohmann::json& data,
                             const std::string& output_file) noexcept
{
	try
	{
		m_environment.write(template_file, data, output_file);
	}
	catch(const std::exception& e)
	{
		LOGGER->warn("Error writing {}: {}", output_file, e.what());
		return false;
	}
	catch(...)
	{
		LOGGER->warn("Unknown error writing {}", output_file);
		return false;
	}

	// save data
	const std::string file_data = concat(output_file, ".json");
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

void to_json(nlohmann::json& j, const printer::greedy_result& serial)
{
	j = nlohmann::json{
	  {"exist", serial.exist},
	  {"value", serial.value},
	  {"time", serial.time},
	};
}

void to_json(nlohmann::json& j, const printer::rwls_result& serial)
{
	j = nlohmann::json{
	  {"exist", serial.exist},
	  {"best", serial.best},
	  {"average", serial.average},
	  {"best_number", serial.best_number},
	  {"total_number", serial.total_number},
	  {"steps", serial.steps},
	  {"time", serial.time},
	  {"top_count", serial.top_count},
	};
}

void to_json(nlohmann::json& j, const printer::memetic_result& serial)
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
	  {"top_count", serial.top_count},
	};
}

void to_json(nlohmann::json& j, const printer::instance_info& serial)
{
	j = nlohmann::json{
	  {"name", serial.name},
	  {"points_number", serial.points_number},
	  {"subsets_number", serial.subsets_number},
	  {"bks", serial.bks},
	};
}

void to_json(nlohmann::json& j, const printer::instance_result& serial)
{
	j = nlohmann::json{
	  {"instance", serial.instance},
	  {"greedy", serial.greedy},
	  {"rwls", serial.rwls},
	  {"memetic", serial.memetic},
	};
}

void to_json(nlohmann::json& j, const printer::rwls_stat& serial)
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

void to_json(nlohmann::json& j, const printer::memetic_config_result& serial)
{
	j = nlohmann::json{
	  {"config", serial.config},
	  {"crossover_operator", serial.crossover_operator},
	  {"wcrossover_operator", serial.wcrossover_operator},
	  {"result", serial.result},
	};
}

void to_json(nlohmann::json& j, const printer::memetic_comparison& serial)
{
	j = nlohmann::json{
	  {"instance", serial.instance},
	  {"exist", serial.exist},
	  {"results", serial.results},
	};
}

void to_json(nlohmann::json& j, const printer::rwls_weights_stats& serial)
{
	j = nlohmann::json{
	  {"instance", serial.instance},
	  {"exist", serial.exist},
	  //{"stopping_criterion", serial.stopping_criterion},
	  {"repetitions", serial.repetitions},
	  {"weights_mean_mean", serial.weights_mean_mean},
	  {"data_file", serial.data_file},
	  {"plot_file", serial.plot_file},
	};
}
