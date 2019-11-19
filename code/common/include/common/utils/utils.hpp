//
// Copyright (c) 2019 Maxime Pinard
//
// Distributed under the MIT license
// See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT
//
#ifndef USCP_UTILS_HPP
#define USCP_UTILS_HPP

#include <vector>

template<typename T>
size_t count_common_elements_sorted(const std::vector<T>& a, const std::vector<T>& b)
{
	size_t i_a = 0;
	size_t i_b = 0;
	size_t count = 0;
	while(i_a < a.size() && i_b < b.size())
	{
		if(a[i_a] < b[i_b])
		{
			++i_a;
		}
		else
		{
			if(b[i_b] == a[i_a])
			{
				++count;
			}
			++i_b;
		}
	}
	return count;
}

template<typename Block, typename Lambda>
void foreach_masked(const std::vector<size_t>& elements,
                    const dynamic_bitset<Block>& mask,
                    Lambda&& lambda)
{
	for(size_t element: elements)
	{
		assert(element < mask.size());
		if(mask.test(element))
		{
			lambda(element);
		}
	}
}

template<typename T, typename Lambda>
void apply_on_common_elements_sorted(const std::vector<T>& a,
                                     const std::vector<T>& b,
                                     Lambda&& lambda)
{
	size_t i_a = 0;
	size_t i_b = 0;
	while(i_a < a.size() && i_b < b.size())
	{
		if(a[i_a] < b[i_b])
		{
			++i_a;
		}
		else
		{
			if(b[i_b] == a[i_a])
			{
				lambda(b[i_b]);
			}
			++i_b;
		}
	}
}

#endif //USCP_UTILS_HPP
