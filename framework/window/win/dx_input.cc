// *************************************************************
// File:    dx_input.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include <cassert>

#include "dx_input.h"

gdm::DxInput::DxInput(HWND window_hndl, HINSTANCE window_hinst)
  : direct_input_{}
  , di_kbd_{}
  , di_mouse_{}
  , kbd_state_{}
  , mouse_state_{}
  , kbd_prev_state_{}
  , pos_x_{0.f}
  , pos_y_{0.f}
  , mouse_sensitive_{1.f}
{
  assert(window_hndl);
  assert(window_hinst);

  HRESULT hr;
  hr = DirectInput8Create(window_hinst, DIRECTINPUT_VERSION, IID_IDirectInput8,
       (void**)&direct_input_, nullptr); 
  assert(hr == S_OK && "Couldn't create direct input");

  hr = direct_input_->CreateDevice(GUID_SysKeyboard, &di_kbd_, nullptr);
  assert(hr == S_OK && "Couldn't create kbd device");

  hr = direct_input_->CreateDevice(GUID_SysMouse, &di_mouse_, nullptr);
  assert(hr == S_OK && "Couldn't create mouse device");

  hr = di_kbd_->SetDataFormat(&c_dfDIKeyboard);
  hr += di_kbd_->SetCooperativeLevel(window_hndl, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
  assert(hr == S_OK && "Couldn't set parameters to kbd device");

  hr = di_mouse_->SetDataFormat(&c_dfDIMouse);
  hr += di_mouse_->SetCooperativeLevel(window_hndl, DISCL_EXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND);
  assert(hr == S_OK && "Couldn't set parameters to mouse device");

  ZeroMemory(&mouse_state_, sizeof(DIMOUSESTATE));
}

gdm::DxInput::~DxInput()
{
  di_kbd_->Unacquire();
  di_mouse_->Unacquire();
  if (direct_input_)
    direct_input_->Release();
  direct_input_ = nullptr;
}

void gdm::DxInput::CaptureKeyboard()
{
  di_kbd_->Acquire();
  di_kbd_->GetDeviceState(sizeof(kbd_state_), static_cast<LPVOID>(&kbd_state_));

  for (std::size_t i = 0; i < 256; ++i) // todo: by memcpy
    kbd_prev_state_[i] = (bool)kbd_prev_state_[i] && (bool)(kbd_state_[i] & 0x80);
}

void gdm::DxInput::CaptureMouse()
{
  di_mouse_->Acquire();
  di_mouse_->GetDeviceState(sizeof(DIMOUSESTATE), &mouse_state_);

  pos_x_ += static_cast<float>(mouse_state_.lX) * mouse_sensitive_;
  pos_y_ += static_cast<float>(mouse_state_.lY) * mouse_sensitive_;
}

void gdm::DxInput::PauseCaptureMouse()
{
  di_mouse_->Unacquire();
}

bool gdm::DxInput::IsKeyboardBtnPressed(BYTE btn) const
{ 
  if ((kbd_state_[btn] & 0x80) && !kbd_prev_state_.test(btn))
  {
    kbd_prev_state_.set(btn);
    return true;
  }
  return false;
}

//--public deprecated

void gdm::DxInput::Capture()
{
  di_kbd_->Acquire();
  di_kbd_->GetDeviceState(sizeof(kbd_state_), static_cast<LPVOID>(&kbd_state_));

  di_mouse_->Acquire();
  di_mouse_->GetDeviceState(sizeof(DIMOUSESTATE), &mouse_state_);

  for (std::size_t i = 0; i < 256; ++i) // todo: by memcpy
    kbd_prev_state_[i] = (bool)kbd_prev_state_[i] && (bool)(kbd_state_[i] & 0x80);
  
  pos_x_ += static_cast<float>(mouse_state_.lX) * mouse_sensitive_;
  pos_y_ += static_cast<float>(mouse_state_.lY) * mouse_sensitive_;
}