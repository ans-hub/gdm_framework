// *************************************************************
// File:    introspection.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_INTROSPECTION_H
#define AH_GDM_INTROSPECTION_H

#include "system/type_list.h"

namespace gdm {

  template <class T>
  struct Spec;

  struct FieldInfo;

  template<class Handler>
  struct Visitor
  {
    Visitor(Handler& handler) : handler_{handler} { }
    
    template<class T, class...Fields>
    void Visit(TypeList<Fields...>, T& object);

  private:
    template<class T, class Field>
    void Visit(T& object);

    template<class T>
    void VisitField(T& value, const FieldInfo& info);

  private:
    Handler& handler_;
  
  }; // struct Visitor

  struct PrintHandler
  {
    PrintHandler() = default;

    template<class T>
    void Handle(T& object, const FieldInfo& info);

  }; // struct PrintHandler 

  template<class Fn>
  struct AnyHandler
  {
    AnyHandler(Fn fn) : fn_{fn} { }

    template<class T>
    void Handle(T& object, const FieldInfo& info);
  
  private:
    Fn fn_;
  
  }; // struct AnyHandler

} // namespace gdm

#include "introspection.inl"

#endif // AH_GDM_INTROSPECTION_H
