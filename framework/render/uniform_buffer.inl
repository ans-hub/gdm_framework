
// *************************************************************
// File:    uniform_buffer.inl
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "uniform_buffer.h"

//--public

template<class T>
gdm::gfx::UniformBuffer<T>::UniformBuffer(uint count, bool mapped, api::Device* device, api::CommandList& cmd)
  : device_{device}
  , staging_buffer_{nullptr}
  , uniform_buffer_{nullptr}
  , to_read_barrier_{nullptr}
  , to_write_barrier_{nullptr}
  , mapped_{mapped}
{
  staging_buffer_ = api::Resource<api::Buffer>(device_, sizeof(T) * count)
    .AddUsage(gfx::TRANSFER_SRC)
    .AddMemoryType(gfx::HOST_VISIBLE);

  uniform_buffer_ = api::Resource<api::Buffer>(device_, sizeof(T) * count)
    .AddUsage(gfx::TRANSFER_DST | gfx::UNIFORM)
    .AddMemoryType(gfx::DEVICE_LOCAL);

  to_read_barrier_ = api::Resource<api::BufferBarrier>(device_)
    .AddBuffer(*uniform_buffer_)
    .AddOldAccess(gfx::EAccess::TRANSFER_WRITE)
    .AddNewAccess(gfx::EAccess::UNIFORM_READ);
  
  to_write_barrier_ = api::Resource<api::BufferBarrier>(device_)
    .AddBuffer(*staging_buffer_)
    .AddOldAccess(gfx::EAccess::UNIFORM_READ)
    .AddNewAccess(gfx::EAccess::TRANSFER_WRITE);

  cmd.PushBarrier(*to_read_barrier_);

  if (mapped_)
    staging_buffer_->Map();
}

template<class T>
void gdm::gfx::UniformBuffer<T>::Update(api::CommandList& cmd, const T* data, uint offset, uint count)
{
  assert(device_);

  cmd.PushBarrier(*to_write_barrier_);

  if (mapped_)
    staging_buffer_->CopyDataToGpu(data, offset, count);
  else
  {
    staging_buffer_->Map();
    staging_buffer_->CopyDataToGpu(data, offset, count);
    staging_buffer_->Unmap();
  }
  
  uint32_t size = sizeof(T) * count;

  cmd.CopyBufferToBuffer(*staging_buffer_, *uniform_buffer_, size);
  cmd.PushBarrier(*to_read_barrier_);
}