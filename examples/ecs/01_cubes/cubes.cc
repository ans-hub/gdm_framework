// *************************************************************
// File:    cubes.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include <tuple>

#include <system/assert_utils.h>
#include <system/timer.h>
#include <system/fps_counter.h>
#include <system/rand_utils.h>

#include <ecs/core.h>
#include <ecs/component.h>
#include <ecs/manager.h>

#include "window.ecs.h"
#include "render.ecs.h"
#include "input.ecs.h"
#include "camera.ecs.h"
#include "stat.ecs.h"
#include "player.ecs.h"
#include "enemy.ecs.h"
#include "physics.ecs.h"
#include "gun.ecs.h"

ecs::EntityManager& mgr = ecs::EntityManager::GetInstance();

ECS_COMPONENT_REGISTER(Transform)
ECS_COMPONENT_REGISTER(Renderable)
ECS_COMPONENT_REGISTER(Target)
ECS_COMPONENT_REGISTER(Ai)
ECS_COMPONENT_REGISTER(Physics)
ECS_COMPONENT_REGISTER(Gun)

ECS_COMPONENT_SINGLETON_REGISTER(Window, 800, 600, "Cubes", MainWindow::CENTERED)
ECS_COMPONENT_SINGLETON_REGISTER(Camera, 75.f, mgr.GetComponent<Window>()->GetAspectRatio(), 0.1f, 100.f)
ECS_COMPONENT_SINGLETON_REGISTER(Input, mgr.GetComponent<Window>()->GetHandle());
ECS_COMPONENT_SINGLETON_REGISTER(Render)
ECS_COMPONENT_SINGLETON_REGISTER(Statistic)

ECS_SYSTEM_REGISTER(player_spawn, Statistic)
ECS_SYSTEM_REGISTER(enemy_spawn, Statistic)
ECS_SYSTEM_REGISTER(player_move, ecs::Dt, Camera, Input)
ECS_SYSTEM_REGISTER(enemy_set_velocity, Transform, Physics, Ai)
ECS_SYSTEM_REGISTER(enemy_move, ecs::Dt, Transform, Physics, Ai)
ECS_SYSTEM_REGISTER(enemy_attack, Transform, Physics, Gun, Ai)
ECS_SYSTEM_REGISTER(enemy_search_target, Transform, Camera, Ai)
ECS_SYSTEM_REGISTER(enemy_player_collision, ecs::Eid, Transform, Camera, Ai)
ECS_SYSTEM_REGISTER(update_gun, ecs::Dt, Gun)

ECS_SYSTEM_REGISTER(render_begin_frame, Render)
ECS_SYSTEM_REGISTER(render_pass_main_prepare, Render)
ECS_SYSTEM_REGISTER(render_pass_main_set_camera, Render, Camera)
ECS_SYSTEM_REGISTER(render_pass_main_prepare_objects, Renderable, Transform)
ECS_SYSTEM_REGISTER(render_pass_main_sort_objects, Render, Camera)
ECS_SYSTEM_REGISTER(render_pass_main_draw_objects, Render)
ECS_SYSTEM_REGISTER(render_end_frame, Render)

//  Todo:
//   - ECS_EVENT_REGISTER(ENEMY_COLLIDED, ecs::Eid, Transform)
//   - sendEvent(ENEMY_COLLIDED)
//   - ECS_EVENT_REGISTER(ON_ENTITY_DESTROYED, event_func, Comp1, Comp2)
//     -- entity_sig = C1:sig | C1:sig;
//     -- events[entity_sig][ON_ENTITY_DESTROYED]=event_func;
//   - void event_func(Comp1, Comp2){}
//   - when destroy in manager: sendEvent(ON_ENTITY_DESTROYED, entity_sig)

using namespace gdm;

int WINAPI wWinMain(HINSTANCE /* hInstance */, HINSTANCE /* prevInstance */, LPWSTR /* cmdLine */, int /* cmdShow */)
{  
  rand::StartRand();

  mgr.CreateEntity<Window*, Render*, Camera*, Statistic*>();

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
      
      ecs::EntityManager::GetInstance().Tick(dt);

      timer.End();
      timer.Wait();
      helpers::PrintFps(fps);
    }
  } while (msg.message != WM_QUIT);

  return static_cast<int>(msg.wParam);
}
