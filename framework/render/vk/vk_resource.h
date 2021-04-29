// *************************************************************
// File:    vk_resource.h
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include <memory>

#ifndef AH_GFX_RESOURCE_H
#define AH_GFX_RESOURCE_H

namespace gdm::vk {
  template<class T> struct Resource;
}

namespace gdm::vk {

template<class T>
struct BaseResourceBuilder
{
  using self = Resource<T>&;

  BaseResourceBuilder(VkDevice device);
  virtual ~BaseResourceBuilder();

  self SetName(const char* name);

  operator T*() { return ptr_; }
  operator std::unique_ptr<T>() { return std::unique_ptr<T>(ptr_); }

protected:
  T* ptr_;
  VkDevice device_;
  const char* name_;

}; // struct BaseResourceBuilder<T>

} // namespace gdm::vk

#include "vk_resource.inl"

#endif // AH_GFX_RESOURCE_H