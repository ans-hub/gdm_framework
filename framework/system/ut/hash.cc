// *************************************************************
// File:    hash.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "3rdparty/catch/catch.hpp"

#include <system/hash_utils.h>

namespace gdm::test_hash {

template <size_t Value>
struct A
{ 	
  constexpr static size_t v_value = Value;
};

TEST_CASE("Hash")
{
  SECTION("Compile time hashing")
  {
    A<hash::Djb2CompileTime("SomeString")> a;
    A<GDM_HASH("AnotherString")> b;
    CHECK(a.v_value == hash::Djb2CompileTime("SomeString"));
    CHECK(b.v_value == hash::Djb2CompileTime("AnotherString"));
  }

	SECTION("Compile time hashing with dynamic tail")
	{
		constexpr auto hash = GDM_HASH("String");
		constexpr Hash hash2 = GDM_HASH_N("String", 2);
		constexpr uint64_t hash3 = GDM_HASH_S("String", "any");
		constexpr uint64_t hash4 = GDM_HASH_S("String", "1");
		constexpr uint64_t hash5 = GDM_HASH_N("String", 1);
		CHECK(hash != hash2);
		CHECK(hash != hash3);
		CHECK(hash != hash4);
		CHECK(hash3 != hash4);
		CHECK(hash2 != hash4);
		CHECK(hash4 != hash5);
		for (int i = 0; i < 3; ++i)
		{
			Hash hash5 = GDM_HASH_N("String", i);
			CHECK(hash5 == hash + i);
		}
	}
}

} // namespace gdm::test_hash
