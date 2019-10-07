//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#include "solver/utils/permutations.hpp"

std::vector<std::vector<dynamic_bitset<>>> generate_permutations(size_t bitsets_size)
{
	// permutations[k] : list of bitset permutations with k bit to 1
	std::vector<std::vector<dynamic_bitset<>>> permutations;
	permutations.reserve(bitsets_size);
	permutations.emplace_back();
	permutations[0].emplace_back();

	for(size_t bitset_size = 0; bitset_size < bitsets_size; ++bitset_size)
	{
		// generate bitsets of size (bitset_size + 1)
		// reverse order

		// special last case (all bits to 1)
		permutations.emplace_back();
		permutations[permutations.size() - 1] = permutations[permutations.size() - 2];
		for(dynamic_bitset<>& permutation: permutations[permutations.size() - 1])
		{
			permutation.push_back(true);
		}

		// general case
		for(size_t i = (permutations.size() - 2); i > 0; --i)
		{
			const size_t old_size = permutations[i].size();
			permutations[i].reserve(old_size + permutations[i - 1].size());
			std::copy(std::cbegin(permutations[i - 1]),
			          std::cend(permutations[i - 1]),
			          std::back_inserter(permutations[i]));
			for(size_t j = 0; j < old_size; ++j)
			{
				permutations[i][j].push_back(false);
			}
			for(size_t j = old_size; j < permutations[i].size(); ++j)
			{
				permutations[i][j].push_back(true);
			}
		}

		// special first case (all bits to 0)
		for(dynamic_bitset<>& permutation: permutations[0])
		{
			permutation.push_back(false);
		}
	}

	return permutations;
}

PermutationsGenerator::PermutationsGenerator(size_t bitset_size, size_t ones_number) noexcept
  : m_finished(false)
  , m_bit_to_add(false)
  , m_bitset_size(bitset_size)
  , m_ones_number(ones_number)
  , m_current(bitset_size)
  , m_lower_permutation((bitset_size > ones_number) && (ones_number > 0)
                          ? std::make_unique<PermutationsGenerator>(bitset_size - 1, ones_number)
                          : nullptr)
{
}

size_t PermutationsGenerator::bitset_size() const noexcept
{
	return m_bitset_size;
}

size_t PermutationsGenerator::ones_number() const noexcept
{
	return m_ones_number;
}

void PermutationsGenerator::reset() noexcept
{
	m_finished = false;
	m_bit_to_add = false;
	m_current.reset();
}

void PermutationsGenerator::reset(size_t ones_number) noexcept
{
	assert(ones_number < m_bitset_size);
	m_finished = false;
	m_bit_to_add = false;
	m_ones_number = ones_number;
	m_current.reset();

	if((m_bitset_size > m_ones_number) && (m_ones_number > 0))
	{
		m_lower_permutation =
		  std::make_unique<PermutationsGenerator>(m_bitset_size - 1, m_ones_number);
	}
}

const dynamic_bitset<>& PermutationsGenerator::next() noexcept
{
	if(m_ones_number == 0) // only 0
	{
		m_finished = true;
	}
	else if(m_bitset_size == m_ones_number) // only 1
	{
		m_current.set();
		m_finished = true;
	}
	else // general case
	{
		assert(m_lower_permutation != nullptr);
		m_current = m_lower_permutation->next();
		m_current.push_back(m_bit_to_add);

		if(m_lower_permutation->finished())
		{
			if(m_bit_to_add)
			{
				m_finished = true;
			}
			else
			{
				m_bit_to_add = true;
				m_lower_permutation->reset(m_ones_number - 1);
			}
		}
	}
	return m_current;
}

bool PermutationsGenerator::finished() noexcept
{
	return m_finished;
}

void increment(dynamic_bitset<>& bitset)
{
	for(size_t i = 0; i < bitset.size(); ++i)
	{
		if(bitset.test(i))
		{
			bitset.reset(i);
		}
		else
		{
			bitset.set(i);
			break;
		}
	}
}
