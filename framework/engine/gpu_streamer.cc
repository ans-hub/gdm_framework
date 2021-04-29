// *************************************************************
// File:    gpu_streamer.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "gpu_streamer.h"

#include <unordered_set>

#include "system/literals.h"
#include "system/diff_utils.h"

#include "render/texture.h"

#include "defines.h"

// --public

gdm::GpuStreamer::GpuStreamer(api::Renderer& ctx)
  : ctx_{ctx}
  , device_{ctx.GetDevice()}
  , staging_buffers_{}
{
  staging_buffers_.reserve(v_max_buffers_);
}

gdm::GpuStreamer::~GpuStreamer()
{
  for (auto&& buffer : staging_buffers_)
    GMDelete(buffer);
  // todo: release images and buffers
}

auto gdm::GpuStreamer::CreateStagingBuffer(uint bytes) -> uint
{
  api::Buffer* buffer = api::Resource<api::Buffer>(&device_, bytes)
    .AddUsage(gfx::TRANSFER_SRC)
    .AddMemoryType(gfx::HOST_VISIBLE | gfx::HOST_COHERENT);

  staging_buffers_.push_back(buffer);

  return static_cast<uint>(staging_buffers_.size() - 1);
}

int gdm::GpuStreamer::FindStagingBuffer(uint min_size)
{
  for (const auto& [i,buffer] : Enumerate(staging_buffers_))
    if (buffer->GetSize() >= min_size)
      return i;

  return -1; 
}

void gdm::GpuStreamer::CopyModelsToGpu(const std::vector<ModelHandle>& models)
{
  api::CommandList setup_list = ctx_.CreateCommandList(GDM_HASH("SceneSetup"), gfx::ECommandListFlags::ONCE);
  api::Fence submit_fence(device_);

  uint vstg = CreateStagingBuffer(64_Mb);
  uint istg = CreateStagingBuffer(32_Mb);
  uint tstg = CreateStagingBuffer(96_Mb);
  
  std::vector<MaterialHandle> materials = helpers::GetMaterialsToLoad(models);

  CopyGeometryToGpu(models, vstg, istg, setup_list);
  CopyMaterialsToGpu(materials, tstg, setup_list);

  if (!TextureFactory::Has(cfg::v_dummy_image))
    CreateDummyView(setup_list);

  setup_list.Finalize();
  ctx_.SubmitCommandLists(api::CommandLists{setup_list}, api::Semaphores::empty, api::Semaphores::empty, submit_fence);
  submit_fence.WaitSignalFromGpu();
}

void gdm::GpuStreamer::CopyGeometryToGpu(const std::vector<ModelHandle>& handles, uint vstg_index, uint istg_index, api::CommandList& cmd)
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

    api::Buffer* vxs_buffer = api::Resource<api::Buffer>(&device_, curr_vbuffer_sz)
      .AddUsage(gfx::VERTEX | gfx::TRANSFER_DST)
      .AddMemoryType(gfx::DEVICE_LOCAL);

    uint curr_ibuffer_sz = curr_ioffset - mesh_ioffsets.back();

    api::Buffer* idx_buffer = api::Resource<api::Buffer>(&device_, curr_ibuffer_sz)
      .AddUsage(gfx::INDEX | gfx::TRANSFER_DST)
      .AddMemoryType(gfx::DEVICE_LOCAL);

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

void gdm::GpuStreamer::CopyMaterialsToGpu(const std::vector<MaterialHandle>& handles, uint tstg_index, api::CommandList& cmd)
{
  api::Buffer& tstg = *staging_buffers_[tstg_index];

  uint curr_offset = 0;
  std::vector<std::pair<uint, AbstractTexture*>> offsets = {};
  std::unordered_set<AbstractTexture*> processed = {}; 

  tstg.Map();
  for (auto [index, material_handle] : Enumerate(handles))
  {
    if (MaterialFactory::ImplementationLoaded(material_handle))
      continue;

    AbstractMaterial* material = MaterialFactory::Get(material_handle);
    std::vector<AbstractTexture**> textures = TextureFactory::Get(material->GetTextureHandles());

    for (auto texture : textures)
    {
      if ((*texture)->HasTextureImpl() || processed.find(*texture) != processed.end())
        continue;

      processed.insert(*texture);
      offsets.push_back(std::make_pair(curr_offset, *texture));
      curr_offset = CopyTextureToStagingBuffer(*texture, tstg, curr_offset);
    }
  }
  tstg.Unmap();

  for (auto&& [offset, abstract_texture] : offsets)
    CopyTextureFromStagingBuffer(cmd, abstract_texture, tstg, offset);
}

void gdm::GpuStreamer::CopyTexturesToGpu(const std::vector<TextureHandle>& handles, uint tstg_index)
{
  api::CommandList setup_list = ctx_.CreateCommandList(GDM_HASH("SceneSetup"), gfx::ECommandListFlags::ONCE);
  api::Fence submit_fence(device_);

  CopyTexturesToGpu(handles, tstg_index, setup_list);

  setup_list.Finalize();
  ctx_.SubmitCommandLists(api::CommandLists{setup_list}, api::Semaphores::empty, api::Semaphores::empty, submit_fence);
  submit_fence.WaitSignalFromGpu();
}

