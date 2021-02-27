// *************************************************************
// File:    cfg_loader.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "cfg_loader.h"

#include "system/string_utils.h"

// --public

gdm::Config::Config(const char* cfg_fname)
{
  fname_ = cfg_fname;
  sname_ = str::GetFileNameFromFpath(fname_);
  str::Rtrim(sname_);

  std::ifstream fs {cfg_fname};

  if (!fs)
    return;

  std::string   line  {""};
  std::string   type  {""};
  std::string   name  {""};
  float         val_f {};
  std::string   val_str {};

  while(std::getline(fs, line))
  {
    if (line == "")
      continue;

    std::istringstream stream {line};
    stream >> type >> name;
    
    if (type == "f")
      stream >> floats_[name];
    else if (type == "i")
      stream >> ints_[name];
    else if (type == "b")
      stream >> bools_[name];
    else if (type == "s")
      stream >> strings_[name];
    else if (type == "v3")
    {
      Vec3f v {};
      while (stream >> v.x >> v.y >> v.z) { }
      vectors3f_[name] = v;
    }
    else if (type == "v4")
    {
      Vec4f v {};
      while (stream >> v.x >> v.y >> v.z >> v.w) { }
      vectors4f_[name] = v;
    }
    else if (type == "af")
    {
      std::vector<float> v {};
      while (stream >> val_f)
        v.push_back(val_f);
      vectorsf_[name] = v;
    }
    line = {""};
  }   
  loaded_ = true;
}

const std::string& gdm::Config::GetFname() const
{
  return fname_;
}

const std::string& gdm::Config::GetSname() const
{
  return sname_;
}

bool gdm::Config::IsLoaded() const
{
  return loaded_;
}
