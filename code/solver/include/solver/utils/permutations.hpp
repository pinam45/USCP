//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_PERMUTATIONS_HPP
#define USCP_PERMUTATIONS_HPP

#include <dynamic_bitset.hpp>

#include <memory>
#include <cstddef>
#include <vector>

std::vector<std::vector<dynamic_bitset<>>> generate_permutations(size_t bitsets_size);

class PermutationsGenerator
{
public:
	PermutationsGenerator(size_t bitset_size, size_t ones_number) noexcept;

	[[nodiscard]] size_t bitset_size() const noexcept;
	[[nodiscard]] size_t ones_number() const noexcept;

	void reset() noexcept;
	void reset(size_t ones_number) noexcept;

	const dynamic_bitset<>& next() noexcept;

	[[nodiscard]] bool finished() noexcept;

private:
	bool m_finished;
	bool m_bit_to_add;
	const size_t m_bitset_size;
	size_t m_ones_number;
	dynamic_bitset<> m_current;
	std::unique_ptr<PermutationsGenerator> m_lower_permutation;
};

void increment(dynamic_bitset<>& bitset);

#endif //USCP_PERMUTATIONS_HPP
