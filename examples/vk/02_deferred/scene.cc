// *************************************************************
// File:    scene.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "scene.h"

#include "data/cfg_loader.h"
#include "system/assert_utils.h"

// --public

gdm::Scene::Scene(api::Device& device)
  : device_{ device }
  , models_{}
{ }

auto gdm::Scene::LoadAbstractModels(const Config& cfg) -> std::vector<ModelHandle>
{
  std::vector<ModelHandle> result;

  auto obj_pathes = cfg.GetAllVals<std::string>("model_");
  auto obj_poses = cfg.GetAllVals<Vec4f>("model_pos_");

  ASSERT(obj_pathes.size() == obj_poses.size());

  for (std::size_t i = 0; i < obj_pathes.size(); ++i)
  {
    const char* model_fpath = obj_pathes[i].c_str();
    ModelHandle model_handle = {};
    if (ModelFactory::Has(model_fpath))
      model_handle = ModelFactory::GetHandle(model_fpath);
    else
      model_handle = ModelFactory::Load(model_fpath);
    result.push_back(model_handle);
    models_.emplace(model_handle);
  }
  for (std::size_t i = 0; i < obj_poses.size(); ++i)
  {
    AbstractModel* model = ModelFactory::Get(result[i]);
    model->tm_ = Mat4f(1.f);
    model->tm_.SetCol(3, obj_poses[i].xyz());
    Mat4f tm = matrix::MakeScale(obj_poses[i][3]) % model->tm_;
    model->tm_ = tm;
  }
  return result;
}

void gdm::Scene::CopyGeometryToGpu(const std::vector<ModelHandle>& handles, api::Buffer& vstg, api::Buffer& istg, api::CommandList& cmd)
{
  uint curr_voffset = 0;
  uint curr_ioffset = 0;  
  std::vector<std::vector<uint>> voffsets = {};
  std::vector<std::vector<uint>> ioffsets = {};

  vstg.Map();
  istg.Map();

  for (auto model_handle : handles)
  {
    AbstractModel* model = ModelFactory::Get(model_handle);
 
    voffsets.push_back({});
    ioffsets.push_back({});

    for (auto mesh_handle : model->meshes_)
    {
      AbstractMesh* mesh = MeshFactory::Get(mesh_handle);
     
      voffsets.back().push_back(curr_voffset);
      vstg.CopyDataToGpu(mesh->interleaving_vxs_buffer_.data(), curr_voffset, mesh->interleaving_vxs_buffer_.size());
      curr_voffset += static_cast<uint>(mesh->interleaving_vxs_buffer_.size() * sizeof(float));
      
      ioffsets.back().push_back(curr_ioffset);
      istg.CopyDataToGpu(mesh->faces_.data(), curr_ioffset, mesh->faces_.size());
      curr_ioffset += static_cast<uint>(mesh->faces_.size() * sizeof(Vec3u));

      auto* vxs_buffer = GMNew api::Buffer(&device_, curr_voffset, gfx::VERTEX | gfx::TRANSFER_DST, gfx::DEVICE_LOCAL);
      auto* idx_buffer = GMNew api::Buffer(&device_, curr_ioffset, gfx::INDEX | gfx::TRANSFER_DST, gfx::DEVICE_LOCAL);
      mesh->SetVertexBuffer(vxs_buffer);
      mesh->SetIndexBuffer(idx_buffer);
    }
  }
  vstg.Unmap();
  istg.Unmap();

  for (std::size_t i = 0; i < handles.size(); ++i)
  {
    AbstractModel* model = ModelFactory::Get(handles[i]);
    for (std::size_t k = 0; k < model->meshes_.size(); ++k)
    {
      AbstractMesh* mesh = MeshFactory::Get(model->meshes_[k]);
      api::Buffer* vx_buffer = mesh->GetVertexBuffer<api::Buffer>();
      uint vxs_size = static_cast<uint>(mesh->interleaving_vxs_buffer_.size() * sizeof(float));
      cmd.CopyBufferToBuffer(vstg, *vx_buffer, voffsets[i][k], 0, vxs_size);
    }
  }

  for (std::size_t i = 0; i < handles.size(); ++i)
  {
    AbstractModel* model = ModelFactory::Get(handles[i]);
    for (std::size_t k = 0; k < model->meshes_.size(); ++k)
    {
      AbstractMesh* mesh = MeshFactory::Get(model->meshes_[k]);
      api::Buffer* idx_buffer = mesh->GetIndexBuffer<api::Buffer>();
      uint idx_size = static_cast<uint>(mesh->faces_.size() * sizeof(Vec3f));
      cmd.CopyBufferToBuffer(istg, *idx_buffer, ioffsets[i][k], 0, idx_size);
    }
  }
}

