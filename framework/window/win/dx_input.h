// *************************************************************
// File:    dx_input.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

// https://gamedev.stackexchange.com/questions/55955/why-directinput-is-not-recommended
// https://docs.microsoft.com/en-us/windows/win32/inputdev/raw-input?redirectedfrom=MSDN

#ifndef AH_GDM_DX_INPUT_H
#define AH_GDM_DX_INPUT_H

#include <windows.h>
#include <dinput.h>
#include <bitset>

namespace gdm {

struct DxInput
{
  DxInput(HWND window_hndl, HINSTANCE window_hinst = GetModuleHandle(NULL));
  ~DxInput();

  void Capture();
  void SetMouseSensitive(float val) { mouse_sensitive_ = val; }
  bool IsKeyboardBtnPressed(BYTE btn) const;
  bool IsKeyboardBtnHold(BYTE btn) const { return kbd_state_[btn] & 0x80; }
  float GetMouseX() const { return pos_x_; }
  float GetMouseY() const { return pos_y_; }

private:
  LPDIRECTINPUT8 direct_input_;
  IDirectInputDevice8* di_kbd_;
  IDirectInputDevice8* di_mouse_ ;

  BYTE kbd_state_[256];
  DIMOUSESTATE mouse_state_;
  mutable std::bitset<256> kbd_prev_state_;

  float pos_x_;
  float pos_y_;
  float mouse_sensitive_;

}; // struct DxInput

} // namespace gdm

#endif // AH_GDM_DX_INPUT_H