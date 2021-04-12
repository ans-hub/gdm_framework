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

#include <scene/playground_scene.h>
#include <scene/playground_renderer.h>
#include <scene/gpu_streamer.h>
#include <scene/cfg_dispatcher.h>
#include <scene/data_helpers.h>
#include <scene/debug_draw.h>
#include <scene/gui_draw.h>

using namespace gdm;

int ProcessMessages(MSG& msg, MainInput& input, MainWindow& win)
{
  int exit = -1;
  
  while(PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
  {
    exit = (msg.message == WM_QUIT) ? static_cast<int>(msg.wParam) : -1;
    ::TranslateMessage(&msg);
    ::DispatchMessage(&msg);
  }

  if (input.IsKeyboardBtnPressed(DIK_ESCAPE))
    ::PostMessage(win.GetHandle(), WM_DESTROY, 0, 0);
  
  return exit;
}

void DrawInfo(PlaygroundRenderer& renderer, const Timer& timer, const FpsCounter& fps, const std::string& cfg_name)
{
  DebugDraw& dd = renderer.GetDebugDraw();
  const float line_height = (float)dd.GetFont()->GetMetrics().font_height_;
  std::string msg;
  msg.append("FPS: ");
  msg.append(std::to_string(fps.ReadPrev()));
  std::string gpu_name;
  gpu_name.append("GPU: ");
  gpu_name.append(renderer.GetGpuInfo().device_props_.deviceName);
  std::string help {"F9 (text), F10 (GUI), F11 (debug)"};
  renderer.GetDebugDraw().DrawString({10.f, 0.f, 0.f}, msg, color::White);
  renderer.GetDebugDraw().DrawString({10.f, line_height, 0.f}, cfg_name, color::LightGreen);
  renderer.GetDebugDraw().DrawString({10.f, line_height * 2, 0.f}, gpu_name, color::LightGreen);
  renderer.GetDebugDraw().DrawString({10.f, line_height * 3, 0.f}, help, color::LightGreen);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR cmdLine, int cmdShow)
{
  ENSUREF(wcslen(cmdLine) != 0, "Config file name is empty");

  std::string cfg_name = str::Utf2Ansi(cmdLine);
  Config cfg(cfg_name.c_str());

  if (!cfg.IsLoaded())
    return 1;

  const uint width = 800;
  const uint height = 600;

  auto win = MainWindow(width, height, "Vulkan scenes", MainWindow::CENTERED);
  auto input = MainInput(win.GetHandle(), hInstance);
  auto api_renderer = api::Renderer(win.GetHandle(), gfx::DEBUG_DEVICE | gfx::PROFILE_MARKS);
  auto gpu_streamer = GpuStreamer(api_renderer);
  auto renderer = PlaygroundRenderer(api_renderer, gpu_streamer, win);
  
  auto scene = PlaygroundScene(cfg, win);
  auto unique_models = data_helpers::GetUniqueModels(scene.GetSceneInstances());
  gpu_streamer.CopyModelsToGpu(unique_models);

  MSG msg {0};
  Timer timer {};
  FpsCounter fps {};
  int exit = -1;

  GDM_PROFILING_ENABLE();

  while(exit == -1)
  {
    exit = ProcessMessages(msg, input, win);

    timer.Start();
    float dt = timer.GetLastDt();

    scene.Update(dt,
      input,
      renderer.GetGuiDraw(),
      renderer.GetDebugDraw()
    );

    renderer.Render(dt,
      scene.GetCamera(),
      scene.GetRenderableInstances(),
      scene.GetRenderableMaterials(),
      scene.GetLamps(),
      scene.GetFlashlights()
    );

    DrawInfo(renderer, timer, fps, cfg_name);

    timer.End();
    timer.Wait();
    fps.Advance();
  }

  return static_cast<int>(msg.wParam);
}

