// *************************************************************
// File:    data_factory.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_DATA_FACT_H
#define AH_GDM_DATA_FACT_H

#include <utility>
#include <vector>
#include <unordered_map>

#include "memory/defines.h"

namespace gdm {

using Handle = uint64_t;

template <class T>
struct DataFactory
{
  static auto GetHandle(const char* fpath) -> Handle;
  static auto Get(Handle handle) -> T&;
  static auto Get(const std::vector<Handle>& handles) -> std::vector<T*>;
  static bool Has(Handle handle);
  static bool Has(const char* fpath);
  static void SetPath(const char* path) { resources_path_ = path; }
  static auto GetPath() -> const char* { return resources_path_; }

protected:
  inline static const char* resources_path_ {""};
  inline static std::unordered_map<Handle, T> resources_ {};
};

namespace helpers {

  auto GenerateHandle(const char* fpath) -> Handle;
  auto GenerateName(const char* fpath, int num) -> std::string;

} // namespace helpers

} // namespace gdm

#include "data_factory.inl"

#endif // AH_GDM_DATA_FACT_H
