//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_TIMER_HPP
#define USCP_TIMER_HPP

#include <chrono>

class timer final
{
public:
	timer() noexcept;
	timer(const timer&) = default;
	timer(timer&&) noexcept = default;
	timer& operator=(const timer& other) = default;
	timer& operator=(timer&& other) noexcept = default;

	[[nodiscard]] double elapsed() const noexcept;
	void reset() noexcept;

private:
	std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<double>> start;
};

#endif //USCP_TIMER_HPP
