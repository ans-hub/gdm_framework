// *************************************************************
// File:    input.ecs.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_ECS_COMP_INPUT_H
#define AH_ECS_COMP_INPUT_H

#include <ecs/component.h>

#include <window/win/dx_input.h>

using namespace gdm;

struct Input : ecs::SingletonComponent<ECS_COMPONENT_IDX>
{
  Input(HWND window_hndl) : input(window_hndl) { }
  DxInput* operator->() { return &input; }
  DxInput input;
};

#endif // AH_ECS_COMP_INPUT_H
