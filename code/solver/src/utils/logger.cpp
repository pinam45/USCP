//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include "utils/logger.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include <iostream>

const std::shared_ptr<spdlog::logger> NULL_LOGGER =
  std::make_shared<spdlog::logger>("null", std::make_shared<spdlog::sinks::null_sink_mt>());
const std::shared_ptr<spdlog::logger> LOGGER =
  std::make_shared<spdlog::logger>("global", std::make_shared<spdlog::sinks::null_sink_mt>());

namespace
{
	constexpr const char* LOG_FILENAME = "log.txt";
	constexpr std::size_t LOG_MAX_SIZE = 1024 * 1024 * 5;
	constexpr std::size_t LOG_MAX_FILES = 3;
} // namespace

bool init_logger()
{
	assert(NULL_LOGGER != nullptr);
	try
	{
		// Clear null sink
		LOGGER->sinks().clear();

		// File sink
		std::shared_ptr<spdlog::sinks::sink> file_sink =
		  std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
		    LOG_FILENAME, LOG_MAX_SIZE, LOG_MAX_FILES);
		assert(file_sink != nullptr);
		file_sink->set_level(spdlog::level::trace);
		LOGGER->sinks().push_back(std::move(file_sink));

		// Console sink
		std::shared_ptr<spdlog::sinks::sink> console_sink =
		  std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		assert(console_sink != nullptr);
		console_sink->set_level(spdlog::level::trace);
		LOGGER->sinks().push_back(std::move(console_sink));

		// Configure logger
		LOGGER->set_level(spdlog::level::trace);
		LOGGER->flush_on(spdlog::level::err);
		spdlog::register_logger(LOGGER);
	}
	catch(const spdlog::spdlog_ex& ex)
	{
		std::cerr << "Logger initialization failed: " << ex.what() << std::endl;
		return false;
	}

	return true;
}
