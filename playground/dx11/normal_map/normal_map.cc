// *************************************************************
// File:    normal_map.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GFX_DX_API
#define GFX_DX_API
#endif

#define GM_MATRIX_INV_TRANSPOSE_TYPE 1

#include <windows.h>
#include <timeapi.h>

#include <iostream>
#include <string>

#include <system/assert_utils.h>
#include <system/timer.h>
#include <system/fps_counter.h>
#include <system/string_utils.h>

#include <math/general.h>
#include <math/matrix.h>
#include <math/vector3.h>

#include <window/win/main_window.h>
#include <window/win/dx_input.h>

#include <render/dx11/dx_material.h>
#include <render/dx11/dx_object.h>
#include <render/dx11/dx_renderer.h>
#include <render/dx11/dx_texture.h>
#include <render/dx11/dx_uniform.h>
#include <render/dx11/dx_defines.h>
#include <render/dx11/dx_vertex_shader.h>
#include <render/dx11/dx_pixel_shader.h>

#include <render/desc/rasterizer_desc.h>
#include <render/desc/input_layout_desc.h>
#include <render/desc/sampler_desc.h>
#include <render/vertex_shader.h>
#include <render/pixel_shader.h>
#include <render/colors.h>

#include <engine/camera_eul.h>

#include "shaders/flat_vert.h"
#include "shaders/nmap_vert.h"
#include "shaders/nmap_frag.h"

using namespace gdm;

enum class ShaderType
{
  FLAT,
  NMAP  
};

struct Shaders
{
  InputLayout layout_std_;
  SamplerDesc sampler_std_;
  VertexShader flat_sh_vx_;
  PixelShader flat_sh_px_;
  PixelShader affn_sh_px_;
  VertexShader nmap_sh_vx_;
  PixelShader nmap_sh_px_;

  Shaders(Renderer& d3d)
    : layout_std_{ StdInputLayout{} }
    , sampler_std_{ StdSamplerDesc{} }
    , flat_sh_vx_{"playground/dx11/normal_map/shaders/flat_vert.hlsl", layout_std_, d3d}
    , flat_sh_px_{"playground/dx11/normal_map/shaders/flat_frag.hlsl", sampler_std_, d3d}
    , affn_sh_px_{"playground/dx11/normal_map/shaders/affine_frag.hlsl", sampler_std_, d3d}
    , nmap_sh_vx_{"playground/dx11/normal_map/shaders/nmap_vert.hlsl", layout_std_, d3d}
    , nmap_sh_px_{"playground/dx11/normal_map/shaders/nmap_frag.hlsl", sampler_std_, d3d}
  { }

}; // struct Shaders

// Shader manager contains different shaders

struct Buffers
{
  DxUniform<FlatVs_PFCB> flat_vs_pfcb_;
  DxUniform<FlatVs_POCB> flat_vs_pocb_;
  DxUniform<NmapVs_PFCB> nmap_vs_pfcb_;
  DxUniform<NmapVs_POCB> nmap_vs_pocb_;
  DxUniform<NmapPs_PFCB> nmap_ps_pfcb_;  // lights
  DxUniform<NmapPs_POCB> nmap_ps_pocb_;  // material

  Buffers(Renderer& d3d)
    : flat_vs_pfcb_{d3d.GetDevice()}
    , flat_vs_pocb_{d3d.GetDevice()}
    , nmap_vs_pfcb_{d3d.GetDevice()}
    , nmap_vs_pocb_{d3d.GetDevice()}
    , nmap_ps_pfcb_{d3d.GetDevice()}
    , nmap_ps_pocb_{d3d.GetDevice()}
  { }

}; // struct Buffers

// Stores scene objects and makes data driven

struct Scene
{
  struct Lamp { DxObject object_; Vec4f color_; };

  Config cfg_;
  const char* prefix_model_;
  const char* prefix_pos_;
  Vec4f global_ambient_;

  std::map<std::string, DxObject> objects_;
  std::vector<DxObject*> objects_ptr_;
  std::map<std::string, Lamp> lamps_;
  std::vector<DxObject*> lamps_ptr_;

