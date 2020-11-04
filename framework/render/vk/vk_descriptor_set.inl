// *************************************************************
// File:    vk_descriptor_set.inl
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "vk_descriptor_set.h"

#include "render/vk/vk_image_view.h"

// --public DescriptorSet

template<>
inline void gdm::vk::DescriptorSet::UpdateContent<gdm::gfx::EResourceType::SAMPLED_IMAGE, gdm::vk::ImageView>(uint num, const ImageView& view)
{
  explicitly_finalized_ = false;

  image_infos_[num].push_back({});
  image_infos_[num].back().sampler = 0;
  image_infos_[num].back().imageView = view;
  image_infos_[num].back().imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  VkWriteDescriptorSet image_desc_set = {};
  image_desc_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  image_desc_set.dstSet = descriptor_set_;
  image_desc_set.dstBinding = num;
  image_desc_set.dstArrayElement = 0;
  image_desc_set.descriptorCount = 1;
  image_desc_set.descriptorType = static_cast<VkDescriptorType>(gfx::EResourceType::SAMPLED_IMAGE);
  image_desc_set.pImageInfo = image_infos_[num].data();
  image_desc_set.pBufferInfo = nullptr;
  image_desc_set.pTexelBufferView = nullptr;

  write_descriptors_.push_back({});
  write_descriptors_.back() = image_desc_set;
}

template<>
inline void gdm::vk::DescriptorSet::UpdateContent<gdm::gfx::EResourceType::SAMPLED_IMAGE, gdm::vk::ImageViews>(uint num, const ImageViews& views)
{
  explicitly_finalized_ = false;

  for (auto view : views)
  {
    image_infos_[num].push_back({});
    image_infos_[num].back().imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_infos_[num].back().sampler = 0;
    image_infos_[num].back().imageView = *view;
  }

  VkWriteDescriptorSet image_desc_set = {};
  image_desc_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  image_desc_set.dstSet = descriptor_set_;
  image_desc_set.dstBinding = num;
  image_desc_set.dstArrayElement = 0;
  image_desc_set.descriptorCount = static_cast<uint>(image_infos_[num].size());
  image_desc_set.descriptorType = static_cast<VkDescriptorType>(gfx::EResourceType::SAMPLED_IMAGE);
  image_desc_set.pImageInfo = image_infos_[num].data();
  image_desc_set.pBufferInfo = nullptr;
  image_desc_set.pTexelBufferView = nullptr;

  write_descriptors_.push_back({});
  write_descriptors_.back() = image_desc_set;
}

template<>
inline void gdm::vk::DescriptorSet::UpdateContent<gdm::gfx::EResourceType::SAMPLER, gdm::vk::Sampler>(uint num, const Sampler& sampler)
{
  explicitly_finalized_ = false;

  image_infos_[num].push_back({});
  image_infos_[num].back().sampler = sampler;

  VkWriteDescriptorSet image_desc_set = {};
  image_desc_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  image_desc_set.dstSet = descriptor_set_;
  image_desc_set.dstBinding = num;
  image_desc_set.dstArrayElement = 0;
  image_desc_set.descriptorCount = 1;
  image_desc_set.descriptorType = static_cast<VkDescriptorType>(gfx::EResourceType::SAMPLER);
  image_desc_set.pImageInfo = image_infos_[num].data();
  image_desc_set.pBufferInfo = nullptr;
  image_desc_set.pTexelBufferView = nullptr;

  write_descriptors_.push_back({});
  write_descriptors_.back() = image_desc_set;
}

template<>
inline void gdm::vk::DescriptorSet::UpdateContent<gdm::gfx::EResourceType::UNIFORM_DYNAMIC, gdm::vk::Buffer>(uint num, const Buffer& buffer)
{
  explicitly_finalized_ = false;

  buffer_infos_[num].push_back({});
  buffer_infos_[num].back().buffer = buffer;
  buffer_infos_[num].back().offset = 0;
  buffer_infos_[num].back().range = VK_WHOLE_SIZE;

  VkWriteDescriptorSet buffer_desc_set = {};
  buffer_desc_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  buffer_desc_set.dstSet = descriptor_set_;
  buffer_desc_set.dstBinding = num;
  buffer_desc_set.dstArrayElement = 0;
  buffer_desc_set.descriptorCount = 1;
  buffer_desc_set.descriptorType = static_cast<VkDescriptorType>(gfx::EResourceType::UNIFORM_DYNAMIC);
  buffer_desc_set.pImageInfo = nullptr;
  buffer_desc_set.pBufferInfo = buffer_infos_[num].data();
  buffer_desc_set.pTexelBufferView = nullptr;

  write_descriptors_.push_back({});
  write_descriptors_.back() = buffer_desc_set;
}

template<>
inline void gdm::vk::DescriptorSet::UpdateContent<gdm::gfx::EResourceType::UNIFORM_BUFFER, gdm::vk::Buffer>(uint num, const Buffer& buffer)
{
  explicitly_finalized_ = false;

  buffer_infos_[num].push_back({});
  buffer_infos_[num].back().buffer = buffer;
  buffer_infos_[num].back().offset = 0;
  buffer_infos_[num].back().range = static_cast<VkDeviceSize>(buffer.GetSize());

  VkWriteDescriptorSet buffer_desc_set = {};
  buffer_desc_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  buffer_desc_set.dstSet = descriptor_set_;
  buffer_desc_set.dstBinding = num;
  buffer_desc_set.dstArrayElement = 0;
  buffer_desc_set.descriptorCount = 1;
  buffer_desc_set.descriptorType = static_cast<VkDescriptorType>(gfx::EResourceType::UNIFORM_BUFFER);
  buffer_desc_set.pImageInfo = nullptr;
  buffer_desc_set.pBufferInfo = buffer_infos_[num].data();
  buffer_desc_set.pTexelBufferView = nullptr;

  write_descriptors_.push_back({});
  write_descriptors_.back() = buffer_desc_set;
}
