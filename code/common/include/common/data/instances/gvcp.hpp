//
// Copyright (c) 2020 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_GVCP_HPP
#define USCP_GVCP_HPP

#include "common/data/instance.hpp"

#include <filesystem>

namespace uscp::problem::gvcp
{
	bool read(const std::filesystem::path& base_path, uscp::problem::instance& instance) noexcept;

	bool write(const uscp::problem::instance& instance,
	           const std::filesystem::path& path,
	           bool override_file = false) noexcept;
} // namespace uscp::problem::gvcp

#endif //USCP_GVCP_HPP