void gdm::GpuStreamer::CopyTexturesToGpu(const std::vector<TextureHandle>& handles, uint tstg_index, api::CommandList& cmd)
{
  api::Buffer& tstg = *staging_buffers_[tstg_index];

  uint curr_offset = 0;
  std::vector<uint> offsets = {};
  
  tstg.Map();
  for (auto [index, texture_handle] : Enumerate(handles))
  {
    AbstractTexture* texture = TextureFactory::Get(texture_handle);
    offsets.push_back(curr_offset);
    curr_offset = CopyTextureToStagingBuffer(texture, tstg, curr_offset);
  }
  tstg.Unmap();

  auto offset = offsets.begin();
  for (auto [index, texture_handle] : Enumerate(handles))
  {
    AbstractTexture* texture = TextureFactory::Get(texture_handle);
    CopyTextureFromStagingBuffer(cmd, texture, tstg, *offset);
    ++offset;
  }
}

//--private

uint gdm::GpuStreamer::CopyTextureToStagingBuffer(AbstractTexture* texture, api::Buffer& stg, uint curr_offset)
{
  AbstractImage* image = ImageFactory::Get(texture->GetImageHandle()); 
  stg.CopyDataToGpu(image->GetRaw().data(), curr_offset, image->GetRaw().size());
  curr_offset += static_cast<uint>(image->GetRaw().size());  

  return curr_offset;
}

void gdm::GpuStreamer::CopyTextureFromStagingBuffer(api::CommandList& cmd, AbstractTexture* texture, api::Buffer& stg, uint curr_offset)
{
  const AbstractImage* image = ImageFactory::Get(texture->GetImageHandle()); 
  uint img_raw_size = ImageFactory::Get(texture->GetImageHandle())->GetRaw().size();

  gfx::Texture* gfx_texture = GMNew gfx::Texture(gfx::tag::SR{}, texture->GetFormat(), image->GetWHD(), cmd, &ctx_.GetDevice()); 
  texture->SetTextureImpl(gfx_texture);

  api::Image& api_img = gfx_texture->GetImageImpl();
  
  api::ImageBarrier* barrier_undef_to_transfer = api::Resource<api::ImageBarrier>(&ctx_.GetDevice())
    .AddImage(api_img)
    .AddOldLayout(gfx::EImageLayout::UNDEFINED)
    .AddNewLayout(gfx::EImageLayout::TRANSFER_DST_OPTIMAL);

  api::ImageBarrier* barrier_transfer_to_shader = api::Resource<api::ImageBarrier>(&ctx_.GetDevice())
    .AddImage(api_img)
    .AddOldLayout(gfx::EImageLayout::TRANSFER_DST_OPTIMAL)
    .AddNewLayout(gfx::EImageLayout::SHADER_READ_OPTIMAL);

  cmd.PushBarrier(*barrier_undef_to_transfer);
  cmd.CopyBufferToImage(stg, api_img, curr_offset, 0, static_cast<uint>(img_raw_size));
  cmd.PushBarrier(*barrier_transfer_to_shader);

  GMDelete(barrier_undef_to_transfer);
  GMDelete(barrier_transfer_to_shader);
}

//--private

void gdm::GpuStreamer::CreateDummyView(api::CommandList& cmd)
{
  ASSERT(!TextureFactory::Has(cfg::v_dummy_image));
  
  ImageHandle image_handle = ImageFactory::Create(cfg::v_dummy_image, 1, 1, 32, 1, 1, 1);
  AbstractImage* dummy_image = ImageFactory::Get(image_handle);
  TextureHandle texture_handle = TextureFactory::Load(image_handle, gfx::EFormatType::UNORM4);
  AbstractTexture* dummy_texture = TextureFactory::Get(texture_handle);

  gfx::Texture* gfx_texture = GMNew gfx::Texture(gfx::tag::SR{}, dummy_texture->GetFormat(), dummy_image->GetWHD(), cmd, &ctx_.GetDevice()); 
  dummy_texture->SetTextureImpl(gfx_texture);

  api::ImageBarrier* barrier_undef_to_srv = api::Resource<api::ImageBarrier>(&ctx_.GetDevice())
    .AddImage(gfx_texture->GetImageImpl())
    .AddOldLayout(gfx::EImageLayout::UNDEFINED)
    .AddNewLayout(gfx::EImageLayout::SHADER_READ_OPTIMAL);

  cmd.PushBarrier(*barrier_undef_to_srv);

  GMDelete(barrier_undef_to_srv);
}

//--helpers

auto gdm::helpers::GetMaterialsToLoad(const std::vector<ModelHandle>& handles) -> std::vector<MaterialHandle>
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
