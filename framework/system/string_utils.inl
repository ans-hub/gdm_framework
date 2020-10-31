// *************************************************************
// File:    string_utils.inl
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "string_utils.h"

#include <tchar.h>

#ifdef _WIN32
#include <windows.h>
#endif

inline std::wstring gdm::str::ToWideStringF(const char* p)
{
  std::string str {p};

  if (str.empty())
    return std::wstring {};

  int required_size = ::MultiByteToWideChar(CP_UTF8, 0, &str[0], static_cast<int>(str.size()), nullptr, 0);
  std::wstring result (required_size, 0);
  ::MultiByteToWideChar(CP_UTF8, 0, &str[0], static_cast<int>(str.size()), &result[0], required_size);
 
  return result;
}