  CameraEul camera_;

  std::string models_path_;
  std::string materials_path_;
  std::string textures_path_;
  std::string images_path_;

  Scene(const std::string& scene_name, Renderer& d3d, MainWindow& win)
    : cfg_{scene_name.c_str()}
    , global_ambient_(0.1f)
    , objects_{}
    , objects_ptr_{}
    , lamps_{}
    , lamps_ptr_{}
    , camera_{75.f, win.GetAspectRatio(), 0.1f, 100.f}
    , models_path_ {cfg_.Get<std::string>("models_factory_path")}
    , materials_path_ {cfg_.Get<std::string>("material_factory_path")}
    , textures_path_ {cfg_.Get<std::string>("texture_factory_path")}
    , images_path_ {cfg_.Get<std::string>("image_factory_path")}
  { }

  void LoadObjects(Renderer& d3d)
  {
    std::vector<std::string> obj_names = cfg_.GetAllKeys<std::string>("model_");
    std::vector<std::string> obj_models = cfg_.GetAllVals<std::string>("model_");
    std::vector<Vec4f> obj_poses = cfg_.GetAllVals<Vec4f>("model_pos_");

    ASSERT(obj_names.size() == obj_models.size());
    ASSERT(obj_names.size() == obj_poses.size());

    for (std::size_t i = 0; i < obj_names.size(); ++i)
    {
      std::string model_path = models_path_ + obj_models[i];
      DxObject object(d3d, model_path, materials_path_, textures_path_);
      object.SetPos(Vec4f(&obj_poses[i][0]).xyz());
      Mat4f tm = matrix::MakeScale(obj_poses[i][3]) % object.GetTm();
      object.SetTm(tm);
      objects_.insert(std::make_pair(obj_names[i], std::move(object)));
    }
  }
  
  void LoadLamps(Renderer& d3d)
  {
    std::vector<std::string> lamp_names = cfg_.GetAllKeys<std::string>("lamp_");
    std::vector<std::string> lamp_models = cfg_.GetAllVals<std::string>("lamp_");
    std::vector<Vec4f> lamp_poses = cfg_.GetAllVals<Vec4f>("lamp_pos_");
    std::vector<Vec4f> lamp_cols = cfg_.GetAllVals<Vec4f>("lamp_col_");

    ASSERT(lamp_names.size() == lamp_models.size());
    ASSERT(lamp_names.size() == lamp_poses.size());
    ASSERT(lamp_names.size() == lamp_cols.size());

    for (std::size_t i = 0; i < lamp_names.size(); ++i)
    {
      std::string model_path = models_path_ + lamp_models[i];
      DxObject object(d3d, model_path, materials_path_, textures_path_);
      object.SetPos(Vec4f(&lamp_poses[i][0]).xyz());
      Mat4f tm = matrix::MakeScale(lamp_poses[i][3]) % object.GetTm();
      object.SetTm(tm);
      lamps_.insert(std::make_pair(lamp_names[i], Lamp{std::move(object), Vec4f(&lamp_cols[i][0])}));
    }
  }

  DxObject* GetObject(const char* fname)
  {
    auto found = objects_.find(fname);
    if (found != objects_.end())
      return &(found->second);
    else
    {
      auto found = lamps_.find(fname);
      if (found != lamps_.end())
        return &(found->second.object_);
    }
    return nullptr;
  }

  void SetupLights(Buffers& cbuffers)
  {
    int i {0};
    DxUniform<NmapPs_PFCB>& cb = cbuffers.nmap_ps_pfcb_;

    for (auto& lamp : lamps_)
    {
      Vec4f& col = lamp.second.color_;
      Vec3f pos = lamp.second.object_.GetPos();
      if (col.w == 0)
      {
        ASSERT(i < cb->lights_.size());
        cb->lights_[i].type_ = LightType::DIR;
        cb->lights_[i].dir_ = Vec4f(pos, 0.f);
        cb->lights_[i].color_ = Vec4f(col, 0.f);
        cb->lights_[i].enabled_ = true;
      }
      else if (col.w == 1)
      {
        ASSERT(i < cb->lights_.size());

        cb->lights_[i].type_ = LightType::POINT;
        cb->lights_[i].pos_ = Vec4f(pos, 1.f);
        cb->lights_[i].color_ = Vec4f(col, 0.f);
        cb->lights_[i].enabled_ = true;
      }
      ++i;
    }
  }

