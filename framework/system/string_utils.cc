// *************************************************************
// File:    string_utils.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "string_utils.h"

#include <filesystem>

int gdm::str::Replace(std::string& str, const std::string& sub, const std::string& replace)
{
  int cnt {0};
  while(true)
  {
    auto found = str.find(sub);
    if (found != std::string::npos) {
      str.replace(found, sub.length(), replace);
      ++cnt;
    }
    else
      break;
  }
  return cnt;
}

std::string gdm::str::Replace(const std::string& str, const std::string& sub, const std::string& replace)
{
  std::string res {str};
  while(true)
  {
    auto found = res.find(sub);
    if (found != std::string::npos)
      res.replace(found, sub.length(), replace);
    else
      break;
  }
  return res;
}

std::vector<gdm::str::Range> gdm::str::Split(
  const char* str, char token, bool get_empty /*=false*/)
{
  int begin = 0;
  int end = static_cast<int>(strlen(str));
  return Split(str, begin, end, token, get_empty);
}

std::vector<gdm::str::Range> gdm::str::Split(
  const char* str, int begin, int end, char token, bool get_empty /*=false*/)  // [b;e)
{
  std::vector<Range> result;
  if (begin >= end)
    return result;
  int curr = begin;
  int size = 0;
  while(str[curr] != str[end])
  {
    if (str[curr] == token)
    {
      if (get_empty || size != 0)
        result.push_back({str + begin, str + begin + size});
      begin = curr + 1;
      size = 0;
    }
    else
      ++size;
    ++curr;
  }
  if (get_empty || size != 0)
    result.push_back({str + begin, str + begin + size});
  return result;
}

std::string gdm::str::ToString(const char* str, int begin, int len)
{
  return std::string(str + begin, len);
}

std::string gdm::str::GetFileExtension(const std::string& str)
{
   std::size_t found = str.rfind('.', str.length());
   if (found != std::string::npos)
      return (str.substr(found + 1, str.length() - found));
   return {""};
}

std::string gdm::str::GetFileNameFromFpath(const std::string& fpath)
{
  std::filesystem::path p {fpath};
  return p.filename().string();
}

std::string gdm::str::GetExtensionFromFpath(const char* fpath)
{
  std::string result {};
  std::string file_path {fpath};
  
  std::size_t pos = file_path.find_last_of('.');
  return file_path.substr(pos + 1);

  return result;
}

std::string gdm::str::GetPathFromFullPath(const char* fpath)
{
  std::string full_path_str = fpath; 
  std::size_t last_slash_idx = full_path_str.rfind('/');

  if (std::string::npos != last_slash_idx)
    return full_path_str.substr(0, last_slash_idx) + '/'; 

  return {""};
}

#ifdef _WIN32

std::string gdm::str::Utf2Ansi(LPCWSTR pw, UINT codepage)
{
  std::wstring wstr {pw};
  if (wstr.empty())
    return std::string {};

  int required_size = WideCharToMultiByte(codepage, 0, &wstr[0], static_cast<int>(wstr.size()), nullptr, 0, nullptr, nullptr);
  std::string result (required_size, 0);
  WideCharToMultiByte(codepage, 0, &wstr[0], static_cast<int>(wstr.size()), &result[0], required_size, nullptr, nullptr);
  return result;
}

std::wstring gdm::str::Ansi2Utf(const char* p, UINT codepage)
{
  std::string str {p};
  if (str.empty())
    return std::wstring {};

  int required_size = MultiByteToWideChar(codepage, 0, &str[0], static_cast<int>(str.size()), nullptr, 0);
  std::wstring result (required_size, 0);
  MultiByteToWideChar(codepage, 0, &str[0], static_cast<int>(str.size()), &result[0], required_size);
  return result;
}

#endif
