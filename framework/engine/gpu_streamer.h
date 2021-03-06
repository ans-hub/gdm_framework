// *************************************************************
// File:    gpu_streamer.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GFX_GPU_STREAMER
#define GFX_GPU_STREAMER

#include "factory/model_factory.h"
#include "factory/texture_factory.h"

#include "render/defines.h"
#include "render/api.h"
#include "render/renderer.h"

namespace gdm {

struct GpuStreamer
{
  GpuStreamer(api::Renderer& ctx);
  ~GpuStreamer();

  uint CreateStagingBuffer(uint bytes);
  auto FindStagingBuffer(uint min_size) -> int;
  auto GetStagingBuffer(uint index) -> api::Buffer& { return *staging_buffers_[index]; };
  void CopyModelsToGpu(const std::vector<ModelHandle>& models);
  void CopyGeometryToGpu(const std::vector<ModelHandle>& models, uint vstg_index, uint istg_index, api::CommandList& list);
  void CopyMaterialsToGpu(const std::vector<MaterialHandle>& materials, uint tstg_index, api::CommandList& list);
  void CopyTexturesToGpu(const std::vector<TextureHandle>& materials, uint tstg_index);
  void CopyTexturesToGpu(const std::vector<TextureHandle>& materials, uint tstg_index, api::CommandList& list);

private:
  uint CopyTextureToStagingBuffer(AbstractTexture* texture, api::Buffer& stg, uint curr_offset);
  void CopyTextureFromStagingBuffer(api::CommandList& cmd, AbstractTexture* texture, api::Buffer& stg, uint curr_offset);
  void CreateDummyView(api::CommandList& cmd);

private:
  constexpr static int v_max_buffers_ = 32;

private:
  api::Renderer& ctx_;
  api::Device& device_;
  std::vector<api::Buffer*> staging_buffers_;
  std::vector<api::Image*> images_;
  std::vector<api::ImageView*> image_views_;

};  // struct GpuStreamer

namespace helpers {

  auto GetMaterialsToLoad(const std::vector<ModelHandle>& handles) -> std::vector<MaterialHandle>;
} // namespace helpers

} // namespace gdm

#endif // GFX_GPU_STREAMER