
// *************************************************************
// File:    uniform_buffer.inl
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "uniform_buffer.h"

//--public

template<class T>
gdm::gfx::UniformBuffer<T>::UniformBuffer(uint count, api::Device* device, api::CommandList& cmd)
  : device_{device}
{
  staging_buffer_ = gfx::Resource<api::Buffer>(device_, sizeof(T) * count)
    .AddUsage(gfx::TRANSFER_SRC)
    .AddMemoryType(gfx::HOST_VISIBLE);

  uniform_buffer_ = gfx::Resource<api::Buffer>(device_, sizeof(T) * count)
    .AddUsage(gfx::TRANSFER_DST | gfx::UNIFORM)
    .AddMemoryType(gfx::DEVICE_LOCAL);

  to_read_barrier_ = gfx::Resource<api::BufferBarrier>(device_)
    .AddBuffer(*uniform_buffer_)
    .AddOldAccess(gfx::EAccess::TRANSFER_WRITE)
    .AddNewAccess(gfx::EAccess::UNIFORM_READ);
  
  to_write_barrier_ = gfx::Resource<api::BufferBarrier>(device_)
    .AddBuffer(*staging_buffer_)
    .AddOldAccess(gfx::EAccess::UNIFORM_READ)
    .AddNewAccess(gfx::EAccess::TRANSFER_WRITE);

  cmd.PushBarrier(*to_read_barrier_);
}

template<class T>
void gdm::gfx::UniformBuffer<T>::Update(api::CommandList& cmd, const T* data, uint offset, uint count)
{
  assert(device_);

  cmd.PushBarrier(*to_write_barrier_);

  staging_buffer_->Map();
  staging_buffer_->CopyDataToGpu(data, offset, count);
  staging_buffer_->Unmap();
  
  cmd.CopyBufferToBuffer(*staging_buffer_, *uniform_buffer_, Size);
  cmd.PushBarrier(*to_read_barrier_);
}