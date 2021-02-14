// *************************************************************
// File:    range.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "3rdparty/catch/catch.hpp"

#include <system/diff_utils.h>

#include <span>
#include <array>
#include <vector>

using namespace gdm;

TEST_CASE("Enumaration")
{
	int i = 32;
	std::vector<int> v;
	for(int k = 0; k < 16; ++k)
		v.push_back(i++);
	i = 0;
	for (auto&& [index, value] : gdm::Enumerate(v))
	{
		CHECK(index == i);
		CHECK(value == i + 32);
		++i;
	}
}

TEST_CASE("Zip span")
{
	SECTION("Mutable vector")
	{
		std::vector<int> v1 { 1,2,3 };
		std::vector<int> v2 { 4,5,6 };

		for (auto&& [val1, val2] : gdm::range::ZipSpan(v1, v2))
		{
			CHECK(val2 - val1 == 3);
			val1 = 0;
			val2 = 0;
		}

		for (auto&& [val1, val2] : gdm::range::ZipSpan(std::span(v1), std::span(v2)))
		{
			CHECK(val1 == 0);
			CHECK(val2 == 0);
		}
	}
  
	SECTION("Immutable vector")
	{
		const std::vector<int> v1 { 1,2,3 };
		const std::vector<int> v2 { 4,5,6 };

		for (auto&& [val1, val2] : gdm::range::ZipSpan(v1, v2))
		{
			CHECK(std::is_const_v<std::remove_reference_t<decltype(val1)>>);
			CHECK(std::is_const_v<std::remove_reference_t<decltype(val2)>>);
			CHECK(val2 - val1 == 3);
		}
	}
}