  void UpdateLights(Buffers& cbuffers, Renderer& d3d)
  {
    DxUniform<NmapPs_PFCB>& pscb = cbuffers.nmap_ps_pfcb_;
    DxUniform<NmapVs_PFCB>& vscb = cbuffers.nmap_vs_pfcb_;

    for (std::size_t i = 0; i < pscb->lights_.size(); ++i)
    {
      if (pscb->lights_[i].type_ == LightType::DIR)
        vscb->u_lights_[i].vector_WS_ = pscb->lights_[i].dir_;
      else
        vscb->u_lights_[i].vector_WS_ = pscb->lights_[i].pos_;
      vscb->u_lights_[i].type_ = pscb->lights_[i].type_;
    }
    pscb->camera_pos_ = Vec4f(camera_.GetPos(), 1.f);

    pscb.UploadToPS(1, d3d.GetDeviceCtx());
  }

  void SetupShaders(Renderer& d3d, Buffers& buffers, Shaders& shaders, ShaderType type, bool affine_mapping)
  {
    Mat4f view = camera_.GetViewMx();
    Mat4f proj = camera_.GetProjectionMx();

    switch(type)
    {
      case ShaderType::FLAT :
      {
        buffers.flat_vs_pfcb_->u_view_proj_ = proj * view;
        buffers.flat_vs_pfcb_.UploadToVS(0, d3d.GetDeviceCtx());        
        d3d.UseVxShader(shaders.flat_sh_vx_);
        if (affine_mapping)
          d3d.UsePxShader(shaders.affn_sh_px_);
        else
          d3d.UsePxShader(shaders.flat_sh_px_);
        break;
      }
      case ShaderType::NMAP : default :
      {
        buffers.nmap_vs_pfcb_->u_view_proj_ = proj * view;
        buffers.nmap_vs_pfcb_->u_cam_pos_WS_ = Vec4f(camera_.GetPos(), 1.f);
        buffers.nmap_vs_pfcb_.UploadToVS(0, d3d.GetDeviceCtx());
        d3d.UseVxShader(shaders.nmap_sh_vx_);
        d3d.UsePxShader(shaders.nmap_sh_px_);
        break;
      }
    }
  }

  void PrepareObjects()
  {
    objects_ptr_.clear();
    lamps_ptr_.clear();

    for (auto& map_pair : objects_)
      objects_ptr_.push_back(&map_pair.second);
    for (auto& map_pair : lamps_)
      lamps_ptr_.push_back(&map_pair.second.object_);

    std::sort(objects_ptr_.rbegin(), objects_ptr_.rend(),
    [&](const DxObject* lhs, const DxObject* rhs){
      return vec3::SqLength(lhs->GetPos() - camera_.GetPos()) < vec3::SqLength(rhs->GetPos() - camera_.GetPos());
    });
  }

  void DrawObjects(Renderer& d3d, Buffers& buffers, ShaderType shader_type)
  {
    for (auto* obj : objects_ptr_)
    {
      buffers.flat_vs_pocb_->u_model_ = obj->GetTm();
      buffers.flat_vs_pocb_->u_color_ = Vec4f(0,0,0,1);
      buffers.nmap_vs_pocb_->u_model_ = obj->GetTm();

      for (auto& mesh : obj->GetMeshes())
      {
        d3d.BeforeRenderPass();
        d3d.SetVertexBuffer(mesh.GetVertexBuffer());
        d3d.SetIndexBuffer(mesh.GetIndexBuffer(), mesh.GetIndiciesCount());
        d3d.SetPixelShaderResource(mesh.GetDiffuseMap());
        d3d.SetPixelShaderResource(mesh.GetNormalMap());
        d3d.SetPixelShaderResource(mesh.GetSpecularMap());

        buffers.nmap_ps_pocb_.SetData(mesh.GetMaterialProps());
        buffers.nmap_ps_pocb_.UploadToPS(0, d3d.GetDeviceCtx());

        if (shader_type == ShaderType::FLAT)
          buffers.flat_vs_pocb_.UploadToVS(1, d3d.GetDeviceCtx());
        else
          buffers.nmap_vs_pocb_.UploadToVS(1, d3d.GetDeviceCtx());

        d3d.RenderPass();
        d3d.AfterRenderPass();
      }
    }
  }

