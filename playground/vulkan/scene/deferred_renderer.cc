// *************************************************************
// File:    deferred_renderer.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "deferred_renderer.h"

#include "system/diff_utils.h"
#include "system/literals.h"

#include "render/shader.h"
#include "render/desc/sampler_desc.h"
#include "render/desc/input_layout_desc.h"
#include "render/desc/rasterizer_desc.h"

#include "helpers.h"

// --public

gdm::DeferredRenderer::DeferredRenderer(HWND window_handle, gfx::DeviceProps flags /*= 0*/)
  : gfx_{ api::Renderer(window_handle, flags) }
  , device_{ gfx_.GetDevice() }
  , submit_fence_(device_)
  , staging_buffers_{}
  , gbuffer_pass_(gfx_)
  , deferred_pass_(gfx::v_num_images, gfx_)
  , debug_pass_(gfx::v_num_images, gfx_)
  , debug_draw_{}
{ }

void gdm::DeferredRenderer::Setup(Scene& scene)
{
  api::CommandList setup_list = gfx_.CreateCommandList(GDM_HASH("SceneSetup"), gfx::ECommandListFlags::ONCE);

  uint vstg = CreateStagingBuffer(MB(64));
  uint istg = CreateStagingBuffer(MB(32));
  uint tstg = CreateStagingBuffer(MB(96));
  
  auto unique_models = helpers::GetUniqueModels(scene.GetSceneInstances());

  CopyGeometryToGpu(unique_models, vstg, istg, setup_list);
  CopyTexturesToGpu(unique_models, tstg, setup_list);
  CreateDummyView(setup_list);

  const api::ImageViews& rmat = scene.GetRenderableMaterials();
 
  gbuffer_pass_.CreateUniforms(setup_list, DeferredRenderer::v_max_objects);
  gbuffer_pass_.CreateImages(setup_list);
  gbuffer_pass_.CreateRenderPass();
  gbuffer_pass_.CreateFramebuffer();
  gbuffer_pass_.CreateDescriptorSet(rmat);
  gbuffer_pass_.CreatePipeline();

  for (uint i = 0; i < gfx::v_num_images; ++i)
    deferred_pass_.CreateUniforms(setup_list, i);

  deferred_pass_.CreateImages(setup_list);
  deferred_pass_.CreateRenderPass();
  deferred_pass_.CreateFramebuffer();
  deferred_pass_.CreatePipeline(gbuffer_pass_.data_.image_views_);

  for (uint i = 0; i < gfx::v_num_images; ++i)
  {
    debug_pass_.CreateUniforms(setup_list, i);
    debug_pass_.CreateBuffer(setup_list, i, 128_Kb);
  }
  debug_pass_.CreateImages(setup_list);
  debug_pass_.CreateRenderPass();
  debug_pass_.CreateFramebuffer();
  debug_pass_.CreatePipeline();

  submit_fence_.Reset();

  setup_list.Finalize();
  gfx_.SubmitCommandLists(api::CommandLists{setup_list}, api::Semaphores::empty, api::Semaphores::empty, submit_fence_);
  submit_fence_.WaitSignalFromGpu();
  submit_fence_.Reset();
}

