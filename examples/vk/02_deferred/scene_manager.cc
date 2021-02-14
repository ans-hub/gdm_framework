// *************************************************************
// File:    scene_manager.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "scene_manager.h"

#include "system/diff_utils.h"
#include "render/shader.h"

#include "desc/sampler_desc.h"
#include "desc/input_layout.h"
#include "desc/rasterizer_desc.h"

// --public

gdm::SceneManager::SceneManager(api::Renderer& rdr)
  : device_{ rdr.GetDevice() }
  , rdr_{rdr}
  , models_{}
  , lamps_{}
  , flashlights_{}
  , staging_buffers_{}
  , dummy_view_{}
  , renderable_materials_{}
{ }

auto gdm::SceneManager::CreateStagingBuffer(uint bytes) -> uint
{
  auto buffer = GMNew api::Buffer(&device_, bytes, gfx::TRANSFER_SRC, gfx::HOST_VISIBLE | gfx::HOST_COHERENT);
  staging_buffers_.push_back(buffer);
  return static_cast<uint>(staging_buffers_.size() - 1);
}

void gdm::SceneManager::SetObjects(const std::vector<ModelInstance>& objs, const std::vector<std::string>& names)
{
  for (auto [instance, name] : range::ZipSpan(objs, names))
  {
    models_.push_back(instance);
    models_names_.push_back(name);
  }
}

void gdm::SceneManager::SetLamps(const std::vector<ModelInstance>& lamps, const std::vector<ModelInstance>& flashlights)
{
  for (auto instance : lamps)
  {
    models_.push_back(instance);
    models_names_.push_back("Light");

    lamps_.push_back({});
    lamps_.back().instance_ = instance;
    lamps_.back().enabled_ = true;
  }
  for (auto instance : flashlights)
  {
    flashlights_.push_back({});
    flashlights_.back().instance_ = instance;
    flashlights_.back().enabled_ = true;
  }
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

void gdm::SceneManager::CopyTexturesToGpu(const std::vector<ModelHandle>& handles, uint tstg_index, api::CommandList& cmd)
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

void gdm::SceneManager::CopyTextureFromStagingBuffer(api::CommandList& cmd, AbstractTexture* texture, api::Buffer& stg, uint curr_offset)
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

void gdm::SceneManager::CreateDummyView(api::CommandList& cmd)
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

auto gdm::SceneManager::GetRenderableInstances() -> std::vector<ModelInstance*>
{
  std::vector<ModelInstance*> renderables;
 
  for (auto& instance : models_)
    renderables.push_back(&instance);
    
  return renderables;
}

auto gdm::SceneManager::GetSceneInstances() -> std::vector<ModelInstance*>
{
  std::vector<ModelInstance*> scene_models;
 
  for (auto& instance : models_)
    scene_models.push_back(&instance);
    
  return scene_models;
}

auto gdm::SceneManager::GetSceneInstancesNames() -> std::vector<std::string>&
{
  return models_names_;
}

auto gdm::SceneManager::GetRenderableMaterials() -> const api::ImageViews&
{
  if (!dummy_view_)
  {
    TextureHandle dummy_handle = dummy_handle = TextureFactory::GetHandle(v_dummy_image);
    AbstractTexture* dummy_texture = TextureFactory::Get(dummy_handle);
    dummy_view_ = dummy_texture->GetApiImageView<api::ImageView>();
  }

  renderable_materials_.clear();
  renderable_materials_.resize(v_max_materials * v_material_type_cnt, dummy_view_);
  
  std::vector<ModelInstance*> renderable = GetRenderableInstances();

  for (auto [index,model_instance] : Enumerate(renderable))
  {
    auto model = ModelFactory::Get(model_instance->handle_);
    for (auto mesh_handle : model->meshes_)
    {
      auto mesh = MeshFactory::Get(mesh_handle);
      auto material = MaterialFactory::Get(mesh->material_);
      auto diffuse_texture = TextureFactory::Get(material->diff_);
      auto normal_texture = TextureFactory::Get(material->norm_);
      auto specular_texture = TextureFactory::Get(material->spec_);
      
      renderable_materials_[material->index_ * 3 + v_diff_offset] = diffuse_texture->GetApiImageView<api::ImageView>();
      renderable_materials_[material->index_ * 3 + v_norm_offset] = normal_texture->GetApiImageView<api::ImageView>();
      renderable_materials_[material->index_ * 3 + v_spec_offset] = specular_texture->GetApiImageView<api::ImageView>();
    }
  }
  return renderable_materials_;
}
