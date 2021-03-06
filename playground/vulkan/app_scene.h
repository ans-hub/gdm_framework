// *************************************************************
// File:    app_scene.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include <string>

#include "render/defines.h"
#include "render/api.h"
#include "render/renderer.h"

#include "window/main_input.h"
#include "window/main_window.h"

#include "factory/model_factory.h"
#include "data/cfg_loader.h"

#include "engine/cfg_dispatcher.h"
#include "engine/gui_manager.h"

#include "app_input.h"

namespace gdm {

struct AppScene
{
  AppScene(Config& cfg, MainWindow& win);
  void Update(float dt, AppInput& input_mgr, GuiManager& gui, DebugDraw& debug_draw);

public:
  void SetObjects(const std::vector<ModelInstance>& objs, const std::vector<std::string>& names);
  void SetLamps(const std::vector<ModelInstance>& lamps, const std::vector<ModelInstance>& flashlights);

  auto GetSceneInstances() -> std::vector<ModelInstance*>;
  auto GetSceneInstancesNames() const -> const std::vector<std::string>&;
  auto GetRenderableInstances() -> std::vector<ModelInstance*>;
  auto GetRenderableMaterials() -> const api::ImageViews&;
  auto GetLamps() -> std::vector<ModelLight>& { return lamps_; }
  auto GetFlashlights() -> std::vector<ModelLight>& { return flashlights_; }
  auto GetCamera() const -> const CameraEul& { return camera_; }

private:
  CameraEul camera_;
  std::vector<ModelInstance> models_;
  std::vector<std::string> models_names_;
  std::vector<ModelLight> flashlights_;
  std::vector<ModelLight> lamps_;
  api::ImageViews renderable_materials_;
  api::ImageView* dummy_view_;
  cfg::Dispatcher cfg_dispatcher_;

}; // struct AppScene

} // namespace gdm
