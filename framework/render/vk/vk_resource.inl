// *************************************************************
// File:    vk_resource.inl
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "vk_resource.h"

#include <memory/defines.h>
#include <render/vk/vk_debug_interface.h>
#include <render/vk/vk_host_allocator.h>

//--public

// todo: add args
template<class T>
gdm::vk::BaseResourceBuilder<T>::BaseResourceBuilder(VkDevice device)
  : ptr_{ GMNew T{} }
  , device_{ device }
  , name_{ nullptr }
{ }

template<class T>
gdm::vk::BaseResourceBuilder<T>::~BaseResourceBuilder()
{
  if constexpr (!std::is_class_v<std::decay_t<decltype(ptr_->GetImpl())>>)
  {
    if (name_)
      debug::SetDebugName(device_, ptr_->GetImpl(), name_);
  }
}

template<class T>
auto gdm::vk::BaseResourceBuilder<T>::SetName(const char* name) -> gdm::vk::BaseResourceBuilder<T>::self
{
  name_ = name;
  return static_cast<self>(*this);
}