void gdm::DeferredRenderer::Update(Scene& scene)
{
  api::CommandList cmd_gbuffer = gfx_.CreateCommandList(GDM_HASH("Gbuffer"), gfx::ECommandListFlags::SIMULTANEOUS);

  api::Semaphore spresent_done(device_);
  api::Semaphore sgbuffer_done(device_);
  api::Semaphore sdeferred_done(device_);
  api::Semaphore sdebug_done(device_);

  gbuffer_pass_.UpdateUniformsData(scene.GetCamera(), scene.GetRenderableInstances());
  gbuffer_pass_.UpdateUniforms(cmd_gbuffer, DeferredRenderer::v_max_objects);
  gbuffer_pass_.UpdateDescriptorSet(scene.GetRenderableMaterials());
  gbuffer_pass_.Draw(cmd_gbuffer, scene.GetRenderableInstances());

  submit_fence_.Reset();
  cmd_gbuffer.Finalize();
  gfx_.SubmitCommandLists(api::CommandLists{cmd_gbuffer}, api::Semaphores::empty, api::Semaphores{sgbuffer_done}, submit_fence_);
  submit_fence_.WaitSignalFromGpu();

  uint curr_frame = gfx_.AcquireNextFrame(spresent_done, api::Fence::null);
  
  api::CommandList cmd_deferred = gfx_.CreateFrameCommandList(curr_frame, gfx::ECommandListFlags::SIMULTANEOUS);

  deferred_pass_.UpdateUniformsData(curr_frame, scene.GetCamera(), scene.GetLamps(), scene.GetFlashlights());
  deferred_pass_.UpdateUniforms(cmd_deferred, curr_frame);
  deferred_pass_.Draw(cmd_deferred, curr_frame);

  submit_fence_.Reset();
  cmd_deferred.Finalize();

  if (!scene.IsDebugMode())
  {
    gfx_.SubmitCommandLists(api::CommandLists{cmd_deferred}, api::Semaphores{sgbuffer_done}, api::Semaphores{sdeferred_done}, submit_fence_);
    gfx_.SubmitPresentation(curr_frame, api::Semaphores{spresent_done, sdeferred_done});
    submit_fence_.WaitSignalFromGpu();
  }
  else
  {
    gfx_.SubmitCommandLists(api::CommandLists{cmd_deferred}, api::Semaphores{sgbuffer_done}, api::Semaphores{sdeferred_done}, submit_fence_);
    submit_fence_.WaitSignalFromGpu();
  
    api::CommandList cmd_debug = gfx_.CreateFrameCommandList(curr_frame, gfx::ECommandListFlags::SIMULTANEOUS);

    debug_pass_.UpdateUniformsData(curr_frame, scene.GetCamera());
    debug_pass_.UpdateUniforms(cmd_debug, curr_frame);
    debug_pass_.UpdateVertexData(cmd_debug, curr_frame, debug_draw_.GetData());
    debug_pass_.Draw(cmd_debug, curr_frame);
    submit_fence_.Reset();
    cmd_debug.Finalize();

    gfx_.SubmitCommandLists(api::CommandLists{cmd_debug}, api::Semaphores{sdeferred_done}, api::Semaphores{sdebug_done}, submit_fence_);
    gfx_.SubmitPresentation(curr_frame, api::Semaphores{spresent_done, sdebug_done});
    submit_fence_.WaitSignalFromGpu();
  }
}

//--private

auto gdm::DeferredRenderer::CreateStagingBuffer(uint bytes) -> uint
{
  auto buffer = GMNew api::Buffer(&device_, bytes, gfx::TRANSFER_SRC, gfx::HOST_VISIBLE | gfx::HOST_COHERENT);
  staging_buffers_.push_back(buffer);
  return static_cast<uint>(staging_buffers_.size() - 1);
}

