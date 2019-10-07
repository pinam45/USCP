//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_LOGGER_HPP
#define USCP_LOGGER_HPP

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h> // for operator<<

#include <filesystem>

extern const std::shared_ptr<spdlog::logger> LOGGER;
extern const std::shared_ptr<spdlog::logger> NULL_LOGGER;

bool init_logger();

inline std::ostream& operator<<(std::ostream& os, const std::filesystem::path& p)
{
	return os << '\"' << p.generic_string() << '\"';
}

#endif //USCP_LOGGER_HPP
