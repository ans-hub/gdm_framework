// *************************************************************
// File:    hash_ut.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "3rdparty/catch/catch.hpp"

#include <system/diff_utils.h>
#include <system/hash_utils.h>
#include <system/string_utils.h>

using namespace gdm;

template <size_t Value>
struct A
{ 
  constexpr static size_t v_value = Value;
};

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

TEST_CASE("Strings")
{
  SECTION("Split string")
  {
    const char* str = "hello";
		auto res = str::Split(str, 0, (int)strlen(str), ' ');
		CHECK(std::string(res[0].begin, res[0].end) == "hello");
		CHECK(res.size() == 1);
	}
	SECTION("Split string 2")
  {
    const char* str = "The next";
		auto res = str::Split(str, 0, (int)strlen(str), ' ');
		CHECK(res.size() == 2);
		CHECK(std::string(res[1].begin, res[1].end) == "next");
	}
	SECTION("Split string 3")
  {
    const char* str = " The next";
		auto res = str::Split(str, 0, (int)strlen(str), ' ');
		CHECK(std::string(res[0].begin, res[0].end) == "The");
		CHECK(res.size() == 2);
	}
	SECTION("Split string 4")
  {
    const char* str = "The next ";
		auto res = str::Split(str, 0, (int)strlen(str), ' ');
		CHECK(std::string(res[0].begin, res[0].end) == "The");
		CHECK(res.size() == 2);
	}
	SECTION("Split string 5")
  {
    const char* str = " The next ";
		auto res = str::Split(str, 0, (int)strlen(str), ' ');
		CHECK(std::string(res[0].begin, res[0].end) == "The");
		CHECK(res.size() == 2);
	}
	SECTION("Split string 6")
  {
    const char* str = "The ";
		auto res = str::Split(str, 0, (int)strlen(str), ' ');
		CHECK(std::string(res[0].begin, res[0].end) == "The");
		CHECK(res.size() == 1);
	}
	SECTION("Split string 7")
  {
    const char* str = " The";
		auto res = str::Split(str, 0, (int)strlen(str), ' ');
		CHECK(std::string(res[0].begin, res[0].end) == "The");
		CHECK(res.size() == 1);
	}
	SECTION("Split string 8")
  {
    const char* str = " The ";
		auto res = str::Split(str, 0, (int)strlen(str), ' ');
		CHECK(res.size() == 1);
	}
	SECTION("Split string 9")
  {
    const char* str = " Most The Diff  A ";
		int begin = 2;
		auto res = str::Split(str, begin, (int)strlen(str), ' ');
		CHECK(res.size() == 4);
		CHECK(std::string(res[0].begin, res[0].end) == "ost");
	}
	SECTION("Split string 10")
  {
    const char* str = " Most The Diff  A   ";
		auto res = str::Split(str, 0, (int)strlen(str), ' ');
		CHECK(std::string(res[2].begin, res[2].end) == "Diff");
		CHECK(res.size() == 4);
	}
	SECTION("Split string 11")
  {
    const char* str = " Most The Diff  A   ";
		auto res = str::Split(str, 1, 5, ' ');
		CHECK(std::string(res[0].begin, res[0].end) == "Most");
		CHECK(res.size() == 1);
	}
	SECTION("Split string 12")
  {
    const char* str = " Most The Diff  A   ";
		auto res = str::Split(str, 0, (int)strlen(str), ' ');
		CHECK(std::string(res[3].begin, res[3].end) == "A");
		CHECK(res.size() == 4);
	}
	SECTION("Split string 13")
  {
    const char* str = "v1//v2";
		auto res = str::Split(str, '/', true);
		CHECK(std::string(res[0].begin, res[0].end) == "v1");
		CHECK(std::string(res[1].begin, res[1].end) == "");
		CHECK(std::string(res[2].begin, res[2].end) == "v2");
		CHECK(res.size() == 3);
	}
	SECTION("Split string 14")
  {
    const char* str = "Kd 1 0.84 0.1";
		auto res = str::Split(str, ' ');
		CHECK(std::string(res[0].begin, res[0].end) == "Kd");
		CHECK(std::string(res[1].begin, res[1].end) == "1");
		CHECK(std::string(res[2].begin, res[2].end) == "0.84");
		CHECK(std::string(res[3].begin, res[3].end) == "0.1");
		CHECK(res.size() == 4);
	}
}
