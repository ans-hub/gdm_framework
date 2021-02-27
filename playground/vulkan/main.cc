// *************************************************************
// File:    deferred.cc
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
#include "render/debug_draw.h"

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

#include "render/desc/rasterizer_desc.h"
#include "render/desc/input_layout_desc.h"
#include "render/desc/sampler_desc.h"

#include <scene/deferred_renderer.h>
#include <scene/cfg_dispatcher.h>
#include <scene/helpers.h>

using namespace gdm;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR cmdLine, int cmdShow)
{
  uint width = 800;
  uint height = 600;
  MainWindow win (width, height, "Vulkan scenes", MainWindow::CENTERED);
  MainInput input (win.GetHandle(), hInstance);

  ENSUREF(wcslen(cmdLine) != 0, "Config file name is empty");

  std::string cfg_name = str::Utf2Ansi(cmdLine);

  Config cfg(cfg_name.c_str());

  if (!cfg.IsLoaded())
    return 1;

  Scene scene(cfg, win);
  DeferredRenderer renderer(win.GetHandle(), gfx::DEBUG_DEVICE | gfx::PROFILE_MARKS);  
  renderer.Setup(scene);

  MSG msg {0};
  Timer timer {60};
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
    
    scene.Update(input, renderer.GetDebugDraw(), dt);
    renderer.Update(scene);

    timer.End();
    timer.Wait();  
  }

  return static_cast<int>(msg.wParam);
}