// *************************************************************
// File:    vk_descriptor_set.cc shader_resources
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

// 1. describe
// 2. allocate

// Resources - just blocks of memory
// Descriptors - in fact handles to resources, also resource (buffers and images) variables which used by shaders
//  (thought as a pointer to resource)
// Descriptor set - storage for set of descriptors, defined by descriptor set layout
// Descriptor set layout - associations of each descriptor binding with with
//  ** This object describes which bindings are in the DescriptorSet for every shader stage
// Descriptor bindings - used to define descriptor set layout
//  number of binding - corresponds to appropriate number in shader stage
//  array - means array on particular binding number (i.e. samplers[2] and sampler0/sampler1 are different) 

// https://developer.nvidia.com/vulkan-shader-resource-binding
// https://software.intel.com/content/www/us/en/develop/articles/api-without-secrets-introduction-to-vulkan-part-6.html

// From Intel:
//  Descriptor sets are opaque objects in which we store handles of resources.
//  Layouts define the structure of descriptor sets—what types of descriptors they contain,
//  how many descriptors of each type there are, and what their order is.

// Descriptor set: samplerA, samplerB, ub0, ub1
// Layout 1: samplerB (num 0), ub0 (num 1)
// Layout 2: samplerA (num 0), samlerB (num 1)

#include "vk_descriptor_set.h"

#include "system/array_utils.h"
#include "system/assert_utils.h"
#include "system/bits_utils.h"
#include "render/vk/vk_buffer.h"
#include "render/vk/vk_host_allocator.h"

// --public DescriptorSet

gdm::vk::DescriptorSet::DescriptorSet(VkDevice device, VkDescriptorSetLayout layout, VkDescriptorPool pool)
  : device_{device}
  , explicitly_finalized_{false}
  , pool_{pool}
  , image_infos_(16)
  , buffer_infos_(16)
  , write_descriptors_{}
  , descriptor_set_layout_{layout}
  , descriptor_set_{Allocate()}
{ }

void gdm::vk::DescriptorSet::UpdateContent(uint num, gfx::EResourceType type, const Sampler& sampler, const ImageView& view)
{
  arr_utils::EnsureIndex(image_infos_, num);
  image_infos_[num].sampler = sampler;
  image_infos_[num].imageView = view;
  image_infos_[num].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  VkWriteDescriptorSet image_desc_set = {};
  image_desc_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  image_desc_set.dstSet = descriptor_set_;
  image_desc_set.dstBinding = num;
  image_desc_set.dstArrayElement = 0;
  image_desc_set.descriptorCount = 1;
  image_desc_set.descriptorType = static_cast<VkDescriptorType>(type);
  image_desc_set.pImageInfo = &image_infos_[num];
  image_desc_set.pBufferInfo = nullptr;
  image_desc_set.pTexelBufferView = nullptr;

  write_descriptors_.push_back({});
  write_descriptors_.back() = image_desc_set;
}

void gdm::vk::DescriptorSet::UpdateContent(uint num, gfx::EResourceType type, const ImageView& view)
{
  arr_utils::EnsureIndex(image_infos_, num);
  image_infos_[num].sampler = 0;
  image_infos_[num].imageView = view;
  image_infos_[num].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  VkWriteDescriptorSet image_desc_set = {};
  image_desc_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  image_desc_set.dstSet = descriptor_set_;
  image_desc_set.dstBinding = num;
  image_desc_set.dstArrayElement = 0;
  image_desc_set.descriptorCount = 1;
  image_desc_set.descriptorType = static_cast<VkDescriptorType>(type);
  image_desc_set.pImageInfo = &image_infos_[num];
  image_desc_set.pBufferInfo = nullptr;
  image_desc_set.pTexelBufferView = nullptr;

  write_descriptors_.push_back({});
  write_descriptors_.back() = image_desc_set;
}

void gdm::vk::DescriptorSet::UpdateContent(uint num, gfx::EResourceType type, const Sampler& sampler)
{
  arr_utils::EnsureIndex(image_infos_, num);
  image_infos_[num].sampler = sampler;

  VkWriteDescriptorSet image_desc_set = {};
  image_desc_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  image_desc_set.dstSet = descriptor_set_;
  image_desc_set.dstBinding = num;
  image_desc_set.dstArrayElement = 0;
  image_desc_set.descriptorCount = 1;
  image_desc_set.descriptorType = static_cast<VkDescriptorType>(type);
  image_desc_set.pImageInfo = &image_infos_[num];
  image_desc_set.pBufferInfo = nullptr;
  image_desc_set.pTexelBufferView = nullptr;

  write_descriptors_.push_back({});
  write_descriptors_.back() = image_desc_set;
}

