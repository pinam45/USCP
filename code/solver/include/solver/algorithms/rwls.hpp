//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_RWLS_HPP
#define USCP_RWLS_HPP

#include "common/data/instance.hpp"
#include "common/data/solution.hpp"
#include "common/utils/random.hpp"
#include "common/algorithms/rwls.hpp"
#include "common/utils/logger.hpp"

#include <nlohmann/json.hpp>

#include <cstddef>
#include <limits>
#include <deque>
#include <vector>
#include <deque>

//#define USCP_RWLS_LOW_MEMORY_FOOTPRINT

namespace uscp::rwls
{
	constexpr size_t TABU_LIST_LENGTH = 2;

	struct position final
	{
		size_t steps = std::numeric_limits<size_t>::max();
		double time = std::numeric_limits<double>::max();

		position& operator+=(const position& other) noexcept;

		[[nodiscard]] position_serial serialize() const noexcept;
		bool load(const position_serial& serial) noexcept;
	};
	position operator+(const position& lhs, const position& rhs) noexcept;
	bool operator<(const position& lhs, const position& rhs) noexcept;

	struct report final
	{
		solution solution_initial;
		solution solution_final;
		position found_at;
		position ended_at;
		position stopping_criterion;

		explicit report(const problem::instance& problem) noexcept;
		report(const report&) = default;
		report(report&&) noexcept = default;
		report& operator=(const report& other) = default;
		report& operator=(report&& other) noexcept = default;

		[[nodiscard]] report_serial serialize() const noexcept;
		bool load(const report_serial& serial) noexcept;
	};

	class rwls final
	{
	public:
		explicit rwls(const problem::instance& problem,
		              std::shared_ptr<spdlog::logger> logger = LOGGER) noexcept;
		rwls(const rwls&) = default;
		rwls(rwls&&) noexcept = default;
		rwls& operator=(const rwls& other) = delete;
		rwls& operator=(rwls&& other) noexcept = delete;

		[[gnu::hot]] void initialize() noexcept;
		[[nodiscard, gnu::hot]] report improve(const uscp::solution& solution,
		                                       random_engine& generator,
		                                       position stopping_criterion) noexcept;

	private:
		struct point_information final // row
		{
			ssize_t weight = 1;
			size_t subsets_covering_in_solution = 0;
		};

		struct subset_information final // column
		{
			ssize_t score = 0;
			ssize_t timestamp = 1;
			bool canAddToSolution = true;
		};

		struct resolution_data final
		{
			random_engine& generator;
			solution& best_solution;
			solution current_solution;
			dynamic_bitset<> uncovered_points; // RWLS name: L
			std::vector<point_information> points_information;
			std::vector<subset_information> subsets_information;
			std::deque<size_t> tabu_subsets;

			dynamic_bitset<> points_tmp1;
			dynamic_bitset<> points_tmp2;

			explicit resolution_data(solution& solution, random_engine& generator) noexcept;
		};

		[[gnu::hot]] ssize_t compute_subset_score(const resolution_data& data,
		                                          size_t subset_number) noexcept;
		[[gnu::hot]] void init(resolution_data& data) noexcept;

		[[gnu::hot]] void add_subset(resolution_data& data, size_t subset_number) noexcept;
		[[gnu::hot]] void remove_subset(resolution_data& data, size_t subset_number) noexcept;

		[[gnu::hot]] void make_tabu(resolution_data& data, size_t subset_number) noexcept;
		[[nodiscard, gnu::hot]] bool is_tabu(const resolution_data& data,
		                                     size_t subset_number) noexcept;

		[[nodiscard, gnu::hot]] static size_t select_subset_to_remove_no_timestamp(
		  const resolution_data& data) noexcept;
		[[nodiscard, gnu::hot]] size_t select_subset_to_remove(
		  const resolution_data& data) noexcept;
		[[nodiscard, gnu::hot]] size_t select_subset_to_add(const resolution_data& data,
		                                                    size_t point_to_cover) noexcept;
		[[nodiscard, gnu::hot]] static size_t select_uncovered_point(
		  resolution_data& data) noexcept;

		const uscp::problem::instance& m_problem;
		std::vector<std::vector<size_t>> m_subsets_points;
		std::vector<std::vector<size_t>> m_subsets_covering_points;
		bool m_initialized;
		const std::shared_ptr<spdlog::logger> m_logger;
	};

	[[nodiscard, gnu::hot]] solution improve(const uscp::solution& solution_initial,
	                                         random_engine& generator,
	                                         position stopping_criterion);
	[[nodiscard, gnu::hot]] report improve_report(const uscp::solution& solution_initial,
	                                              random_engine& generator,
	                                              position stopping_criterion);

	[[nodiscard]] report expand(const report& reduced_report) noexcept;
} // namespace uscp::rwls

#endif //USCP_RWLS_HPP
