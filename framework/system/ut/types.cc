// *************************************************************
// File:    types.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "3rdparty/catch/catch.hpp"

#include <system/type_traits.h>
#include <system/rtti.h>

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
using HasAlias = typename T::AliasA;

template <class T>
using HasAliasA0 = decltype((void)std::declval<typename T::AliasA>);

template <class T>
using HasAliasA1 = decltype(std::declval<typename T::AliasA>());

template <class T>
using HasAliasB = decltype((void)std::declval<typename T::AliasB>);

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
		CHECK(gdm::v_IsDetected<HasAlias, A>);
		CHECK(gdm::v_IsDetected<HasAliasA0, A>);
		CHECK(gdm::v_IsDetected<HasAliasA1, A>);	
		CHECK(!gdm::v_IsDetected<HasAliasB, A>);
		CHECK(gdm::v_IsDetected<HasVar, A>);
		CHECK(gdm::v_IsDetected<HasStaticVarA, A>);
		CHECK(!gdm::v_IsDetected<HasStaticVarB, A>);
	}

	SECTION("RTTI check types")
	{
		CHECK(gdm::IsClass<gdm::test::Base>::BaseTo<gdm::test::DerivedA>());
		CHECK(gdm::IsClass<gdm::test::Base>::BaseTo<gdm::test::DerivedB>());
		CHECK(!gdm::IsClass<gdm::test::DerivedA>::BaseTo<gdm::test::Base>());
		CHECK(!gdm::IsClass<gdm::test::Base>::BaseTo<gdm::test::Any>());
	}

	SECTION("RTTI check instantiations (using template recursion)")
	{
		DerivedA derivedA;
		DerivedB derivedB;
		DerivedA* baseA = &derivedB;
		Base* base = &derivedB;
		Any any;

		CHECK(gdm::IsClass(*base).BaseTo<DerivedB>());
		CHECK(!gdm::IsClass(any).BaseTo<DerivedB>());
		CHECK(gdm::IsClass(*baseA).BaseTo<DerivedB>());
		CHECK(gdm::IsClass(derivedB).BaseTo<DerivedB>());
		CHECK(!gdm::IsClass(derivedA).BaseTo<Base>());
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

	SECTION("Variable name")
	{		
		extern int var_1;
		CHECK(gdm::GetInstanceName<decltype(var_1), var_1>() == "var_1");
	}
}

} // namespace gdm::test