void gdm::vk::DescriptorSet::UpdateContent(uint num, gfx::EResourceType type, const Buffer& buffer)
{
  arr_utils::EnsureIndex(buffer_infos_, num);
  buffer_infos_[num].buffer = buffer;
  buffer_infos_[num].offset = 0;

  if (type == gfx::EResourceType::UNIFORM_DYNAMIC)
    buffer_infos_[num].range = VK_WHOLE_SIZE;
  else
    buffer_infos_[num].range = static_cast<VkDeviceSize>(buffer.GetSize());

  VkWriteDescriptorSet buffer_desc_set = {};
  buffer_desc_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  buffer_desc_set.dstSet = descriptor_set_;
  buffer_desc_set.dstBinding = num;
  buffer_desc_set.dstArrayElement = 0;
  buffer_desc_set.descriptorCount = 1;
  buffer_desc_set.descriptorType = static_cast<VkDescriptorType>(type);
  buffer_desc_set.pImageInfo = nullptr;
  buffer_desc_set.pBufferInfo = &buffer_infos_[num];
  buffer_desc_set.pTexelBufferView = nullptr;

  write_descriptors_.push_back({});
  write_descriptors_.back() = buffer_desc_set;
}

void gdm::vk::DescriptorSet::Finalize()
{
  ASSERTF(!explicitly_finalized_, "Trying to finalize already finalized set");
 
  vkUpdateDescriptorSets(device_, static_cast<uint32_t>(write_descriptors_.size()), write_descriptors_.data(), 0, nullptr);

  image_infos_.clear();
  buffer_infos_.clear();
  write_descriptors_.clear();  
  explicitly_finalized_ = true;
}

gdm::vk::DescriptorSet::operator VkDescriptorSet() const
{
  ASSERTF(explicitly_finalized_, "Trying to access underlying data while descriptio set not finalized");
  return descriptor_set_;
}

// --private DescrpitorSet

auto gdm::vk::DescriptorSet::Allocate() -> VkDescriptorSet
{
  VkDescriptorSetAllocateInfo allocate_info = {};
  allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocate_info.descriptorPool = pool_;
  allocate_info.descriptorSetCount = 1;
  allocate_info.pSetLayouts = &descriptor_set_layout_;

  VkDescriptorSet descriptor_set = {};
  VkResult res = vkAllocateDescriptorSets(device_, &allocate_info, &descriptor_set);
  ASSERTF(res == VK_SUCCESS, "vkAllocateDescriptorSets failed %d", res);
 
  return descriptor_set;
}

// --public DescriptorSetLayout

gdm::vk::DescriptorSetLayout::DescriptorSetLayout(VkDevice device)
  : device_{device}
  , explicitly_finalized_{false}
  , bindings_{}
  , bindings_flags_{}
  , descriptor_set_layout_{VK_NULL_HANDLE}
{ }

gdm::vk::DescriptorSetLayout::~DescriptorSetLayout()
{
  vkDestroyDescriptorSetLayout(device_, descriptor_set_layout_, HostAllocator::GetPtr());
}

uint gdm::vk::DescriptorSetLayout::AddBinding(uint num, gfx::EResourceType type, gfx::EShaderStage stage, gfx::BindingFlags flags /*=0*/)
{
  bindings_.push_back({});
  bindings_flags_.push_back({});  

  ASSERTF(!explicitly_finalized_, "Adding to finalized layout");
  
  auto& binding = bindings_.back();
  binding.binding = num;
  binding.descriptorType = static_cast<VkDescriptorType>(type);
  binding.descriptorCount = 1;
  binding.stageFlags = static_cast<VkShaderStageFlags>(stage);
  binding.pImmutableSamplers = NULL;

  auto& binding_flags = bindings_flags_.back();
  binding_flags = static_cast<VkDescriptorBindingFlagBits>(flags);

  return static_cast<uint>(bindings_.size() - 1);
}

void gdm::vk::DescriptorSetLayout::Finalize()
{
  ASSERTF(!explicitly_finalized_, "Trying to finalize already finalized layout");

  VkDescriptorSetLayoutBindingFlagsCreateInfo set_layout_binding_create_info = {};
  set_layout_binding_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
  set_layout_binding_create_info.bindingCount = static_cast<uint>(bindings_.size());
  set_layout_binding_create_info.pBindingFlags = bindings_flags_.data();

  VkDescriptorSetLayoutCreateInfo set_layout_create_info = {};
  set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  set_layout_create_info.pNext = &set_layout_binding_create_info;
  set_layout_create_info.bindingCount = static_cast<uint>(bindings_.size());
  set_layout_create_info.pBindings = bindings_.data();

  bool non_updateable_after_bind = true;
  for (auto& flags : bindings_flags_)
    non_updateable_after_bind &= (!bits::HasFlag(flags,gfx::EBindingFlags::UPDATE_AFTER_BIND));
  
  if (!non_updateable_after_bind)
    set_layout_create_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

  VkResult res = vkCreateDescriptorSetLayout(device_, &set_layout_create_info, HostAllocator::GetPtr(), &descriptor_set_layout_);
  ASSERTF(res == VK_SUCCESS, "vkCreateDescriptorSetLayout failed %d", res);

  explicitly_finalized_ = true;
}

gdm::vk::DescriptorSetLayout::operator VkDescriptorSetLayout() const
{
  ASSERTF(explicitly_finalized_, "Trying to access underlying data while descriptio set not finalized");
  return descriptor_set_layout_;
}
