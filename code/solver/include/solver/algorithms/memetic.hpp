//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_MEMETIC_HPP
#define USCP_MEMETIC_HPP

#include "common/data/instance.hpp"
#include "common/data/solution.hpp"
#include "common/utils/random.hpp"
#include "common/algorithms//memetic.hpp"
#include "solver/algorithms/rwls.hpp"

#include <cstddef>
#include <limits>
#include <string>

namespace uscp::memetic
{
	struct position final
	{
		size_t generation = std::numeric_limits<size_t>::max();
		uscp::rwls::position rwls_cumulative_position;
		double time = std::numeric_limits<double>::max();

		[[nodiscard]] position_serial serialize() const noexcept;
		bool load(const position_serial& serial) noexcept;
	};

	struct config final
	{
		position stopping_criterion;
		uscp::rwls::position rwls_stopping_criterion;

		[[nodiscard]] config_serial serialize() const noexcept;
		bool load(const config_serial& serial) noexcept;
	};

	struct report final
	{
		solution solution_final;
		std::vector<ssize_t> points_weights_final;
		position found_at;
		position ended_at;
		config solve_config;
		std::string crossover_operator;
		std::string wcrossover_operator;

		explicit report(const problem::instance& problem) noexcept;
		report(const report&) = default;
		report(report&&) noexcept = default;
		report& operator=(const report& other) = default;
		report& operator=(report&& other) noexcept = default;

		[[nodiscard]] report_serial serialize() const noexcept;
		bool load(const report_serial& serial) noexcept;
	};

	template<typename Crossover, typename WeightsCrossover>
	class memetic final
	{
	public:
		explicit memetic(const problem::instance& problem) noexcept;
		memetic(const memetic&) = default;
		memetic(memetic&&) noexcept = default;
		memetic& operator=(const memetic& other) = delete;
		memetic& operator=(memetic&& other) noexcept = delete;

		[[gnu::hot]] void initialize() noexcept;
		[[gnu::hot]] report solve(random_engine& generator, const config& config) noexcept;

	private:
		template<size_t v>
		struct dependent_false : public std::false_type
		{
		};

		const uscp::problem::instance& m_problem;
		Crossover m_crossover;
		WeightsCrossover m_weights_crossover;
		uscp::rwls::rwls m_rwls;
		bool m_initialized;
	};

	[[nodiscard]] report expand(const report& reduced_report) noexcept;
} // namespace uscp::memetic

#include "solver/algorithms/memetic.tpp"

#endif //USCP_MEMETIC_HPP
