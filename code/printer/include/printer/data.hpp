//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_DATA_HPP
#define USCP_DATA_HPP

#include "printer/printer.hpp"

#include <nlohmann/json.hpp>

namespace uscp::data
{
	bool load(const nlohmann::json& data, printer& printer) noexcept;
}

#endif //USCP_DATA_HPP