  void DrawLamps(Renderer& d3d, Buffers& buffers)
  {
    int i = {0};
    for (auto* obj : lamps_ptr_)
    {
      auto& light = buffers.nmap_ps_pfcb_->lights_[i];
      ++i;
      if (!light.enabled_)
        continue;

      buffers.flat_vs_pocb_->u_model_ = obj->GetTm();
      buffers.flat_vs_pocb_->u_color_ = light.color_;
      // buffers.nmap_vs_pocb_->u_model_ = obj->GetTm();

      for (auto& mesh : obj->GetMeshes())
      {
        d3d.BeforeRenderPass();
        d3d.SetVertexBuffer(mesh.GetVertexBuffer());
        d3d.SetIndexBuffer(mesh.GetIndexBuffer(), mesh.GetIndiciesCount());
        d3d.SetPixelShaderResource(mesh.GetDiffuseMap());
        d3d.SetPixelShaderResource(mesh.GetNormalMap());
        d3d.SetPixelShaderResource(mesh.GetSpecularMap());

        buffers.flat_vs_pocb_.UploadToVS(1, d3d.GetDeviceCtx());

        d3d.RenderPass();
        d3d.AfterRenderPass();
      }
    }
  }

}; // struct Scene

// Per-object scene logic

struct Logic
{
  float rot_angle_ = 0.f;
  float rot_angle_lamp_ = 0.f;
  float rot_speed_ = 4.f;
  float rot_speed_lamp_ = 20.f;
  bool affine_mapping_ = {false};
  bool transparency_ = {false};
  ShaderType shader_type_ = {ShaderType::NMAP};

  void SetupCamera(Scene& scene)
  {
    Vec3f cam_pos (&scene.cfg_.Get<Vec3f>("initial_cam_pos")[0]);
    scene.camera_.SetPos(cam_pos);
    scene.camera_.SetMoveSpeed(2.f);
  }

  void UpdateCamera(Scene& scene, DxInput& input, float dt)
  {
    CameraEul& cam = scene.camera_;

    cam.Rotate(input.GetMouseY(), input.GetMouseX());

    dt += dt * static_cast<int>(input.IsKeyboardBtnHold(DIK_LSHIFT)) * 2;

    if (input.IsKeyboardBtnHold(DIK_W))
      cam.Move(cam.GetTm().GetCol(2), dt);
    if (input.IsKeyboardBtnHold(DIK_S))
      cam.Move(-cam.GetTm().GetCol(2), dt);
    if (input.IsKeyboardBtnHold(DIK_A))
      cam.Move(-cam.GetTm().GetCol(0), dt);
    if (input.IsKeyboardBtnHold(DIK_D))
      cam.Move(cam.GetTm().GetCol(0), dt);
    if (input.IsKeyboardBtnHold(DIK_R))
      cam.Move(cam.GetTm().GetCol(1), dt);
    if (input.IsKeyboardBtnHold(DIK_F))
      cam.Move(-cam.GetTm().GetCol(1), dt);
  }

