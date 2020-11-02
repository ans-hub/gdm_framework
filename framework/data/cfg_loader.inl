// *************************************************************
// File:    cfg_loader.inl
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "cfg_loader.h"

// --public
 
template<>
inline int gdm::Config::Get<int>(const std::string& name) const
{ 
  return ints_.at(name);
}

template<>
inline float gdm::Config::Get<float>(const std::string& name) const
{
  return floats_.at(name);
}

template<>
inline bool gdm::Config::Get<bool>(const std::string& name) const
{
  return bools_.at(name);
}

template<>
inline const std::string& gdm::Config::Get<const std::string&>(const std::string& name) const
{
  static std::string s_dummy {""};
  return strings_.count(name) ? strings_.at(name) : s_dummy;
}

template<>
inline std::string gdm::Config::Get<std::string>(const std::string& name) const
{
  return strings_.count(name) ? strings_.at(name) : "";
}

template<>
inline gdm::Vec3f gdm::Config::Get<gdm::Vec3f>(const std::string& name) const
{
  return vectors3f_.at(name);
}

template<>
inline gdm::Vec4f gdm::Config::Get<gdm::Vec4f>(const std::string& name) const
{
  return vectors4f_.at(name);
}

template<>
inline std::vector<float> gdm::Config::Get<std::vector<float>>(const std::string& name) const
{
  return vectorsf_.at(name);
}

template<>
inline std::vector<std::string> gdm::Config::GetAllVals<std::string>(const std::string& name) const
{
  std::vector<std::string> result {};
  std::for_each(strings_.begin(), strings_.end(),
  [&name, &result](const std::pair<std::string, std::string>& pair)
  {
    if (pair.first.find(name) == 0)
      result.push_back(pair.second);
  });
  return result;
}

template<>
inline std::vector<float> gdm::Config::GetAllVals<float>(const std::string& name) const
{
  std::vector<float> result {};
  std::for_each(floats_.begin(), floats_.end(),
  [&name, &result](const std::pair<std::string, float>& pair)
  {
    if (pair.first.find(name) == 0)
      result.push_back(pair.second);
  });
  return result;
}

template<>
inline std::vector<gdm::Vec3f> gdm::Config::GetAllVals<gdm::Vec3f>(const std::string& name) const
{
  std::vector<Vec3f> result {};
  std::for_each(vectors3f_.begin(), vectors3f_.end(),
  [&name, &result](const std::pair<std::string, Vec3f>& pair)
  {
    if (pair.first.find(name) == 0)
      result.push_back(pair.second);
  });
  return result;
}

template<>
inline std::vector<gdm::Vec4f> gdm::Config::GetAllVals<gdm::Vec4f>(const std::string& name) const
{
  std::vector<Vec4f> result {};
  std::for_each(vectors4f_.begin(), vectors4f_.end(),
  [&name, &result](const std::pair<std::string, Vec4f>& pair)
  {
    if (pair.first.find(name) == 0)
      result.push_back(pair.second);
  });
  return result;
}

template<>
inline std::vector<std::string> gdm::Config::GetAllKeys<std::string>(const std::string& name) const
{
  std::vector<std::string> result {};
  std::for_each(strings_.begin(), strings_.end(),
  [&name, &result](const std::pair<std::string, std::string>& pair)
  {
    if (pair.first.find(name) == 0)
      result.push_back(pair.first);
  });
  return result;
}

template<>
inline std::vector<std::string> gdm::Config::GetAllKeys<gdm::Vec3f>(const std::string& name) const
{
  std::vector<std::string> result {};
  std::for_each(vectors3f_.begin(), vectors3f_.end(),
  [&name, &result](const std::pair<std::string, gdm::Vec3f>& pair)
  {
    if (pair.first.find(name) == 0)
      result.push_back(pair.first);
  });
  return result;
}

template<>
inline std::vector<std::string> gdm::Config::GetAllKeys<gdm::Vec4f>(const std::string& name) const
{
  std::vector<std::string> result {};
  std::for_each(vectors4f_.begin(), vectors4f_.end(),
  [&name, &result](const std::pair<std::string, gdm::Vec4f>& pair)
  {
    if (pair.first.find(name) == 0)
      result.push_back(pair.first);
  });
  return result;
}

template<>
inline bool gdm::Config::Has<int>(const std::string& name) const
{ 
  return ints_.count(name);
}

template<>
inline bool gdm::Config::Has<float>(const std::string& name) const
{
  return floats_.count(name);
}

template<>
inline bool gdm::Config::Has<bool>(const std::string& name) const
{
  return bools_.count(name);
}

template<>
inline bool gdm::Config::Has<std::string>(const std::string& name) const
{
  return strings_.count(name);
}

template<>
inline bool gdm::Config::Has<gdm::Vec3f>(const std::string& name) const
{
  return vectors3f_.count(name);
}

template<>
inline bool gdm::Config::Has<gdm::Vec4f>(const std::string& name) const
{
  return vectors4f_.count(name);
}

template<>
inline bool gdm::Config::Has<std::vector<float>>(const std::string& name) const
{
  return vectorsf_.count(name);
}
