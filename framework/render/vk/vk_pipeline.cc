// *************************************************************
// File:    vk_pipeline.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "render/vk/vk_pipeline.h"
#include "render/vk/vk_host_allocator.h"

#include "render/shader_compiler.h"
#include "render/shader.h"

#include "system/assert_utils.h"

// --public

gdm::vk::Pipeline::Pipeline(VkDevice device)
  : device_{ device }
  , allocator_{ *HostAllocator::GetPtr() }
  , compiled_{ false }
  , pipeline_{VK_NULL_HANDLE}
  // and other
{ }

void gdm::vk::Pipeline::SetShaderStage(const Shader& shader, gfx::EShaderType type)
{
  ASSERTF(!IsCompiled(), "Trying to change read only pipeline");
  
  int shader_index = static_cast<int>(type);
  if (shader_index >= shaders_.size())
    shaders_.resize(shader_index + 1);
  shaders_[shader_index] = &shader;
}

void gdm::vk::Pipeline::SetInputLayout(const InputLayout& input_layout)
{
  ASSERTF(!IsCompiled(), "Trying to change read only pipeline");
  input_layout_ = input_layout;
}

void gdm::vk::Pipeline::SetRasterizerState(const RasterizerDesc& desc)
{
  ASSERTF(!IsCompiled(), "Trying to change read only pipeline");
  rasterizer_state_ = desc;
}

void gdm::vk::Pipeline::SetViewportState(const ViewportDesc& desc)
{
  ASSERTF(!IsCompiled(), "Trying to change read only pipeline");
  viewport_desc_ = desc;
}

void gdm::vk::Pipeline::SetRenderPass(VkRenderPass pass)
{
  ASSERTF(!IsCompiled(), "Trying to change read only pipeline");
  render_pass_ = pass;
}

void gdm::vk::Pipeline::SetDescriptorSetLayouts(const DescriptorSetLayouts& layouts)
{
  ASSERTF(!IsCompiled(), "Trying to change read only pipeline");
  descriptor_set_layouts_ = layouts;
}

void gdm::vk::Pipeline::SetDynamicState(gfx::EDynamicState state)
{
  ASSERTF(!IsCompiled(), "Trying to change read only pipeline");
  dynamic_state_.push_back(static_cast<VkDynamicState>(state)); // todo: check no dups
}

gdm::vk::Pipeline::operator VkPipeline() const 
{
  ASSERTF(compiled_, "Trying access non compiled pipeline");
  return pipeline_;
}

void gdm::vk::Pipeline::SetBlendAttachmentsCount(uint count)
{
  ASSERTF(!compiled_, "Trying access non compiled pipeline");
  blend_attachments_count_ = count;
}

gdm::vk::Pipeline::operator VkPipelineLayout() const 
{
  ASSERTF(compiled_, "Trying access non compiled pipeline");
  return pipeline_layout_;
}