  void UpdateInput(Scene& scene, Buffers& buffers, DxInput& input, float dt)
  {
    if (input.IsKeyboardBtnPressed(DIK_9))
      shader_type_ = ShaderType::FLAT;
    if (input.IsKeyboardBtnPressed(DIK_8))
      shader_type_ = ShaderType::NMAP;
    if (input.IsKeyboardBtnPressed(DIK_7))
      transparency_ = !transparency_;
    if (input.IsKeyboardBtnPressed(DIK_0))
      affine_mapping_ = !affine_mapping_;
    auto& lb = buffers.nmap_ps_pfcb_;
    if (input.IsKeyboardBtnPressed(DIK_1) && lb->lights_.size() >= 1)
      lb->lights_[0].enabled_ = !lb->lights_[0].enabled_;
    if (input.IsKeyboardBtnPressed(DIK_2) && lb->lights_.size() >= 2)
      lb->lights_[1].enabled_ = !lb->lights_[1].enabled_;
    if (input.IsKeyboardBtnPressed(DIK_3) && lb->lights_.size() >= 3)
      lb->lights_[2].enabled_ = !lb->lights_[2].enabled_;
    if (input.IsKeyboardBtnPressed(DIK_4) && lb->lights_.size() >= 4)
      lb->lights_[3].enabled_ = !lb->lights_[3].enabled_;
  }

  void UpdateObjects(Scene& scene, Buffers& buffers, DxInput& input, float dt)
  {
    DxObject* sub = scene.GetObject("model_smar");
    if (sub)
    {
      scene.global_ambient_ = Vec4f(0.1f);
      Vec3f subm_vel (0.f, 0.f, 2.f);
      float max_cam_dist = 40.f;
      Vec3f initial_cam_pos (&scene.cfg_.Get<Vec3f>("initial_cam_pos")[0]);
      if (vec3::SqLength(scene.camera_.GetPos()) == 0.f)
        scene.camera_.SetPos(initial_cam_pos);
      Vec3f current_cam_pos (scene.camera_.GetPos());
      if (vec3::Length(current_cam_pos - sub->GetPos()) > max_cam_dist)
        current_cam_pos = sub->GetPos() + Vec3f(2.5f, 1.f, 0.f) + subm_vel * 15.f;
      scene.camera_.SetPos(current_cam_pos);
      scene.camera_.LookAt(sub->GetPos() + subm_vel * 2.f);
      sub->SetPos(sub->GetPos() + subm_vel * dt);

      auto& light = buffers.nmap_ps_pfcb_->lights_[3];
      // light.enabled_ = true;
      light.type_ = LightType::POINT;
      light.pos_ = Vec4f(scene.camera_.GetPos(), 1.f);
      Vec3f col (&scene.cfg_.Get<Vec3f>("flashlight_color")[0]);
      light.color_ = Vec4f(col,1);
    }

    DxObject* spaceship = scene.GetObject("model_spaceship");
    if (spaceship)
    {
      scene.global_ambient_ = Vec4f(0.1f);

      rot_angle_ += rot_speed_ * dt;
      Mat4f rot = matrix::MakeRotateY(rot_angle_);
      spaceship->SetTm(rot % matrix::ClearOrient(spaceship->GetTm()));

      auto& light = buffers.nmap_ps_pfcb_->lights_[3];
      // light.enabled_ = true;
      light.type_ = LightType::POINT;
      light.pos_ = Vec4f(scene.camera_.GetPos(), 1.f);
      Vec3f col (&scene.cfg_.Get<Vec3f>("flashlight_color")[0]);
      light.color_ = Vec4f(col,1);
    }

    DxObject* wheel_obj = scene.GetObject("model_wheel");
    DxObject* cube_obj = scene.GetObject("model_stone");
    DxObject* sphere_obj = scene.GetObject("model_sphstone");
    if (wheel_obj && cube_obj && sphere_obj)
    {
      rot_angle_ += rot_speed_ * dt;
      Mat4f rot_wheel = matrix::MakeRotate(rot_angle_, wheel_obj->GetTm().GetCol(2));
      wheel_obj->SetTm(rot_wheel % matrix::ClearOrient(wheel_obj->GetTm()));
      Mat4f rot_general = matrix::MakeRotate(rot_angle_, vec3::Normalize(Vec3f(1.f,1.f,1.f)));
      cube_obj->SetTm(rot_general % matrix::ClearOrient(cube_obj->GetTm()));
      sphere_obj->SetTm(rot_general % matrix::ClearOrient(sphere_obj->GetTm()));
    }
  }

