// *************************************************************
// File:    cfg_loader.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

// LEGACY, use json loader

#ifndef AH_GDM_CFG_LOADER_H
#define AH_GDM_CFG_LOADER_H

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <array>
#include <map>
#include <memory>
#include <stdexcept>

#include "math/vector3.h"
#include "math/vector4.h"

namespace gdm {

struct Config
{
  Config() = default;
  Config(const char* cfg_fname);
 
  auto GetFname() const -> const std::string&;
  auto GetSname() const -> const std::string&;
  bool IsLoaded() const;
 
public:
  template<class T>
  auto Get(const std::string& name) const -> T;

  template<class T>
  bool Has(const std::string& name) const;
  
  template<class T>
  auto GetAllKeys(const std::string& name) const -> std::vector<std::string>;
  
  template<class T>
  auto GetAllVals(const std::string& name) const -> std::vector<T>;

private:
  bool loaded_ = {};
  std::string fname_ = {};
  std::string sname_ = {};

  std::map<std::string, int> ints_ = {};
  std::map<std::string, float> floats_ = {};
  std::map<std::string, bool> bools_ = {};
  std::map<std::string, std::string> strings_ = {};
  std::map<std::string, std::vector<float>>  vectorsf_ = {};
  std::map<std::string, Vec3f> vectors3f_ = {};
  std::map<std::string, Vec4f> vectors4f_ = {};

}; // struct Config

struct CfgExcept : std::runtime_error
{
  CfgExcept(const char* msg) : std::runtime_error(msg) { }

}; // struct CfgExcept

} // namespace gdm

#include "cfg_loader.inl"

#endif // AH_GDM_CFG_LOADER_H

// TODO: load all data in structs like: type str, elems cnt, words count
//       and store it sequentally, ie: some_type name bla0 bla1 bla2
//
//       cfg.RegisterType("v3", Vec3f);
//       struct Bla { float a; int b; const char* c; }  // std layout
//       cfg.Get<> 
