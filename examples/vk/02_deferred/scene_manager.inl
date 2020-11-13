// *************************************************************
// File:    scene_manager.inl
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "scene_manager.h"

#include "system/assert_utils.h"

// --public

template<class Ubo, class PassData>
inline void gdm::SceneManager::CreateUbo(api::CommandList& cmd, PassData& pass, uint count)
{
  auto* staging = GMNew api::Buffer(&device_, sizeof(Ubo) * count, gfx::TRANSFER_SRC, gfx::HOST_VISIBLE);
  auto* uniform = GMNew api::Buffer(&device_, sizeof(Ubo) * count, gfx::TRANSFER_DST | gfx::UNIFORM, gfx::DEVICE_LOCAL);
  auto* to_read_barrier = GMNew api::BufferBarrier(&device_, *uniform, gfx::EAccess::TRANSFER_WRITE, gfx::EAccess::UNIFORM_READ);
  auto* to_write_barrier = GMNew api::BufferBarrier(&device_, *uniform, gfx::EAccess::UNIFORM_READ, gfx::EAccess::TRANSFER_WRITE);
  
  cmd.PushBarrier(*to_read_barrier);
  
  if constexpr (Ubo::v_type_ == gfx::EUboType::PER_FRAME)
  {
    pass.pfcb_uniform_ = uniform;
    pass.pfcb_staging_ = staging;
    pass.pfcb_to_read_barrier_ = to_read_barrier;
    pass.pfcb_to_write_barrier_ = to_write_barrier;
  }
  else if constexpr (Ubo::v_type_ == gfx::EUboType::PER_OBJECT)
  {
    pass.pocb_uniform_ = uniform;
    pass.pocb_staging_ = staging;
    pass.pocb_to_read_barrier_ = to_read_barrier;
    pass.pocb_to_write_barrier_ = to_write_barrier;
    bool map_and_never_unmap = [&](){ pass.pocb_staging_->Map(); return true; }();
  }
  else
  {
    ASSERTF(false, "Such type of ubo is not implemented");
  }
}

template<class Ubo, class PassData>
inline void gdm::SceneManager::UpdateUBO(api::CommandList& cmd, PassData& pass, uint count)
{
  if constexpr (Ubo::v_type_ == gfx::EUboType::PER_FRAME)
  {
    cmd.PushBarrier(*pass.pfcb_to_write_barrier_);
    pass.pfcb_staging_->Map();
    pass.pfcb_staging_->CopyDataToGpu(&pass.pfcb_data_, 1);
    pass.pfcb_staging_->Unmap();
    cmd.CopyBufferToBuffer(*pass.pfcb_staging_, *pass.pfcb_uniform_, sizeof(Ubo));
    cmd.PushBarrier(*pass.pfcb_to_read_barrier_); 
  }
  else if constexpr (Ubo::v_type_ == gfx::EUboType::PER_OBJECT)
  {
    cmd.PushBarrier(*pass.pocb_to_write_barrier_);
    pass.pocb_staging_->CopyDataToGpu(pass.pocb_data_.data(), pass.pocb_data_.size());
    uint pocb_size = static_cast<uint>(sizeof(Ubo) * pass.pocb_data_.size());
    cmd.CopyBufferToBuffer(*pass.pocb_staging_, *pass.pocb_uniform_, pocb_size);
    cmd.PushBarrier(*pass.pocb_to_read_barrier_);
  }
  else
  {
    ASSERTF(false, "Such type of ubo is not implemented");
  }
}
