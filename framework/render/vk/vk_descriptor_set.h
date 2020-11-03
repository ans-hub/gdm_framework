// *************************************************************
// File:    vk_descriptor_set.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GM_VK_DESC_SET_H
#define GM_VK_DESC_SET_H

#include <vector>

#include "render/defines.h"

#include "render/vk/vk_sampler.h"
#include "render/vk/vk_buffer.h"
#include "render/vk/vk_image.h"

namespace gdm::vk {

struct DescriptorSet
{
  DescriptorSet(VkDevice device, VkDescriptorSetLayout layout, VkDescriptorPool pool);

  void UpdateContent(uint num, gfx::EResourceType type, const Sampler& sampler, const ImageView& view);
  void UpdateContent(uint num, gfx::EResourceType type, const ImageView& view);
  void UpdateContent(uint num, gfx::EResourceType type, const Sampler& sampler);
  void UpdateContent(uint num, gfx::EResourceType type, const Buffer& uniform);
  void Finalize();

  operator VkDescriptorSet() const;

private:
  auto Allocate() -> VkDescriptorSet;

  constexpr static uint v_max_bindings = 16;

private:
  VkDevice device_;
  bool explicitly_finalized_;
  VkDescriptorPool pool_;
  std::array<VkDescriptorImageInfo, v_max_bindings> image_infos_;
  std::array<VkDescriptorBufferInfo, v_max_bindings> buffer_infos_;
  std::vector<VkWriteDescriptorSet> write_descriptors_; 
  VkDescriptorSetLayout descriptor_set_layout_;
  VkDescriptorSet descriptor_set_;

}; // struct DescriptorSet

struct DescriptorSetLayout
{
  DescriptorSetLayout(VkDevice device);
  ~DescriptorSetLayout();

  uint AddBinding(uint num, gfx::EResourceType type, gfx::EShaderStage shader, gfx::BindingFlags flags = 0);
  void Finalize();
  auto GetBindings() const -> std::vector<VkDescriptorSetLayoutBinding> { return bindings_; }

  operator VkDescriptorSetLayout() const;

private:
  VkDevice device_;
  bool explicitly_finalized_;
  std::vector<VkDescriptorSetLayoutBinding> bindings_;
  std::vector<VkDescriptorBindingFlags> bindings_flags_;
  VkDescriptorSetLayout descriptor_set_layout_;
  bool is_updateable_after_bind_;

}; // struct DescriptorSetLayout

using DescriptorSets = std::vector<std::reference_wrapper<DescriptorSet>>;
using DescriptorSetLayouts = std::vector<VkDescriptorSetLayout>;

} // namespace gdm::vk

#endif // GM_VK_DESC_SET_H