void gdm::DeferredRenderer::CopyGeometryToGpu(const std::vector<ModelHandle>& handles, uint vstg_index, uint istg_index, api::CommandList& cmd)
{
  api::Buffer& vstg = *staging_buffers_[vstg_index];
  api::Buffer& istg = *staging_buffers_[istg_index];
  vstg.Map();
  istg.Map();

  uint curr_voffset = 0;
  uint curr_ioffset = 0;  
  std::vector<MeshHandle> meshes_to_load = {};
  std::vector<uint> mesh_voffsets = {};
  std::vector<uint> mesh_ioffsets = {};

  for (auto model_handle : handles)
  {
    AbstractModel* model = ModelFactory::Get(model_handle);

    for (auto mesh_handle : model->meshes_)
      if (!MeshFactory::ImplementationLoaded(mesh_handle))
        meshes_to_load.push_back(mesh_handle);
  }

  for (auto [index, mesh_handle] : Enumerate(meshes_to_load))
  {
    AbstractMesh* mesh = MeshFactory::Get(mesh_handle);

    mesh_voffsets.push_back(curr_voffset);
    vstg.CopyDataToGpu(mesh->interleaving_vxs_buffer_.data(), curr_voffset, mesh->interleaving_vxs_buffer_.size());
    curr_voffset += static_cast<uint>(mesh->interleaving_vxs_buffer_.size() * sizeof(float));
    
    mesh_ioffsets.push_back(curr_ioffset);
    istg.CopyDataToGpu(mesh->faces_.data(), curr_ioffset, mesh->faces_.size());
    curr_ioffset += static_cast<uint>(mesh->faces_.size() * sizeof(Vec3u));

    uint curr_vbuffer_sz = curr_voffset - mesh_voffsets.back();
    auto* vxs_buffer = GMNew api::Buffer(&device_, curr_vbuffer_sz, gfx::VERTEX | gfx::TRANSFER_DST, gfx::DEVICE_LOCAL);
    uint curr_ibuffer_sz = curr_ioffset - mesh_ioffsets.back();
    auto* idx_buffer = GMNew api::Buffer(&device_, curr_ibuffer_sz, gfx::INDEX | gfx::TRANSFER_DST, gfx::DEVICE_LOCAL);
    mesh->SetVertexBuffer(vxs_buffer);
    mesh->SetIndexBuffer(idx_buffer);
  }

  vstg.Unmap();
  istg.Unmap();

  for (std::size_t i = 0; i < meshes_to_load.size(); ++i)
  {
    MeshHandle handle = meshes_to_load[i];
    AbstractMesh* mesh = MeshFactory::Get(meshes_to_load[i]);

    api::Buffer* vx_buffer = mesh->GetVertexBuffer<api::Buffer>();
    uint vxs_size = static_cast<uint>(mesh->interleaving_vxs_buffer_.size() * sizeof(float));
    cmd.CopyBufferToBuffer(vstg, *vx_buffer, mesh_voffsets[i], 0, vxs_size);

    api::Buffer* idx_buffer = mesh->GetIndexBuffer<api::Buffer>();
    uint idx_size = static_cast<uint>(mesh->faces_.size() * sizeof(Vec3f));
    cmd.CopyBufferToBuffer(istg, *idx_buffer, mesh_ioffsets[i], 0, idx_size);
  }
}

void gdm::DeferredRenderer::CopyTexturesToGpu(const std::vector<ModelHandle>& handles, uint tstg_index, api::CommandList& cmd)
{
  api::Buffer& tstg = *staging_buffers_[tstg_index];

  uint curr_offset = 0;
  std::vector<uint> offsets = {};
  std::vector<MaterialHandle> materials_to_load = GetMaterialsToLoad(handles);
  
  tstg.Map();
  for (auto [index, material_handle] : Enumerate(materials_to_load))
  {
    AbstractMaterial* material = MaterialFactory::Get(material_handle);
    auto textures = TextureFactory::Get(material->GetTextureHandles());
    for (auto texture : textures)
    {
      offsets.push_back(curr_offset);
      curr_offset = CopyTextureToStagingBuffer(*texture, tstg, curr_offset);
    }
  }
  tstg.Unmap();

  auto offset = offsets.begin();
  for (auto [index, material_handle] : Enumerate(materials_to_load))
  {
    AbstractMaterial* material = MaterialFactory::Get(material_handle);
    auto textures = TextureFactory::Get(material->GetTextureHandles());
    for (auto texture : textures)
    {
      CopyTextureFromStagingBuffer(cmd, *texture, tstg, *offset);
      ++offset;
    }
  }
}

auto gdm::DeferredRenderer::GetMaterialsToLoad(const std::vector<ModelHandle>& handles) -> std::vector<MaterialHandle>
{
  std::vector<MaterialHandle> result;
  for (auto model_handle : handles)
  {
    AbstractModel* model = ModelFactory::Get(model_handle);
    for (auto material_handle : model->materials_)
      if (!MaterialFactory::ImplementationLoaded(material_handle))
        result.push_back(material_handle);
  }
  return result;
}