void gdm::Scene::CopyTexturesToGpu(const std::vector<ModelHandle>& handles, api::Buffer& tstg, api::CommandList& cmd)
{
  using Offset = std::array<uint, 3>;

  Offset curr_offset = {0,0,0};
  std::vector<std::vector<Offset>> offsets = {};
  tstg.Map();

  for (auto model_handle : handles)
  {
    AbstractModel* model = ModelFactory::Get(model_handle);
    offsets.push_back({});
    auto& model_offsets = offsets.back();

    for (auto mesh_handle : model->meshes_)
    {
      model_offsets.push_back({});
      auto& mesh_offsets = model_offsets.back();

      AbstractMesh* mesh = MeshFactory::Get(mesh_handle);
      AbstractMaterial* mat = MaterialFactory::Get(mesh->material_);
      std::vector<TextureHandle> tex_handles = mat->GetTextures();
      auto tex = TextureFactory::Get(tex_handles);

      std::vector<AbstractImage*> images;
      std::vector<AbstractImage::StorageType> raw;
      for(std::size_t i = 0; i < tex.size(); ++i)
      {
        ImageHandle image_handle = (*tex[i])->image_;
        images.push_back(ImageFactory::Get(image_handle));
        raw.push_back(images.back()->GetRaw());
      }
      
      for (std::size_t i = 0; i < raw.size(); ++i)
      {
        mesh_offsets[i] = curr_offset[i];
        tstg.CopyDataToGpu(raw[i].data(), curr_offset[i], raw[i].size());
        curr_offset[i] += static_cast<uint>(raw[i].size());
        gfx::ImageUsage img_usage = gfx::TRANSFER_DST_IMG | gfx::SAMPLED;
        gfx::FormatType img_format = gfx::UNORM4;
        uint img_w = static_cast<uint>(images[i]->GetWidth());
        uint img_h = static_cast<uint>(images[i]->GetHeight());
        auto* img = GMNew api::Image2D(&device_, img_w, img_h, img_usage, img_format);
        auto* img_view = GMNew api::ImageView(api::helpers::CreateImageView(device_, *img, img->GetFormat()));
        AbstractTexture* curr_tex = (*tex[i]);
        curr_tex->SetImageBuffer(img);
        curr_tex->SetImageView(img_view);
      }
    }
  }
  tstg.Unmap();

  for (std::size_t i = 0; i < handles.size(); ++i)
  {
    AbstractModel* model = ModelFactory::Get(handles[i]);
    for (std::size_t k = 0; k < model->meshes_.size(); ++k)
    {
      AbstractMesh* mesh = MeshFactory::Get(model->meshes_[k]);
      AbstractMaterial* material = MaterialFactory::Get(mesh->material_);
      auto textures = TextureFactory::Get(material->GetTextures());
      
      for (std::size_t j = 0; j < textures.size(); ++j)
      {
        AbstractTexture* tex = *textures[j];
        api::Image2D* img = tex->GetImageBuffer<api::Image2D>();
        auto barrier_undef_to_transfer = api::ImageBarrier(&device_, *img, gfx::EImageLayout::UNDEFINED, gfx::EImageLayout::TRANSFER_DST_OPTIMAL);
        auto barrier_transfer_to_shader = api::ImageBarrier(&device_, *img, gfx::EImageLayout::TRANSFER_DST_OPTIMAL, gfx::EImageLayout::SHADER_READ_OPTIMAL);
        auto& img_raw = ImageFactory::Get(tex->image_)->GetRaw();
        cmd.PushBarrier(barrier_undef_to_transfer);
        cmd.CopyBufferToImage(tstg, *img, offsets[i][k][j], 0, static_cast<uint>(img_raw.size()));
        cmd.PushBarrier(barrier_transfer_to_shader);
      }
    }
  }
}

void gdm::Scene::UpdateCamera(CameraEul& cam, MainInput& input, float dt)
{
  cam.Rotate(input.GetMouseY(), input.GetMouseX());

  dt += dt * static_cast<int>(input.IsKeyboardBtnHold(DIK_LSHIFT)) * 2;

  if (input.IsKeyboardBtnHold(DIK_W))
    cam.Move(-cam.GetTm().GetCol(0), dt);
  if (input.IsKeyboardBtnHold(DIK_S))
    cam.Move(cam.GetTm().GetCol(0), dt);
  if (input.IsKeyboardBtnHold(DIK_A))
    cam.Move(-cam.GetTm().GetCol(2), dt);
  if (input.IsKeyboardBtnHold(DIK_D))
    cam.Move(cam.GetTm().GetCol(2), dt);
  if (input.IsKeyboardBtnHold(DIK_R))
    cam.Move(cam.GetTm().GetCol(1), dt);
  if (input.IsKeyboardBtnHold(DIK_F))
    cam.Move(-cam.GetTm().GetCol(1), dt);
}
