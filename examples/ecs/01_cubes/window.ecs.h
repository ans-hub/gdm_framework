// *************************************************************
// File:    window.ecs.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_ECS_COMP_WINDOW_H
#define AH_ECS_COMP_WINDOW_H

#include <ecs/component.h>

#include <window/win/main_window.h>

using namespace gdm;

struct Window : ecs::SingletonComponent<ECS_COMPONENT_IDX>
{
  Window(int w, int h, const char* name, MainWindow::EWindowProps props)
    : window(w, h, name, props)
  { }
  MainWindow* operator->() { return &window; }
  MainWindow window;
};

#endif // AH_ECS_COMP_WINDOW_H