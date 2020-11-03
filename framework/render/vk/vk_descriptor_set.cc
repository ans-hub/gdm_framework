// *************************************************************
// File:    vk_descriptor_set.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "vk_descriptor_set.h"

#include "system/array_utils.h"
#include "system/assert_utils.h"
#include "system/bits_utils.h"
#include "render/vk/vk_buffer.h"
#include "render/vk/vk_host_allocator.h"

// --public DescriptorSet

gdm::vk::DescriptorSet::DescriptorSet(VkDevice device, const DescriptorSetLayout& layout, VkDescriptorPool pool)
  : device_{device}
  , explicitly_finalized_{false}
  , pool_{pool}
  , image_infos_()
  , buffer_infos_()
  , write_descriptors_{}
  , descriptor_set_layout_{layout}
  , descriptor_set_{ Allocate(layout) }
{ }

void gdm::vk::DescriptorSet::Finalize()
{
  ASSERTF(!explicitly_finalized_, "Trying to finalize already finalized set");
 
  vkUpdateDescriptorSets(device_, static_cast<uint32_t>(write_descriptors_.size()), write_descriptors_.data(), 0, nullptr);
  
  for(auto& image_info : image_infos_)
    image_info.clear();
  for(auto& buffer_info : buffer_infos_)
    buffer_info.clear();
      
  write_descriptors_.clear();
  explicitly_finalized_ = true;
}

gdm::vk::DescriptorSet::operator VkDescriptorSet() const
{
  ASSERTF(explicitly_finalized_, "Trying to access underlying data while descriptio set not finalized");
  return descriptor_set_;
}

// --private DescrpitorSet

auto gdm::vk::DescriptorSet::Allocate(const DescriptorSetLayout& layout) -> VkDescriptorSet
{
  VkDescriptorSetAllocateInfo allocate_info = {};
  allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocate_info.descriptorPool = pool_;
  allocate_info.descriptorSetCount = 1;
  allocate_info.pSetLayouts = &descriptor_set_layout_;

  VkDescriptorSetVariableDescriptorCountAllocateInfo variable_count_alloc_info = {};
  variable_count_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
  variable_count_alloc_info.descriptorSetCount = 1;
  
  uint32_t variable_desc_count = static_cast<uint32_t>(layout.GetVariableDescriptorsCount());
  variable_count_alloc_info.pDescriptorCounts = &variable_desc_count;

  allocate_info.pNext = &variable_count_alloc_info;

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

uint gdm::vk::DescriptorSetLayout::AddBinding(uint num, uint count, gfx::EResourceType type, gfx::EShaderStage stage, gfx::BindingFlags flags /*=0*/)
{
  bindings_.push_back({});
  bindings_flags_.push_back({});  

  ASSERTF(!explicitly_finalized_, "Adding to finalized layout");
  
  auto& binding = bindings_.back();
  binding.binding = num;
  binding.descriptorType = static_cast<VkDescriptorType>(type);
  binding.descriptorCount = count;
  binding.stageFlags = static_cast<VkShaderStageFlags>(stage);
  binding.pImmutableSamplers = NULL;

  // todo: remove
  // if (bits::HasFlag(flags, gfx::VARIABLE_DESCRIPTOR))
  //   binding.descriptorCount = DescriptorSet::v_max_variable_descriptor;

  auto& binding_flags = bindings_flags_.back();
  binding_flags = static_cast<VkDescriptorBindingFlagBits>(flags);

  return static_cast<uint>(bindings_.size() - 1);
}

void gdm::vk::DescriptorSetLayout::Finalize()
{
  ASSERTF(!explicitly_finalized_, "Trying to finalize already finalized layout");

  VkDescriptorSetLayoutBindingFlagsCreateInfo set_layout_binding_create_info = {};
  set_layout_binding_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO; // aka _EXT
  set_layout_binding_create_info.bindingCount = static_cast<uint>(bindings_.size());
  set_layout_binding_create_info.pBindingFlags = bindings_flags_.data();

  VkDescriptorSetLayoutCreateInfo set_layout_create_info = {};
  set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  set_layout_create_info.pNext = &set_layout_binding_create_info;
  set_layout_create_info.bindingCount = static_cast<uint>(bindings_.size());
  set_layout_create_info.pBindings = bindings_.data();

  bool non_updateable_after_bind = true;
  for (auto& flags : bindings_flags_)
    non_updateable_after_bind &= (!bits::HasFlag(flags, gfx::EBindingFlags::UPDATE_AFTER_BIND));
  
  if (!non_updateable_after_bind)
    set_layout_create_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

  VkResult res = vkCreateDescriptorSetLayout(device_, &set_layout_create_info, HostAllocator::GetPtr(), &descriptor_set_layout_);
  ASSERTF(res == VK_SUCCESS, "vkCreateDescriptorSetLayout failed %d", res);

  explicitly_finalized_ = true;
}

uint gdm::vk::DescriptorSetLayout::GetVariableDescriptorsCount() const
{
  uint total = 0;
  uint i = 0;
  for (auto& flags : bindings_flags_)
  {
    if (bits::HasFlag(flags, gfx::EBindingFlags::VARIABLE_DESCRIPTOR))
      total += bindings_[i].descriptorCount;
    ++i;
  }
  return total;
}

gdm::vk::DescriptorSetLayout::operator VkDescriptorSetLayout() const
{
  ASSERTF(explicitly_finalized_, "Trying to access underlying data while descriptio set not finalized");
  return descriptor_set_layout_;
}
