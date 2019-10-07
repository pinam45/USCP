//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_INSTANCES_HPP
#define USCP_INSTANCES_HPP

#include "common/data/instances.hpp"

namespace uscp::problem
{
	bool read(const uscp::problem::instance_info& info, uscp::problem::instance& instance) noexcept;

	bool check_instances() noexcept;
} // namespace uscp::problem

#endif //USCP_INSTANCES_HPP
