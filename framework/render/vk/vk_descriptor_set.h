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

struct DescriptorSetLayout;

struct DescriptorSet
{
  DescriptorSet(VkDevice device, const DescriptorSetLayout& layout, VkDescriptorPool pool);

  template<gfx::EResourceType Type, class...Args>
  void UpdateContent(uint num, const Args&...args);
  void Finalize();

public:
  operator VkDescriptorSet() const;
  constexpr static uint v_max_bindings = 16;
  constexpr static uint v_max_variable_descriptor = 16;

private:
  auto Allocate(const DescriptorSetLayout& layout) -> VkDescriptorSet;

private:
  VkDevice device_;
  bool explicitly_finalized_;
  VkDescriptorPool pool_;
  std::array<std::vector<VkDescriptorImageInfo>, v_max_bindings> image_infos_;
  std::array<std::vector<VkDescriptorBufferInfo>, v_max_bindings> buffer_infos_;
  std::vector<VkWriteDescriptorSet> write_descriptors_; 
  VkDescriptorSetLayout descriptor_set_layout_;
  VkDescriptorSet descriptor_set_;

}; // struct DescriptorSet

struct DescriptorSetLayout
{
  DescriptorSetLayout(VkDevice device);
  ~DescriptorSetLayout();

  uint AddBinding(uint num, uint count, gfx::EResourceType type, gfx::EShaderStage shader, gfx::BindingFlags flags = 0);
  void Finalize();
  auto GetBindings() const -> std::vector<VkDescriptorSetLayoutBinding> { return bindings_; }
  uint GetVariableDescriptorsCount() const;
  
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

#include "vk_descriptor_set.inl"

#endif // GM_VK_DESC_SET_H
