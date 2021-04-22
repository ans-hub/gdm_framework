// *************************************************************
// File:    uniform_buffer.h
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GFX_UNIFORM_BUFFER_H
#define GFX_UNIFORM_BUFFER_H

#include "render/api.h"
#include "render/defines.h"
#include "render/renderer.h"

namespace gdm::gfx {

  template <class T>
  struct UniformBuffer
  {
    UniformBuffer(const UniformBuffer&) =delete;
    UniformBuffer& operator=(const UniformBuffer&) =delete;

    UniformBuffer(uint count, bool mapped, api::Device* device, api::CommandList& cmd);

    void Update(api::CommandList& cmd, const T* data, uint offset, uint count);

    auto GetImpl() const -> const api::Buffer& { return *uniform_buffer_.get(); }
    auto GetImpl() -> api::Buffer& { return *uniform_buffer_.get(); }

    auto GetToReadBarrier() -> api::BufferBarrier* { return to_read_barrier_.get(); }
    auto GetToWriteBarrier() -> api::BufferBarrier* { return to_write_barrier_.get(); }

  private:
    api::Device* device_;
    std::unique_ptr<api::Buffer> staging_buffer_;
    std::unique_ptr<api::Buffer> uniform_buffer_;
    std::unique_ptr<api::BufferBarrier> to_read_barrier_;
    std::unique_ptr<api::BufferBarrier> to_write_barrier_;
    bool mapped_;
  };

} // namespace gdm

#include "uniform_buffer.inl"

#endif // GFX_UNIFORM_BUFFER_H
