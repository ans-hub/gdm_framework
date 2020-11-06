// *************************************************************
// File:    scene_manager.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "scene_manager.h"

#include "data/cfg_loader.h"
#include "memory/defines.h"
#include "render/api.h"
#include "system/assert_utils.h"

// --public

gdm::SceneManager::SceneManager(api::Device& device)
  : device_{ device }
  , models_{}
  , staging_buffers_{}
  , dummy_view_{}
  , pocb_uniform_{}
  , pocb_staging_{}
  , pocb_to_write_barriers_{}
  , pocb_to_read_barriers_{}
  , pfcb_uniform_{}
  , pfcb_staging_{}
  , pfcb_to_write_barriers_{}
  , pfcb_to_read_barriers_{}
{ }

auto gdm::SceneManager::CreateStagingBuffer(uint bytes) -> uint
{
  auto buffer = GMNew api::Buffer(&device_, bytes, gfx::TRANSFER_SRC, gfx::HOST_VISIBLE | gfx::HOST_COHERENT);
  staging_buffers_.push_back(buffer);
  return static_cast<uint>(staging_buffers_.size() - 1);
}

void gdm::SceneManager::SetModels(const std::vector<ModelHandle>& models)
{
  for (auto handle : models)
    models_.emplace(handle);
}

void gdm::SceneManager::CopyGeometryToGpu(const std::vector<ModelHandle>& handles, uint vstg_index, uint istg_index, api::CommandList& cmd)
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

  for (auto mesh_handle : meshes_to_load)
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

void gdm::SceneManager::CopyTexturesToGpu(const std::vector<ModelHandle>& handles, uint tstg_index, api::CommandList& cmd)
{
  api::Buffer& tstg = *staging_buffers_[tstg_index];

  uint curr_offset = 0;
  std::vector<uint> offsets = {};
  std::vector<MaterialHandle> materials_to_load = GetMaterialsToLoad(handles);
  
  tstg.Map();
  for (auto material_handle : materials_to_load)
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
  for (auto material_handle : materials_to_load)
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

// --private

auto gdm::SceneManager::GetMaterialsToLoad(const std::vector<ModelHandle>& handles) -> std::vector<MaterialHandle>
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

uint gdm::SceneManager::CopyTextureToStagingBuffer(AbstractTexture* texture, api::Buffer& stg, uint curr_offset)
{
  AbstractImage* image = ImageFactory::Get(texture->image_);
 
  stg.CopyDataToGpu(image->GetRaw().data(), curr_offset, image->GetRaw().size());
  curr_offset += static_cast<uint>(image->GetRaw().size());

  gfx::ImageUsage img_usage = gfx::TRANSFER_DST_IMG | gfx::SAMPLED;
  gfx::FormatType img_format = gfx::UNORM4;
  uint img_w = static_cast<uint>(image->GetWidth());
  uint img_h = static_cast<uint>(image->GetHeight());
  auto* api_img = GMNew api::Image2D(&device_, img_w, img_h, img_usage, img_format);
  auto* api_img_view = GMNew api::ImageView(device_, *api_img, api_img->GetFormat());
  texture->SetApiImageBuffer(api_img);
  texture->SetApiImageView(api_img_view);

  return curr_offset;
}

void gdm::SceneManager::CopyTextureFromStagingBuffer(api::CommandList& cmd, AbstractTexture* texture, api::Buffer& stg, uint curr_offset)
{
  api::Image2D* img = texture->GetImageBuffer<api::Image2D>();
  auto& img_raw = ImageFactory::Get(texture->image_)->GetRaw();
  auto barrier_undef_to_transfer = api::ImageBarrier(&device_, *img, gfx::EImageLayout::UNDEFINED, gfx::EImageLayout::TRANSFER_DST_OPTIMAL);
  auto barrier_transfer_to_shader = api::ImageBarrier(&device_, *img, gfx::EImageLayout::TRANSFER_DST_OPTIMAL, gfx::EImageLayout::SHADER_READ_OPTIMAL);
  cmd.PushBarrier(barrier_undef_to_transfer);
  cmd.CopyBufferToImage(stg, *img, curr_offset, 0, static_cast<uint>(img_raw.size()));
  cmd.PushBarrier(barrier_transfer_to_shader);
}

void gdm::SceneManager::CreateDummyView(api::CommandList& cmd)
{
  ASSERT(!TextureFactory::Has(v_dummy_image));
  
  ImageHandle image_handle = ImageFactory::Create(v_dummy_image, 1, 1, 32, 1, 0, 0);
  AbstractImage* dummy_image = ImageFactory::Get(image_handle);
  TextureHandle texture_handle = TextureFactory::Load(image_handle);
  AbstractTexture* dummy_texture = TextureFactory::Get(texture_handle);

  const gfx::ImageUsage img_usage = gfx::TRANSFER_DST_IMG | gfx::SAMPLED;
  const gfx::FormatType img_format = gfx::UNORM4;
  const uint img_w = static_cast<uint>(dummy_image->GetWidth());
  const uint img_h = static_cast<uint>(dummy_image->GetHeight());
  
  auto* api_img = GMNew api::Image2D(&device_, img_w, img_h, img_usage, img_format);
  auto* api_img_view = GMNew api::ImageView(device_, *api_img, api_img->GetFormat());
  dummy_texture->SetApiImageBuffer(api_img);
  dummy_texture->SetApiImageView(api_img_view);

  auto barrier_undef_to_srv = api::ImageBarrier(&device_, *api_img, gfx::EImageLayout::UNDEFINED, gfx::EImageLayout::SHADER_READ_OPTIMAL);
  cmd.PushBarrier(barrier_undef_to_srv);
}

auto gdm::SceneManager::GetRenderableMaterials() -> RenderableMaterials
{
  TextureHandle dummy_handle = dummy_handle = TextureFactory::GetHandle(v_dummy_image);
  AbstractTexture* dummy_texture = TextureFactory::Get(dummy_handle);
  api::ImageView* dummy_view = dummy_texture->GetImageView<api::ImageView>();

  RenderableMaterials materials = {};
  materials.diffuse_views_.resize(v_max_materials, dummy_view);
  materials.specular_views_.resize(v_max_materials, dummy_view);
  materials.normal_views_.resize(v_max_materials, dummy_view);

  for (auto model_handle : GetRenderableModels())
  {
    auto model = ModelFactory::Get(model_handle);
    for (auto mesh_handle : model->meshes_)
    {
      auto mesh = MeshFactory::Get(mesh_handle);
      auto material = MaterialFactory::Get(mesh->material_);

      auto diffuse_texture = TextureFactory::Get(material->diff_);
      materials.diffuse_views_[material->index_] = diffuse_texture->GetImageView<api::ImageView>();
      auto specular_texture = TextureFactory::Get(material->spec_);
      materials.specular_views_[material->index_] = specular_texture->GetImageView<api::ImageView>();
      auto normal_texture = TextureFactory::Get(material->norm_);
      materials.normal_views_[material->index_] = normal_texture->GetImageView<api::ImageView>();
    }
  }
  return materials;
}
