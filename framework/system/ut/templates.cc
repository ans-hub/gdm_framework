// *************************************************************
// File:    types.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "3rdparty/catch/catch.hpp"

#include <map>

#include "system/type_traits.h"
#include "system/type_id.h"

namespace gdm::test {

// Deduce template argument by return type

struct Data
{
	float val_;
};

template<class T>
auto RequestData() -> T;

template<>
auto RequestData() -> int { return 42; }

template<>
auto RequestData() -> float { return 13.f; }

template<>
auto RequestData() -> Data { return Data{10.f}; }

struct Wrapper
{
	template<class T>
	operator T() { return RequestData<T>(); }
};

auto RequestData() -> Wrapper
{
	return Wrapper{};
}

TEST_CASE("Deduce type depends on result type")
{
		int i = RequestData();
		float f = RequestData();
		Data d = RequestData();
	
		CHECK(i == 42);
		CHECK(fabs(f-13.f) <= 0.0001f);
		CHECK(fabs(d.val_-10.f) <= 0.0001f);
}

// Ops (untyped to typed mapping)

struct IOps
{
	virtual int GetTypeId() = 0;
	virtual const char* GetMessage() = 0;
};

template<class T>
struct Ops : IOps
{
	Ops(const char* message) : message_{message} { }

	virtual int GetTypeId() override { return gdm::TypeId<T>(); }
	virtual const char* GetMessage() override { return message_; }

private:
	const char* message_;
};

struct Registrator
{
	template<class T, class...Args>
	static bool RegisterOps(Args&&...args)
	{
		int runtime_id = gdm::TypeId<T>();
		IOps* ops = new Ops<T>(std::forward<Args>(args...));
		ops_[runtime_id] = ops;
	
		return true;
	}
	
	IOps* GetOps(int id) { return ops_[id]; }

	static std::map<int, IOps*> ops_;
};

std::map<int, IOps*> Registrator::ops_;

inline static bool v_register_type_A = Registrator::RegisterOps<int>("int");
inline static bool v_register_type_B = Registrator::RegisterOps<float>("float");

TEST_CASE("Bind dynamic id to template")
{
	Registrator r;
	auto* a = r.GetOps(gdm::TypeId<int>());
	auto* b = r.GetOps(gdm::TypeId<float>());
	CHECK(a->GetTypeId() == gdm::TypeId<int>());
	CHECK(b->GetTypeId() == gdm::TypeId<float>());
}

// Heterogen container

TEST_CASE("Heterogen container")
{

}

} // namespace gdm::test
