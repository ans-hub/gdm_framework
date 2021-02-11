// *************************************************************
// File:    introspection.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "3rdparty/catch/catch.hpp"

#include <system/introspection.h>
#include <system/type_list.h>

namespace gdm::test {

	struct Aaa
	{
		int field1;
		int field2;
	};

} // namespace gdm::test

namespace gdm {

	template <>
	struct Spec<gdm::test::Aaa>
	{
		struct Field_field1
		{
			constexpr static inline auto PTR = &gdm::test::Aaa::field1;
			constexpr static inline const char* NAME = "field1";
		};
		struct Field_field2
		{
			constexpr static inline auto PTR = &gdm::test::Aaa::field2;
			constexpr static inline const char* NAME = "field2";
		};
		using Fields = TypeList<Field_field1, Field_field2>;
	};

} // namespace gdm

namespace gdm::test {

TEST_CASE("Introspection")
{
	SECTION("Base case")
	{
		Aaa a { 1, 2 };
		
		gdm::PrintHandler handler {};
		gdm::Visitor print_visitor(handler);
		print_visitor.Visit(Spec<Aaa>::Fields{}, a);

		auto fn_add = [](auto& value){ ++value; };

		gdm::AnyHandler any_handler(fn_add);
		gdm::Visitor any_visitor(any_handler);
		any_visitor.Visit(Spec<Aaa>::Fields{}, a);

		CHECK(a.field1 == 2);
		CHECK(a.field2 == 3);
	}
}

} // namespace gdm::test
