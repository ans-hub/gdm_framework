// *************************************************************
// File:    scene_manager.inl
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "scene_manager.h"

#include "data/cfg_loader.h"
#include "memory/defines.h"
#include "render/api.h"
#include "system/assert_utils.h"

// --public

template<class T, uint Count>
inline void gdm::SceneManager::CreatePerObjectUBO(api::CommandList& cmd)
{
  auto* pocb_buf = GMNew api::Buffer(&device_, sizeof(T) * Count, gfx::TRANSFER_DST | gfx::UNIFORM, gfx::DEVICE_LOCAL);
  pocb_uniform_.push_back(pocb_buf);
  auto* pocb_staging_buf = GMNew api::Buffer(&device_, sizeof(T) * Count, gfx::TRANSFER_SRC, gfx::HOST_VISIBLE); // and COHERENT too?
  pocb_staging_.push_back(pocb_staging_buf);
  auto* pocb_to_read_barrier = GMNew api::BufferBarrier(&device_, *pocb_buf, gfx::EAccess::TRANSFER_WRITE, gfx::EAccess::UNIFORM_READ);
  pocb_to_read_barriers_.push_back(pocb_to_read_barrier);
  auto* pocb_to_write_barrier = GMNew api::BufferBarrier(&device_, *pocb_buf, gfx::EAccess::UNIFORM_READ, gfx::EAccess::TRANSFER_WRITE);
  pocb_to_write_barriers_.push_back(pocb_to_write_barrier);
  
  bool map_and_never_unmap = [&](){ pocb_staging_buf->Map(); return true; }();
  cmd.PushBarrier(*pocb_to_read_barrier);
}

template<class T, uint Count>
inline void gdm::SceneManager::CreatePerFrameUBO(api::CommandList& cmd)
{
  auto* pfcb_buf = GMNew api::Buffer(&device_, sizeof(T) * Count, gfx::TRANSFER_DST | gfx::UNIFORM, gfx::DEVICE_LOCAL);
  pfcb_uniform_.push_back(pfcb_buf);
  auto* pfcb_staging_buf = GMNew api::Buffer(&device_, sizeof(T) * Count, gfx::TRANSFER_SRC, gfx::HOST_VISIBLE); // and COHERENT too?
  pfcb_staging_.push_back(pfcb_staging_buf);
  auto* pfcb_to_read_barrier = GMNew api::BufferBarrier(&device_, *pfcb_buf, gfx::EAccess::TRANSFER_WRITE, gfx::EAccess::UNIFORM_READ);
  pfcb_to_read_barriers_.push_back(pfcb_to_read_barrier);
  auto* pfcb_to_write_barrier = GMNew api::BufferBarrier(&device_, *pfcb_buf, gfx::EAccess::UNIFORM_READ, gfx::EAccess::TRANSFER_WRITE);
  pfcb_to_write_barriers_.push_back(pfcb_to_write_barrier);

  cmd.PushBarrier(*pfcb_to_read_barrier);
}

template<class T>
inline void gdm::SceneManager::UpdatePerFrameUBO(api::CommandList& cmd, uint curr_frame, const T& pfcb)
{
  cmd.PushBarrier(*pfcb_to_write_barriers_[curr_frame]);
  pfcb_staging_[curr_frame]->Map();
  pfcb_staging_[curr_frame]->CopyDataToGpu(&pfcb, 1);
  pfcb_staging_[curr_frame]->Unmap();
  cmd.CopyBufferToBuffer(*pfcb_staging_[curr_frame], *pfcb_uniform_[curr_frame], sizeof(T));
  cmd.PushBarrier(*pfcb_to_read_barriers_[curr_frame]); 
}

template<class T>
inline void gdm::SceneManager::UpdatePerObjectUBO(api::CommandList& cmd, uint curr_frame)
{
  std::vector<T> pocbs = {};

  for (auto model_handle : GetRenderableModels())
  {
    AbstractModel* model = ModelFactory::Get(model_handle);
    for (auto mesh_handle : model->meshes_)
    {
      AbstractMesh* mesh = MeshFactory::Get(mesh_handle);
      AbstractMaterial* material = MaterialFactory::Get(mesh->material_);
      pocbs.push_back({});
      pocbs.back().u_model_ = model->tm_;
      pocbs.back().u_material_index_ = material->index_;
    }
  }

  cmd.PushBarrier(*pocb_to_write_barriers_[curr_frame]);
  pocb_staging_[curr_frame]->CopyDataToGpu(pocbs.data(), pocbs.size());
  uint pocb_size = static_cast<uint>(sizeof(T) * pocbs.size());
  cmd.CopyBufferToBuffer(*pocb_staging_[curr_frame], *pocb_uniform_[curr_frame], pocb_size);
  cmd.PushBarrier(*pocb_to_read_barriers_[curr_frame]);
}