uint gdm::DeferredRenderer::CopyTextureToStagingBuffer(AbstractTexture* texture, api::Buffer& stg, uint curr_offset)
{
  AbstractImage* image = ImageFactory::Get(texture->image_);
 
  stg.CopyDataToGpu(image->GetRaw().data(), curr_offset, image->GetRaw().size());
  curr_offset += static_cast<uint>(image->GetRaw().size());

  auto* api_img = GMNew api::Image2D(&device_, image->GetWidth(), image->GetHeight());
  auto* api_img_view = GMNew api::ImageView(device_);
  
  api_img->GetProps()
    .AddFormatType(gfx::UNORM4)
    .AddImageUsage(gfx::TRANSFER_DST_IMG | gfx::SAMPLED)
    .Create();
  api_img_view->GetProps()
    .AddImage(*api_img)
    .AddFormatType(api_img->GetFormat())
    .Create();
  
  texture->SetApiImage(api_img);
  texture->SetApiImageView(api_img_view);

  return curr_offset;
}

void gdm::DeferredRenderer::CopyTextureFromStagingBuffer(api::CommandList& cmd, AbstractTexture* texture, api::Buffer& stg, uint curr_offset)
{
  api::Image2D* img = texture->GetApiImage<api::Image2D>();
  auto& img_raw = ImageFactory::Get(texture->image_)->GetRaw();
  
  api::ImageBarrier barrier_undef_to_transfer;
  api::ImageBarrier barrier_transfer_to_shader;

  barrier_undef_to_transfer.GetProps()
    .AddImage(*img)
    .AddOldLayout(gfx::EImageLayout::UNDEFINED)
    .AddNewLayout(gfx::EImageLayout::TRANSFER_DST_OPTIMAL)
    .Finalize();
  barrier_transfer_to_shader.GetProps()
    .AddImage(*img)
    .AddOldLayout(gfx::EImageLayout::TRANSFER_DST_OPTIMAL)
    .AddNewLayout(gfx::EImageLayout::SHADER_READ_OPTIMAL)
    .Finalize();

  cmd.PushBarrier(barrier_undef_to_transfer);
  cmd.CopyBufferToImage(stg, *img, curr_offset, 0, static_cast<uint>(img_raw.size()));
  cmd.PushBarrier(barrier_transfer_to_shader);
}

void gdm::DeferredRenderer::CreateDummyView(api::CommandList& cmd)
{
  ASSERT(!TextureFactory::Has(v_dummy_image));
  
  ImageHandle image_handle = ImageFactory::Create(v_dummy_image, 1, 1, 32, 1, 1, 1);
  AbstractImage* dummy_image = ImageFactory::Get(image_handle);
  TextureHandle texture_handle = TextureFactory::Load(image_handle);
  AbstractTexture* dummy_texture = TextureFactory::Get(texture_handle);

  api::Image2D* api_img = GMNew api::Image2D(&device_, dummy_image->GetWidth(), dummy_image->GetHeight());
  api::ImageView* api_img_view = GMNew api::ImageView(device_);
  api::ImageBarrier barrier_undef_to_srv;

  api_img->GetProps()
    .AddImageUsage(gfx::TRANSFER_DST_IMG | gfx::SAMPLED)
    .AddFormatType(gfx::UNORM4)
    .Create();
  api_img_view->GetProps()
    .AddImage(*api_img)
    .AddFormatType(api_img->GetFormat())
    .Create();
  barrier_undef_to_srv.GetProps()
    .AddImage(*api_img)
    .AddOldLayout(gfx::EImageLayout::UNDEFINED)
    .AddNewLayout(gfx::EImageLayout::SHADER_READ_OPTIMAL)
    .Finalize();

  cmd.PushBarrier(barrier_undef_to_srv);
  dummy_texture->SetApiImage(api_img);
  dummy_texture->SetApiImageView(api_img_view);
}
