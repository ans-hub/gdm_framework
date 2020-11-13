// *************************************************************
// File:    vk_pipeline.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GM_VK_PIPELINE_H
#define GM_VK_PIPELINE_H

#include "render/defines.h"
#include "render/viewport_desc.h"
#include "render/rasterizer_desc.h"
#include "render/input_layout.h"
#include "render/vk/vk_descriptor_set.h"

namespace gdm {
  struct Shader;
}

namespace gdm::vk {

struct Pipeline
{
  Pipeline(VkDevice device);

  void Compile();
  bool IsCompiled() const { return compiled_; }

  void SetShaderStage(const Shader& shader, gfx::EShaderType type);
  void SetInputLayout(const InputLayout& input_layout);
  void SetRasterizerState(const RasterizerDesc& desc);
  void SetViewportState(const ViewportDesc& desc);
  void SetRenderPass(VkRenderPass pass);
  void SetDescriptorSetLayouts(const DescriptorSetLayouts& layouts);
  void SetDynamicState(gfx::EDynamicState dynamic_state);
  void SetBlendAttachmentsCount(uint count);

  operator VkPipeline() const;
  operator VkPipelineLayout() const;

private:
  auto CreateShaderModule(const Shader& shader) -> VkShaderModule;
  void DestroyShaderModule(VkShaderModule shader);
  auto GetShaderStageBit(gfx::EShaderType type) -> VkShaderStageFlagBits;

private:
  VkDevice device_;
  const VkAllocationCallbacks& allocator_;
  bool compiled_;
  VkPipeline pipeline_;

private:
  std::vector<const Shader*> shaders_ = {};
  ViewportDesc viewport_desc_ = {};
  RasterizerDesc rasterizer_state_ = {};
  VkRenderPass render_pass_ = VK_NULL_HANDLE;
  std::vector<VkDescriptorSetLayout> descriptor_set_layouts_ = {};
  InputLayout input_layout_ = {};
  std::vector<VkDynamicState> dynamic_state_ = {};
  VkPipelineLayout pipeline_layout_ = VK_NULL_HANDLE;
  uint blend_attachments_count_ = 1;

}; // struct Pipeline

} // namespace gdm::vk

#endif // GM_VK_PIPELINE_H
