// *************************************************************
// File:    string_utils.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_STRINGS_H
#define AH_GDM_STRINGS_H

#include <string>
#include <vector>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#endif

namespace gdm::str {

  struct Range { const char* begin; const char* end; };

  auto Replace(std::string& str, const std::string& sub, const std::string& replace) -> int;
  auto Replace(const std::string& str, const std::string& sub, const std::string& replace) -> std::string;
  auto Split(const char* str, char token, bool get_empty = false) -> std::vector<Range>;
  auto Split(const char* str, int begin, int end, char token, bool get_empty = false) -> std::vector<Range>;
  auto ToString(const char* str, int begin, int len) -> std::string;
  auto GetFileExtension(const std::string& str) -> std::string;
  auto GetFileNameFromFpath(const std::string& str) -> std::string;
  auto GetExtensionFromFpath(const char* fpath) -> std::string;
  auto GetPathFromFullPath(const char* fpath) -> std::string;

#ifdef _WIN32
  std::string Utf2Ansi(LPCWSTR pw, UINT codepage = CP_UTF8);
  std::wstring Ansi2Utf(const char* x, UINT codepage = CP_UTF8);
  std::wstring ToWideStringF(const char* p);
#endif

  void Ltrim(std::string &s);
  void Rtrim(std::string &s);
  void Trim(std::string &s);
  std::string LtrimCopy(std::string s);
  std::string RtrimCopy(std::string s);
  std::string TrimCopy(std::string s);

} // namespace gdm::str

#if defined (_WIN32)
  #define CSTR(s) gdm::str::ToWideStringF(s).c_str()
#else
  #define CSTR(s)
#endif

#include "string_utils.inl"

#endif  // AH_GDM_STRINGS_H