  void UpdateLamps(Scene& scene, Buffers& buffers, DxInput& input, float dt)
  {
    int i = 0;
    for (auto& lamp_pair : scene.lamps_)
    {
      auto& lamp = lamp_pair.second;
      auto& light = buffers.nmap_ps_pfcb_->lights_[i];
      if (!light.enabled_)
        continue;
      if (light.type_ == LightType::DIR)
        lamp.object_.SetPos(Vec3f(0.f) + vec3::Normalize(light.dir_.xyz()) * (scene.camera_.z_far_ - 1.f));
      else
        lamp.object_.SetPos(light.pos_.xyz());
      ++i;
    }

    DxObject* lamp = scene.GetObject("lamp_l3");
    if (lamp)
    {
      int lighting_num = 2;
      Mat4f rot = matrix::MakeRotateX(rot_speed_lamp_ * dt); 
      lamp->SetPos(rot * lamp->GetPos());
      auto& light = buffers.nmap_ps_pfcb_->lights_[lighting_num];
      light.pos_ = Vec4f(lamp->GetPos(), 1.f);
    }
  }

}; // struct Logic

void PrintLog(Scene& scene, FpsCounter& fps, const DxInput& input)
{
  fps.Advance();
  if (fps.Ready())
  {
    LOGF("Fps: %d, mouse x %f, y %f, cam %.02f %.02f %.02f\n", (int)fps.ReadPrev(), input.GetMouseX(), input.GetMouseY(),
          scene.camera_.GetPos().x, scene.camera_.GetPos().y, scene.camera_.GetPos().z);
  }
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR cmdLine, int cmdShow)
{
  // Prepare devices

  MainWindow win (800, 600, "DirectX scene", MainWindow::CENTERED);
  Renderer d3d (win.GetHandle());
  d3d.AlphaBlendingEnabled(false);
  Shaders shaders(d3d);
  Buffers buffers(d3d);
  DxInput input (win.GetHandle(), hInstance);
  input.SetMouseSensitive(0.1f);
  
  ENSUREF(wcslen(cmdLine) != 0, "Point to config file");

  Scene scene (str::Utf2Ansi(cmdLine), d3d, win);
  scene.LoadLamps(d3d);
  scene.LoadObjects(d3d);
  scene.SetupLights(buffers);

  Logic logic {};
  logic.SetupCamera(scene);

  MSG msg {0};
  Timer timer {60};
  FpsCounter fps {};

  do {
    if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    else
    {
      timer.Start();
      float dt = timer.GetLastDt();

      input.Capture();

      if (win.IsResized())
      {
        d3d.ReinitBuffers();
        scene.camera_.SetAspectRatio(win.GetAspectRatio());
      }
      win.ProcessInput(input);

      logic.UpdateCamera(scene, input, dt);
      logic.UpdateInput(scene, buffers, input, dt);
      logic.UpdateObjects(scene, buffers, input, dt);
      logic.UpdateLamps(scene, buffers, input, dt);
      // scene.FrustumCulling();

      d3d.AlphaBlendingEnabled(logic.transparency_);
      d3d.BeginFrame(gfx::RS_STATE | gfx::RS_VIEWPORT | gfx::CLEAR_SHADER_RES | gfx::NULL_VBUFF | gfx::NULL_IBUFF);
      d3d.SwitchOnBackBuffer();
      d3d.PrepareRenderTargets();
    
      scene.UpdateLights(buffers, d3d);
      scene.SetupShaders(d3d, buffers, shaders, ShaderType::FLAT, false);
      scene.DrawLamps(d3d, buffers);
      scene.SetupShaders(d3d, buffers, shaders, logic.shader_type_, logic.affine_mapping_);
      scene.PrepareObjects();
      scene.DrawObjects(d3d, buffers, logic.shader_type_);

      d3d.EndFrame();
      timer.End();
      timer.Wait();

      PrintLog(scene, fps, input);
    }

  } while (msg.message != WM_QUIT);

  d3d.ReportLiveObjects();

  return static_cast<int>(msg.wParam);
}
