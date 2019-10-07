//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_FORMAT_HPP
#define USCP_FORMAT_HPP

#include <type_traits>
#include <string>
#include <sstream>
#include <iomanip>

template<typename T, std::enable_if_t<std::is_floating_point_v<std::decay_t<T>>, int> = 0>
[[nodiscard]] inline std::string format(T&& value, int precision = 2) noexcept
{
	std::ostringstream stream;
	stream << std::fixed << std::setprecision(precision) << std::forward<T>(value);
	return stream.str();
}

[[nodiscard]] inline std::string format(bool value) noexcept
{
	std::ostringstream stream;
	stream << std::boolalpha << value;
	return stream.str();
}

#endif //USCP_FORMAT_HPP
