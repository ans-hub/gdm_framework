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

private:
  VkDevice device_;
  bool explicitly_finalized_;
  VkDescriptorPool pool_;
  std::vector<VkDescriptorImageInfo> image_infos_;
  std::vector<VkDescriptorBufferInfo> buffer_infos_;
  std::vector<VkWriteDescriptorSet> write_descriptors_; 
  VkDescriptorSetLayout descriptor_set_layout_;
  VkDescriptorSet descriptor_set_;

}; // struct DescriptorSet

struct DescriptorSetLayout
{
  DescriptorSetLayout(VkDevice device);
  ~DescriptorSetLayout();

  void AddBinding(uint num, gfx::EResourceType type, gfx::EShaderStage shader);
  void Finalize();
  auto GetBindings() const -> std::vector<VkDescriptorSetLayoutBinding> { return bindings_; }

  operator VkDescriptorSetLayout() const;

private:
  VkDevice device_;
  bool explicitly_finalized_;
  std::vector<VkDescriptorSetLayoutBinding> bindings_;
  VkDescriptorSetLayout descriptor_set_layout_;

}; // struct DescriptorSetLayout

using DescriptorSetLayouts = std::vector<VkDescriptorSetLayout>;

} // namespace gdm::vk

#endif // GM_VK_DESC_SET_H
