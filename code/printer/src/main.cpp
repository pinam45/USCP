//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include "common/utils/logger.hpp"
#include "common/utils/timer.hpp"
#include "printer/printer.hpp"
#include "printer/data.hpp"
#include "git_info.hpp"

#include <cxxopts.hpp>
#include <nlohmann/json.hpp>

#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <deque>
#include <iterator>
#include <fstream>
#include <regex>

int main(int argc, char* argv[])
{
	std::ios_base::sync_with_stdio(false);
	std::setlocale(LC_ALL, "C");

	std::vector<std::string> input_folder_files;
	std::string output_prefix = "printer_out_";
	std::string validation_regex_txt = ".*\\.json";
	std::basic_regex<char> validation_regex;
	bool rwls_stats = false;
	bool memetic_comparisons = false;
	try
	{
		std::ostringstream help_txt;
		help_txt << "Unicost Set Cover Problem Printer for OR-Library and STS instances";
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
		options.add_option(
		  "",
		  cxxopts::Option("i,input",
		                  "Input folders/files to process",
		                  cxxopts::value<std::vector<std::string>>(input_folder_files),
		                  "FOLDER/FILE"));
		options.add_option(
		  "",
		  cxxopts::Option("o,output_prefix",
		                  "Output file prefix",
		                  cxxopts::value<std::string>(output_prefix)->default_value("printer_out_"),
		                  "PREFIX"));
		options.add_option(
		  "",
		  cxxopts::Option(
		    "v,validation",
		    "Input files name validation regex",
		    cxxopts::value<std::string>(validation_regex_txt)->default_value(".*\\.json"),
		    "REGEX"));
		options.add_option(
		  "",
		  cxxopts::Option("rwls_stats",
		                  "Generate RWLS stats table",
		                  cxxopts::value<bool>(rwls_stats)->default_value("false")));
		options.add_option(
		  "",
		  cxxopts::Option("memetic_comparisons",
		                  "Generate memetic comparison tables",
		                  cxxopts::value<bool>(memetic_comparisons)->default_value("false")));
		cxxopts::ParseResult result = options.parse(argc, argv);

		if(result.count("help"))
		{
			std::cout << options.help({"", "Group"}) << std::endl;
			return EXIT_SUCCESS;
		}

		if(input_folder_files.empty())
		{
			std::cout << "No folder or file specified, nothing to do" << std::endl;
			return EXIT_SUCCESS;
		}

		validation_regex = validation_regex_txt;
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

		printer printer;
		printer.generate_rwls_stats(rwls_stats);
		printer.generate_memetic_comparisons(memetic_comparisons);
		std::deque<std::string> paths(std::cbegin(input_folder_files),
		                              std::cend(input_folder_files));
		const timer timer;
		while(!paths.empty())
		{
			const std::string path = paths.front();
			paths.pop_front();

			std::error_code error;
			if(!std::filesystem::exists(path, error))
			{
				if(error)
				{
					SPDLOG_LOGGER_DEBUG(
					  LOGGER, "std::filesystem::exists failed: {}", error.message());
					LOGGER->error("Check if file/folder exist failed for {}", path);
				}
				else
				{
					LOGGER->error("Non-existing file/folder {}", path);
				}
				continue;
			}

			if(std::filesystem::is_directory(path, error))
			{
				std::filesystem::directory_iterator directory_iterator(path, error);
				if(error)
				{
					SPDLOG_LOGGER_DEBUG(
					  LOGGER, "Directory iterator creation failed: {}", error.message());
					LOGGER->error("Failed to get content of folder {}", path);
					continue;
				}
				for(const std::filesystem::directory_entry& entry: directory_iterator)
				{
					paths.push_back(entry.path().string());
				}
				continue;
			}
			if(error)
			{
				SPDLOG_LOGGER_DEBUG(
				  LOGGER, "std::filesystem::is_directory failed: {}", error.message());
				LOGGER->error("Check if path is a directory failed for {}", path);
			}

			if(std::filesystem::is_regular_file(path, error))
			{
				if(!std::regex_match(path, validation_regex))
				{
					SPDLOG_LOGGER_TRACE(LOGGER, "Ignored file {}", path);
					continue;
				}
				std::ifstream file_stream(path);
				if(!file_stream)
				{
					SPDLOG_LOGGER_DEBUG(LOGGER, "std::ifstream constructor failed");
					LOGGER->error("Failed to read file {}", path);
					continue;
				}
				LOGGER->info("Started processing {}", path);
				nlohmann::json data;
				file_stream >> data;
				if(!uscp::data::load(data, printer))
				{
					LOGGER->error("Failed to load data from {}", path);
				}
				continue;
			}
			if(error)
			{
				SPDLOG_LOGGER_DEBUG(
				  LOGGER, "std::filesystem::is_regular_file failed: {}", error.message());
				LOGGER->error("Check if path is a regular file failed for {}", path);
			}
		}
		LOGGER->info("Loaded all data in {}s", timer.elapsed());

		if(!printer.generate_document())
		{
			LOGGER->error("Failed to generate document");
		}
	}
	LOGGER->info("END");
	return EXIT_SUCCESS;
}
