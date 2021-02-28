// *************************************************************
// File:    gpu_streamer.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GFX_GPU_STREAMER
#define GFX_GPU_STREAMER

#include "data/model_factory.h"
#include "data/texture_factory.h"

#include "render/defines.h"
#include "render/api.h"
#include "render/renderer.h"

namespace gdm {

struct GpuStreamer
{
  GpuStreamer(api::Renderer& gfx);

  uint CreateStagingBuffer(uint bytes);
  auto GetStagingBuffer(uint index) -> api::Buffer& { return *staging_buffers_[index]; };
  void CopyModelsToGpu(const std::vector<ModelHandle>& models);
  void CopyGeometryToGpu(const std::vector<ModelHandle>& models, uint vstg_index, uint istg_index, api::CommandList& list);
  void CopyTexturesToGpu(const std::vector<ModelHandle>& models, uint tstg_index, api::CommandList& list);

private:
  uint CopyTextureToStagingBuffer(AbstractTexture* texture, api::Buffer& stg, uint curr_offset);
  void CopyTextureFromStagingBuffer(api::CommandList& cmd, AbstractTexture* texture, api::Buffer& stg, uint curr_offset);
  void CreateDummyView(api::CommandList& cmd);

private:
  api::Renderer& gfx_;
  api::Device& device_;
  std::vector<api::Buffer*> staging_buffers_;

};  // struct GpuStreamer

} // namespace gdm

#endif // GFX_GPU_STREAMER