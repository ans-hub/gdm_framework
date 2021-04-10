// *************************************************************
// File:    main_window.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "main_window.h"

#include <vector>

#include <system/assert_utils.h>

namespace gdm::_private {

thread_local std::vector<WndProcFn> wnd_procs = {};

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  bool input_processed = false;

  for (size_t i = 0; i < _private::wnd_procs.size() && !input_processed; ++i)
    input_processed |= _private::wnd_procs[i](hwnd, message, wParam, lParam);

  if (input_processed)
    return true;

  PAINTSTRUCT paint_struct;
  HDC hDC;
  switch (message)
  {
    case WM_NCCREATE:

      // Here lParam is pointer we set early in CreateWindowEx to our window class instance
      // Also we need to update window pos as it cached, for updating ptr purposes
      // So returning 0 is false for CreateWindowEx proc, so we return 1
    
      SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) ((CREATESTRUCT*)lParam)->lpCreateParams);
      SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
      return 1;
    
    case WM_PAINT:
      hDC = BeginPaint(hwnd, &paint_struct);
      EndPaint(hwnd, &paint_struct);
      break;
    
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    
    case WM_SIZE:
    {
      gdm::MainWindow* wnd = (gdm::MainWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
      int w = ((int)(short)LOWORD(lParam));
      int h = ((int)(short)HIWORD(lParam));
      wnd->SetResized(w, h);
      break;
    }

    default: 
      return DefWindowProc(hwnd, message, wParam, lParam);
  }
  return 0;
}

} // namespace gdm::_private

// --public

gdm::MainWindow::MainWindow(int w, int h, const char* name, unsigned int win_props)
  : width_{w}
  , height_{h}
  , name_{name}
  , h_inst_{0}
  , hndl_{0}
  , full_screen_{false}
  , resized_{false}
{
  hndl_ = InitializeWindow();
  assert(hndl_ && "Couldn't register window class");

  ShowWindow(hndl_, SW_SHOWNORMAL);
  UpdateWindow(hndl_);
  
  bool make_fullscreen = static_cast<bool>(win_props & EWindowProps::FULLSCREEN);
  bool make_centered = static_cast<bool>(win_props & EWindowProps::CENTERED);
  
  if (make_fullscreen)
    ToggleFullscreen();
  else if (make_centered)
    CenterWindow();
}

void gdm::MainWindow::CenterWindow()
{
  RECT rc;
  GetWindowRect(hndl_, &rc);
  int w = rc.right - rc.left;
  int h = rc.bottom - rc.top;
  int x_pos = (GetSystemMetrics(SM_CXSCREEN)/2 - w/2);
  int y_pos = (GetSystemMetrics(SM_CYSCREEN)/2 - h/2);
  SetWindowPos(hndl_, 0, x_pos, y_pos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

void gdm::MainWindow::ToggleFullscreen()
{
  WINDOWPLACEMENT wpc;
  if (!full_screen_)
  {
    GetWindowPlacement(hndl_, &wpc);
    if (hwnd_style == 0)
      hwnd_style = GetWindowLong(hndl_, GWL_STYLE);
    if (hwnd_style_ex == 0)
      hwnd_style_ex = GetWindowLong(hndl_, GWL_EXSTYLE);

    LONG newidth_hwnd_style = hwnd_style;
    newidth_hwnd_style &= ~WS_BORDER;
    newidth_hwnd_style &= ~WS_DLGFRAME;
    newidth_hwnd_style &= ~WS_THICKFRAME;

    LONG newidth_hwnd_style_ex = hwnd_style_ex;
    newidth_hwnd_style_ex &= ~WS_EX_WINDOWEDGE;

    SetWindowLong(hndl_, GWL_STYLE, newidth_hwnd_style | WS_POPUP);
    SetWindowLong(hndl_, GWL_EXSTYLE, newidth_hwnd_style_ex | WS_EX_TOPMOST);
    ShowWindow(hndl_, SW_SHOWMAXIMIZED);
  }
  else
  {
    SetWindowLong(hndl_, GWL_STYLE, hwnd_style);
    SetWindowLong(hndl_, GWL_EXSTYLE, hwnd_style_ex);
    ShowWindow(hndl_, SW_SHOWNORMAL);
    SetWindowPlacement(hndl_, &wpc);
  }
  full_screen_ = !full_screen_;
}

HWND gdm::MainWindow::InitializeWindow()
{
  WNDCLASSEX wc = {0};
  wc.cbSize = sizeof(WNDCLASSEX);
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = &gdm::_private::WndProc;
  wc.hInstance = h_inst_;
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wc.hIcon = LoadIcon(h_inst_, MAKEINTRESOURCE(k_dx_icon_));
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
  wc.lpszMenuName = nullptr;
  wc.lpszClassName = name_;

  bool success = RegisterClassEx(&wc);
  ASSERTF(success, "Couldn't register window class - %u", GetLastError());

  RECT windowRect {0, 0, width_, height_};
  AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, false);

  return CreateWindowExA(0, name_, name_, WS_OVERLAPPEDWINDOW,
                         windowRect.right - windowRect.left,
                         windowRect.bottom - windowRect.top,
                         width_, height_,
                         nullptr,  // parent windows
                         nullptr,  // menus
                         h_inst_,
                         this      // store this pointer to process it later
  );
}

void gdm::MainWindow::RegisterAdditionalWndProc(WndProcFn fn)
{
  _private::wnd_procs.push_back(fn);
}

//--public deprecated

// deprecated because here we should capture input instead of passing dx input and process it

void gdm::MainWindow::ProcessInput(DxInput& input)
{
  if (input.IsKeyboardBtnPressed(DIK_ESCAPE))
    PostMessage(hndl_, WM_DESTROY, 0, 0);
}
