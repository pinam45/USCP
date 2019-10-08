//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include "common/utils/logger.hpp"
#include "git_info.hpp"

#include <cxxopts.hpp>

#include <cstdlib>
#include <iostream>

int main(int argc, char* argv[])
{
	std::vector<std::string> input_folder_files;
	std::string output_prefix = "printer_out_";
	std::string validation_regex = ".*\\.json";
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
		  cxxopts::Option("v,validation",
		                  "Input files name validation regex",
		                  cxxopts::value<std::string>(validation_regex)->default_value(".*\\.json"),
		                  "REGEX"));
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
		//TODO
	}
	LOGGER->info("END");
	return EXIT_SUCCESS;
}
