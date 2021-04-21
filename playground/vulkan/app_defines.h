// *************************************************************
// File:    defines.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GFX_APP_DEFINES_H
#define GFX_APP_DEFINES_H

#include <factory/model_factory.h>
#include <engine/camera_eul.h>
#include <render/defines.h>
#include <window/main_input.h>
#include <engine/debug_draw.h>

namespace gdm::cfg {

  constexpr static float v_znear = 0.1f;
  constexpr static float v_zfar = 100.f;
  constexpr static float v_fov = 75.f;
  constexpr static uint v_max_objects = 512;
  constexpr static uint v_material_type_cnt = 3; // diff_map + norm_map + v_spec_map
  constexpr static uint v_max_materials = 32;
  constexpr static uint v_diff_offset = 0;
  constexpr static uint v_norm_offset = 1;
  constexpr static uint v_spec_offset = 2;

} // namespace gdm::cfg

#endif // GFX_APP_DEFINES_H