// *************************************************************
// File:    vk_buffer.inl
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "vk_buffer.h"

#include "vk_device.h"

#include "memory/helpers.h"
#include "system/assert_utils.h"
#include "system/bits_utils.h"

// --public

template<class T>
inline void gdm::vk::Buffer::CopyDataToGpu(const T* data, uint offset, size_t count)
{
  CopyDataToGpu(static_cast<const void*>(data), offset, count * sizeof(T));
}
