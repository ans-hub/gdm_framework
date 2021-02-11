// *************************************************************
// File:    type_id.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_TYPE_ID_H
#define AH_GDM_TYPE_ID_H

namespace gdm {

	template<typename T>
	struct TypeId
	{
    operator int() { return index_; }
    operator int() const { return index_; }

	private:
		static int index_;
	};

} // namespace gdm

#include "type_id.inl"

#endif // AH_GDM_TYPE_ID_H