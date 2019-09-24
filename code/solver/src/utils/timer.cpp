//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include "utils/timer.hpp"

timer::timer() noexcept:start(std::chrono::system_clock::now())
{
}

double timer::elapsed() const noexcept
{
	const std::chrono::duration<double> elapsed = std::chrono::system_clock::now() - start;
	return elapsed.count();
}

void timer::reset() noexcept
{
	start = std::chrono::system_clock::now();
}
