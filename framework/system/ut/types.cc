// *************************************************************
// File:    types.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "3rdparty/catch/catch.hpp"

#include <system/type_traits.h>

namespace gdm::test {

struct A
{
	using AliasA = int;

	bool Some();

	int var;
	static int static_var;
};

struct Base
{
	RTTI_ROOT(Base)
};

struct DerivedA : Base
{
	RTTI_DECL(Base, DerivedA)
};

struct DerivedB : DerivedA
{
	RTTI_DECL(DerivedA, DerivedB)
};

struct Any
{
	RTTI_ROOT(Any)
};

template <class T>
using HasSome = decltype(std::declval<T>().Some());

template <class T>
using HasOther = decltype(std::declval<T>().Other());

template <class T>
using HasAliasA = decltype((void)std::declval<T::AliasA>);

template <class T>
using HasAliasB = decltype((void)std::declval<T::AliasB>);

template <class T>
using HasVar = decltype(std::declval<T>().var);

template <class T>
using HasStaticVarA = decltype((void)std::declval<T::static_var>);

template <class T>
using HasStaticVarB = decltype((void)std::declval<T::static_null>);

TEST_CASE("Type traits")
{
	SECTION("Detected")
	{
		CHECK(gdm::v_IsDetected<HasSome, A>);
		CHECK(!gdm::v_IsDetected<HasOther, A>);
		CHECK(gdm::v_IsDetected<HasAliasA, A>);
		CHECK(!gdm::v_IsDetected<HasAliasB, A>);
		CHECK(gdm::v_IsDetected<HasVar, A>);
		CHECK(gdm::v_IsDetected<HasStaticVarA, A>);
		CHECK(!gdm::v_IsDetected<HasStaticVarB, A>);
	}

	SECTION("RTTI check types")
	{
		CHECK(gdm::IsClass<gdm::test::Base>::Of<gdm::test::DerivedA>());
		CHECK(gdm::IsClass<gdm::test::Base>::Of<gdm::test::DerivedB>());
		CHECK(!gdm::IsClass<gdm::test::DerivedA>::Of<gdm::test::Base>());
		CHECK(!gdm::IsClass<gdm::test::Base>::Of<gdm::test::Any>());
	}

	SECTION("RTTI check instantiations (using template recursion)")
	{
		DerivedB derived;
		Base* base = &derived;
		Any any;

		CHECK(gdm::IsClass(*base).Of<DerivedB>());
		CHECK(!gdm::IsClass(any).Of<DerivedB>());
	}

	SECTION("RTTI check instantiations (using virtual methods)")
	{
		DerivedB derived;
		Base* base = &derived;
		Any any;

		CHECK(base->Is<DerivedB>());
		CHECK(!any.Is<DerivedB>());
		CHECK(base->As<DerivedB>().Who() == gdm::TypeId<DerivedB>());
	}
}

} // namespace gdm::test
