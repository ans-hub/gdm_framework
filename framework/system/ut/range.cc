// *************************************************************
// File:    range.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "3rdparty/catch/catch.hpp"

#include <system/diff_utils.h>

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
