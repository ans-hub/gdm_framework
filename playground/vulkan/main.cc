// *************************************************************
// File:    main.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GFX_VK_API
#define GFX_VK_API
#endif

#include "render/defines.h"
#include "render/api.h"
#include "render/renderer.h"
#include "render/colors.h"
#include "render/shader.h"
#include "render/camera_eul.h"

#include "memory/defines.h"

#include "window/main_window.h"
#include "window/main_input.h"

#include "data/data_storage.h"
#include "data/model_factory.h"

#include "system/fps_counter.h"
#include "system/timer.h"
#include "system/hash_utils.h"
#include "system/array_utils.h"
#include "system/diff_utils.h"
#include "system/literals.h"
#include "system/profiler.h"
#include "system/font.h"

#include "render/desc/rasterizer_desc.h"
#include "render/desc/input_layout_desc.h"
#include "render/desc/sampler_desc.h"

#include <scene/scene_renderer.h>
#include <scene/gpu_streamer.h>
#include <scene/cfg_dispatcher.h>
#include <scene/data_helpers.h>
#include <scene/debug_draw.h>

using namespace gdm;

void DrawInfo(SceneRenderer& scene_renderer, const Timer& timer, const FpsCounter& fps, const std::string& cfg_name)
{
  DebugDraw& dd = scene_renderer.GetDebugDraw();
  const float line_height = (float)dd.GetFont()->GetMetrics().font_height_;
  std::string msg;
  msg.append("FPS: ");
  msg.append(std::to_string(fps.ReadPrev()));
  std::string gpu_name = scene_renderer.GetGpuInfo().device_props_.deviceName;
  scene_renderer.GetDebugDraw().DrawString({10.f, 0.f, 0.f}, msg, color::White);
  scene_renderer.GetDebugDraw().DrawString({10.f, line_height, 0.f}, cfg_name, color::LightGreen);
  scene_renderer.GetDebugDraw().DrawString({10.f, line_height * 2, 0.f}, gpu_name, color::LightGreen);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR cmdLine, int cmdShow)
{
  uint width = 800;
  uint height = 600;

  auto win = MainWindow(width, height, "Vulkan scenes", MainWindow::CENTERED);
  auto input = MainInput(win.GetHandle(), hInstance);
  auto api_renderer = api::Renderer(win.GetHandle(), gfx::DEBUG_DEVICE | gfx::PROFILE_MARKS);
  auto gpu_streamer = GpuStreamer(api_renderer);
  auto scene_renderer = SceneRenderer(api_renderer, gpu_streamer);

  ENSUREF(wcslen(cmdLine) != 0, "Config file name is empty");

  std::string cfg_name = str::Utf2Ansi(cmdLine);
  Config cfg(cfg_name.c_str());

  if (!cfg.IsLoaded())
    return 1;

  auto scene = Scene(cfg, win);
  auto unique_models = helpers::GetUniqueModels(scene.GetSceneInstances());  
  
  gpu_streamer.CopyModelsToGpu(unique_models);

  MSG msg {0};
  Timer timer {};
  FpsCounter fps {};

  GDM_PROFILING_ENABLE();

  int exit = -1;

  while(exit == -1)
  {
    while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
      exit = (msg.message == WM_QUIT) ? static_cast<int>(msg.wParam) : -1;
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    input.Capture();
    win.ProcessInput(input);
    timer.Start();
    float dt = timer.GetLastDt();

    scene.Update(dt,
      input,
      scene_renderer.GetDebugDraw());
    
    scene_renderer.Render(dt,
      scene.GetCamera(),
      scene.GetRenderableInstances(),
      scene.GetRenderableMaterials(),
      scene.GetLamps(),
      scene.GetFlashlights());

    DrawInfo(scene_renderer, timer, fps, cfg_name);

    timer.End();
    timer.Wait();
    fps.Advance();
  }

  return static_cast<int>(msg.wParam);
}

