// *************************************************************
// File:    type_id.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_TYPE_ID_H
#define AH_GDM_TYPE_ID_H

#include <type_traits>

namespace gdm {

	auto GetCounterGlobal() -> int&;

	namespace _private
	{
		template<class T>
		struct TypeId
		{
	    operator int() const { static int v = ++GetCounterGlobal(); return v; }	
		};
	}

	template<class T>
	struct TypeId
	{
		operator int() const { static int v = gdm::_private::TypeId<std::decay_t<T>>(); return v; }
	};

} // namespace gdm

#endif // AH_GDM_TYPE_ID_H