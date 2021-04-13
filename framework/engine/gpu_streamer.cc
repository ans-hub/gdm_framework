// *************************************************************
// File:    gpu_streamer.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "gpu_streamer.h"

#include <set>

#include "system/literals.h"
#include "system/diff_utils.h"

#include "defines.h"

// --public

gdm::GpuStreamer::GpuStreamer(api::Renderer& gfx)
  : gfx_{gfx}
  , device_{gfx.GetDevice()}
  , staging_buffers_{}
{
  staging_buffers_.reserve(v_max_buffers_);
}

auto gdm::GpuStreamer::CreateStagingBuffer(uint bytes) -> uint
{
  auto buffer = GMNew api::Buffer(&device_, bytes, gfx::TRANSFER_SRC, gfx::HOST_VISIBLE | gfx::HOST_COHERENT);
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
  api::CommandList setup_list = gfx_.CreateCommandList(GDM_HASH("SceneSetup"), gfx::ECommandListFlags::ONCE);
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
  gfx_.SubmitCommandLists(api::CommandLists{setup_list}, api::Semaphores::empty, api::Semaphores::empty, submit_fence);
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

void gdm::GpuStreamer::CopyMaterialsToGpu(const std::vector<MaterialHandle>& handles, uint tstg_index, api::CommandList& cmd)
{
  api::Buffer& tstg = *staging_buffers_[tstg_index];

  uint curr_offset = 0;
  std::vector<uint> offsets = {};
  
  tstg.Map();
  for (auto [index, material_handle] : Enumerate(handles))
  {
    AbstractMaterial* material = MaterialFactory::Get(material_handle);
    std::vector<AbstractTexture**> textures = TextureFactory::Get(material->GetTextureHandles());

    for (auto texture : textures)
    {
      offsets.push_back(curr_offset);
      curr_offset = CopyTextureToStagingBuffer(*texture, tstg, curr_offset);
    }
  }
  tstg.Unmap();

  auto offset = offsets.begin();
  for (auto [index, material_handle] : Enumerate(handles))
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

void gdm::GpuStreamer::CopyTexturesToGpu(const std::vector<TextureHandle>& handles, uint tstg_index)
{
  api::CommandList setup_list = gfx_.CreateCommandList(GDM_HASH("SceneSetup"), gfx::ECommandListFlags::ONCE);
  api::Fence submit_fence(device_);

  CopyTexturesToGpu(handles, tstg_index, setup_list);

  setup_list.Finalize();
  gfx_.SubmitCommandLists(api::CommandLists{setup_list}, api::Semaphores::empty, api::Semaphores::empty, submit_fence);
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
  AbstractImage* image = ImageFactory::Get(texture->image_);
 
  stg.CopyDataToGpu(image->GetRaw().data(), curr_offset, image->GetRaw().size());
  curr_offset += static_cast<uint>(image->GetRaw().size());

  auto* api_img = GMNew api::Image2D(&device_, image->GetWidth(), image->GetHeight());
  auto* api_img_view = GMNew api::ImageView(device_);
  
  gfx::EFormatType texture_format = gfx::EFormatType::FORMAT_TYPE_MAX;
  
  if (texture->format_ == AbstractTexture::EFormatType::FORMAT_TYPE_MAX)
    texture_format = v_default_texture_fmt;
  else
    texture_format = helpers::ConvertData2RenderTextureFormat(texture->format_);

  api_img->GetProps()
    .AddFormatType(texture_format)
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

void gdm::GpuStreamer::CopyTextureFromStagingBuffer(api::CommandList& cmd, AbstractTexture* texture, api::Buffer& stg, uint curr_offset)
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

//--private

void gdm::GpuStreamer::CreateDummyView(api::CommandList& cmd)
{
  ASSERT(!TextureFactory::Has(cfg::v_dummy_image));
  
  ImageHandle image_handle = ImageFactory::Create(cfg::v_dummy_image, 1, 1, 32, 1, 1, 1);
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

auto gdm::helpers::ConvertData2RenderTextureFormat(AbstractTexture::EFormatType type) -> gfx::EFormatType
{
  switch(type)
  {
    case AbstractTexture::EFormatType::F1: return gfx::EFormatType::F1;
    case AbstractTexture::EFormatType::F2: return gfx::EFormatType::F2;
    case AbstractTexture::EFormatType::F3: return gfx::EFormatType::F3;
    case AbstractTexture::EFormatType::F4: return gfx::EFormatType::F4;
    case AbstractTexture::EFormatType::F4HALF: return gfx::EFormatType::F4HALF;
    case AbstractTexture::EFormatType::SRGB4: return gfx::EFormatType::SRGB4;
    case AbstractTexture::EFormatType::UNORM4: return gfx::EFormatType::UNORM4;
    case AbstractTexture::EFormatType::D24_UNORM_S8_UINT: return gfx::EFormatType::D24_UNORM_S8_UINT;
    case AbstractTexture::EFormatType::D32_SFLOAT_S8_UINT: return gfx::EFormatType::D32_SFLOAT_S8_UINT;
    case AbstractTexture::EFormatType::R8_UNORM: return gfx::EFormatType::R8_UNORM;
    case AbstractTexture::EFormatType::D16_UNORM: return gfx::EFormatType::D16_UNORM;
    default:
    {
      ASSERTF(false, "No association with texture type");
      return gfx::EFormatType::FORMAT_TYPE_MAX;
    }
  }
}
