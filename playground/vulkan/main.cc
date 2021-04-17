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
#include "engine/gpu_streamer.h"
#include "engine/debug_draw.h"

#include "engine/gui_manager.h"
#include "engine/cfg_dispatcher.h"
#include "engine/gui_manager.h"
#include "engine/camera_eul.h"

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

#include "app_gui.h"
#include "app_scene.h"
#include "app_renderer.h"
#include "app_input.h"
#include "app_helpers.h"

using namespace gdm;

int ProcessMessages(MSG& msg, const MainInput& input, MainWindow& win)
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

void DrawInfo(DebugDraw& debug_draw, AppRenderer& renderer, const Timer& timer, const FpsCounter& fps, const std::string& cfg_name)
{
  const float line_height = (float)debug_draw.GetFont()->GetMetrics().font_height_;
  std::string msg;
  msg.append("FPS: ");
  msg.append(std::to_string(fps.ReadPrev()));
  std::string gpu_name;
  gpu_name.append("GPU: ");
  gpu_name.append(renderer.GetGpuInfo().device_props_.deviceName);
  std::string help {"F7-F8 (gui), F9 (wire), F11 (text)"};
  debug_draw.DrawString({10.f, 0.f, 0.f}, msg, color::White);
  debug_draw.DrawString({10.f, line_height, 0.f}, cfg_name, color::LightGreen);
  debug_draw.DrawString({10.f, line_height * 2, 0.f}, gpu_name, color::LightGreen);
  debug_draw.DrawString({10.f, line_height * 3, 0.f}, help, color::LightGreen);
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
  win.RegisterAdditionalWndProc(gui::WinProc);

  auto api_renderer = api::Renderer(win.GetHandle(), gfx::DEBUG_DEVICE | gfx::PROFILE_MARKS);
  auto gpu_streamer = GpuStreamer(api_renderer);

  auto gui = GuiManager();
  int gui_cb_example = gui.RegisterGuiCallback(gui::GuiExampleCb);
  int gui_cb_docking = gui.RegisterGuiCallback(gui::GuiDockingCb);

  auto input = PlaygroundInput(win.GetHandle(), hInstance);
  input.SetGuiButton(gui_cb_example, DIK_F7);
  input.SetGuiButton(gui_cb_docking, DIK_F8);
  input.SetWireButton(DIK_F9);
  input.SetTextButton(DIK_F11);
  // input.SetCameraMoveButton() and others

  auto debug_draw = DebugDraw{};
  debug_draw.AddFont(gpu_streamer, "assets/fonts/arial.ttf", 14);
  debug_draw.ToggleActivateWire();
  debug_draw.ToggleActivateText();

  auto renderer = AppRenderer(api_renderer, gpu_streamer, debug_draw);
  auto scene = PlaygroundScene(cfg, win);
  auto unique_models = app_helpers::GetUniqueModels(scene.GetSceneInstances());
  gpu_streamer.CopyModelsToGpu(unique_models);

  MSG msg {0};
  Timer timer {};
  FpsCounter fps {};
  int exit = -1;

  GDM_PROFILING_ENABLE();

  while(exit == -1)
  {
    exit = ProcessMessages(msg, input.GetRawInput(), win);

    timer.Start();
    float dt = timer.GetLastDt();

    // todo: notify renderer
    // if (win.IsResized())
    //   api_renderer.RequestSwapChainRecreate();

    input.Update();
    debug_draw.Update();

    scene.Update(
      dt,
      input,
      gui,
      debug_draw);

    DrawInfo(debug_draw, renderer, timer, fps, cfg_name);
    
    renderer.Render(
      dt,
      debug_draw,
      scene.GetCamera(),
      scene.GetRenderableInstances(),
      scene.GetRenderableMaterials(),
      scene.GetLamps(),
      scene.GetFlashlights(),
      gui.GetGuiCallbacks());

    timer.End();
    timer.Wait();
    fps.Advance();
  }

  return static_cast<int>(msg.wParam);
}

