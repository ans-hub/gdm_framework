// *************************************************************
// File:    data_factory.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "data_factory.h"

#include <string>

#include <system/hash_utils.h>
#include <system/assert_utils.h>
#include <system/array_utils.h>
#include <system/mesh_utils.h>
#include <system/string_utils.h>

#include "helpers.h"

// --public

gdm::Handle gdm::helpers::GenerateHandle(const char* fpath)
{
  std::string fname = str::GetFileNameFromFpath(fpath);
  uint64_t hash = GDM_HASH(fname.c_str());
  return hash;
}

std::string gdm::helpers::GenerateName(const char* fpath, int num)
{
  std::string fname = str::GetFileNameFromFpath(fpath);
  fname += std::to_string(num);
  return fname;
}
