// *************************************************************
// File:    introspection.inl
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "introspection.h"

//--public

struct gdm::FieldInfo
{
  const char* name;
};

template<class Handler>
template<class T, class...Fields>
inline void gdm::Visitor<Handler>::Visit(TypeList<Fields...>, T& object)
{
	(this->Visit<T, Fields>(object), ...);
}

template<class T>
void gdm::PrintHandler::Handle(T& object, const FieldInfo& info)
{
  printf("%d %s\n", object, info.name);
}

template<class Fn>
template<class T>
void gdm::AnyHandler<Fn>::Handle(T& object, const FieldInfo& info)
{
  fn_(object);
}

// --private

template<class Handler>
template<class T, class Field>
inline void gdm::Visitor<Handler>::Visit(T& object)
{
  FieldInfo info { Field::NAME };
	auto& val = object.*Field::PTR;

	this->VisitField(val, info);
}

template<class Handler>
template<class T>
inline void gdm::Visitor<Handler>::VisitField(T& value, const FieldInfo& info)
{
	if (std::is_same_v<T, int>)
	{
    handler_.Handle(value, info);
	}
}