void gdm::vk::Pipeline::Compile()
{
  ASSERTF(!IsCompiled(), "Trying to change read only pipeline");

  VkPipelineInputAssemblyStateCreateInfo input_assembly{};
  input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  input_assembly.topology = static_cast<VkPrimitiveTopology>(input_layout_.GetPrimitiveType());
  input_assembly.primitiveRestartEnable = VK_FALSE;

  std::vector<VkVertexInputBindingDescription> vertex_binding_descritpion;
  if (input_layout_.GetSize() != 0)
  {
    vertex_binding_descritpion.push_back({});
    vertex_binding_descritpion.back().binding = 0;
    vertex_binding_descritpion.back().stride = input_layout_.GetSize();
    vertex_binding_descritpion.back().inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  }

  const auto& elements = input_layout_.GetData();
  std::vector<VkVertexInputAttributeDescription> vertex_attribute_descritpion (elements.size()); 

  for (size_t i = 0; i < elements.size(); ++i)
  {
    vertex_attribute_descritpion[i].location = static_cast<uint>(i);
    vertex_attribute_descritpion[i].binding = 0;
    vertex_attribute_descritpion[i].format = static_cast<VkFormat>(elements[i].type_);
    vertex_attribute_descritpion[i].offset = elements[i].offset_;
  }

  VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
  vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertex_input_info.vertexBindingDescriptionCount = static_cast<uint>(vertex_binding_descritpion.size());
  vertex_input_info.pVertexBindingDescriptions = vertex_binding_descritpion.data();
  vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint>(vertex_attribute_descritpion.size());
  vertex_input_info.pVertexAttributeDescriptions = vertex_attribute_descritpion.data();

  std::vector<VkShaderModule> shader_modules{};
  std::vector<VkPipelineShaderStageCreateInfo> shader_stages{};

  for (const Shader* shader : shaders_)
  {
    if (!shader)
      continue;

    VkShaderModule module = CreateShaderModule(*shader);
    shader_modules.push_back(module);

    VkPipelineShaderStageCreateInfo shader_stage_info{};
    shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stage_info.stage = GetShaderStageBit(shader->GetType());
    shader_stage_info.module = module;
    shader_stage_info.pName = "main";

    shader_stages.push_back(shader_stage_info);
  }

  VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {};
  dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_state_create_info.dynamicStateCount = static_cast<uint>(dynamic_state_.size());
  dynamic_state_create_info.pDynamicStates = dynamic_state_.data();

  VkViewport viewport{};
  viewport.x = viewport_desc_.x_;
  viewport.y = viewport_desc_.y_;
  viewport.width = viewport_desc_.width_;
  viewport.height = viewport_desc_.height_;
  viewport.minDepth = viewport_desc_.min_depth_;
  viewport.maxDepth = viewport_desc_.max_depth_;

  VkRect2D scissor = { 0, 0, static_cast<uint32_t>(viewport_desc_.width_), static_cast<uint32_t>(viewport_desc_.height_) };
  
  VkPipelineViewportStateCreateInfo viewport_state{};
  viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_state.viewportCount = 1;
  viewport_state.pViewports = &viewport;
  viewport_state.scissorCount = 1;
  viewport_state.pScissors = &scissor;

  VkPipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = static_cast<VkPolygonMode>(rasterizer_state_.fill_);
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = static_cast<VkCullModeFlagBits>(rasterizer_state_.cull_);
  rasterizer.frontFace = static_cast<VkFrontFace>(rasterizer_state_.front_face_);
  rasterizer.depthBiasEnable = VK_FALSE;

  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachment(blend_attachments_count_);
  for(auto& attachment : color_blend_attachment)
  {
    attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    attachment.blendEnable = VK_FALSE;
  }

  VkPipelineColorBlendStateCreateInfo color_blending{};
  color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  color_blending.logicOpEnable = VK_FALSE;
  color_blending.logicOp = VK_LOGIC_OP_COPY;
  color_blending.attachmentCount = blend_attachments_count_;
  color_blending.pAttachments = color_blend_attachment.data();
  color_blending.blendConstants[0] = 0.0f;
  color_blending.blendConstants[1] = 0.0f;
  color_blending.blendConstants[2] = 0.0f;
  color_blending.blendConstants[3] = 0.0f;

  VkStencilOpState noOP_stencil_state = {};
  noOP_stencil_state.failOp = VK_STENCIL_OP_KEEP;
  noOP_stencil_state.passOp = VK_STENCIL_OP_KEEP;
  noOP_stencil_state.depthFailOp = VK_STENCIL_OP_KEEP;
  noOP_stencil_state.compareOp = VK_COMPARE_OP_ALWAYS;
  noOP_stencil_state.compareMask = 0;
  noOP_stencil_state.writeMask = 0;
  noOP_stencil_state.reference = 0;

  VkPipelineDepthStencilStateCreateInfo depth_state = {};
  depth_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depth_state.depthTestEnable = VK_TRUE;
  depth_state.depthWriteEnable = VK_TRUE;
  depth_state.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
  depth_state.depthBoundsTestEnable = VK_FALSE;
  depth_state.stencilTestEnable = VK_FALSE;
  depth_state.front = noOP_stencil_state;
  depth_state.back = noOP_stencil_state;
  depth_state.minDepthBounds = 0;
  depth_state.maxDepthBounds = 0;

  VkPipelineLayoutCreateInfo pipeline_layout_info{};
  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_info.setLayoutCount = static_cast<uint>(descriptor_set_layouts_.size());
  pipeline_layout_info.pSetLayouts = descriptor_set_layouts_.data();
  pipeline_layout_info.pushConstantRangeCount = 0;

  VkResult res = vkCreatePipelineLayout(device_, &pipeline_layout_info, nullptr, &pipeline_layout_);
  ASSERTF(res == VK_SUCCESS, "vkCreatePipelineLayout error %d", res);

  VkGraphicsPipelineCreateInfo pipeline_info = {};
  pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_info.stageCount = static_cast<uint32_t>(shader_stages.size());
  pipeline_info.pStages = shader_stages.data();
  pipeline_info.pVertexInputState = &vertex_input_info;
  pipeline_info.pInputAssemblyState = &input_assembly;
  pipeline_info.pViewportState = &viewport_state;
  pipeline_info.pDynamicState = &dynamic_state_create_info;
  pipeline_info.pDepthStencilState = &depth_state;
  pipeline_info.pRasterizationState = &rasterizer;
  pipeline_info.pMultisampleState = &multisampling;
  pipeline_info.pColorBlendState = &color_blending;
  pipeline_info.layout = pipeline_layout_;
  pipeline_info.renderPass = render_pass_;
  pipeline_info.subpass = 0;
  pipeline_info.basePipelineHandle = NULL;
  pipeline_info.basePipelineIndex = 0;

  res = vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &pipeline_);
  ASSERTF(res == VK_SUCCESS, "vkCreateGraphicsPipelines error %d", res);

  compiled_ = true;
  
  for(VkShaderModule module : shader_modules)
    DestroyShaderModule(module);
  
  compiled_ = true;
}

// -private

auto gdm::vk::Pipeline::CreateShaderModule(const Shader& shader) -> VkShaderModule
{
  VkShaderModuleCreateInfo shader_create_info = {};
  shader_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  shader_create_info.codeSize = shader.GetBlob().size;
  shader_create_info.pCode = (const uint32_t*)shader.GetBlob().ptr;

  VkShaderModule shader_module;
  VkResult res = vkCreateShaderModule(device_, &shader_create_info, &allocator_, &shader_module);
  ASSERTF(res == VK_SUCCESS, "vkCreateShaderModule error %d", res);

  return shader_module;
}

void gdm::vk::Pipeline::DestroyShaderModule(VkShaderModule module)
{
  vkDestroyShaderModule(device_, module, &allocator_);
}

auto gdm::vk::Pipeline::GetShaderStageBit(gfx::EShaderType type) -> VkShaderStageFlagBits
{
  switch(type)
  {
    case gfx::EShaderType::VX : return VK_SHADER_STAGE_VERTEX_BIT;
    case gfx::EShaderType::PX : return VK_SHADER_STAGE_FRAGMENT_BIT;
    default :
    {
      ASSERTF(false, "Unknown shader type while get shader stage big");
      return {};
    }
  }  
}
