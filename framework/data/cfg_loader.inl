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
inline std::array<float,3> gdm::Config::Get<std::array<float,3>>(const std::string& name) const
{
  return vectors3f_.at(name);
}

template<>
inline std::array<float,4> gdm::Config::Get<std::array<float,4>>(const std::string& name) const
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
inline std::vector<std::array<float,3>> gdm::Config::GetAllVals<std::array<float,3>>(const std::string& name) const
{
  std::vector<std::array<float,3>> result {};
  std::for_each(vectors3f_.begin(), vectors3f_.end(),
  [&name, &result](const std::pair<std::string, std::array<float,3>>& pair)
  {
    if (pair.first.find(name) == 0)
      result.push_back(pair.second);
  });
  return result;
}

template<>
inline std::vector<std::array<float,4>> gdm::Config::GetAllVals<std::array<float,4>>(const std::string& name) const
{
  std::vector<std::array<float,4>> result {};
  std::for_each(vectors4f_.begin(), vectors4f_.end(),
  [&name, &result](const std::pair<std::string, std::array<float,4>>& pair)
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
inline std::vector<std::string> gdm::Config::GetAllKeys<std::array<float,3>>(const std::string& name) const
{
  std::vector<std::string> result {};
  std::for_each(vectors3f_.begin(), vectors3f_.end(),
  [&name, &result](const std::pair<std::string, std::array<float,3>>& pair)
  {
    if (pair.first.find(name) == 0)
      result.push_back(pair.first);
  });
  return result;
}

template<>
inline std::vector<std::string> gdm::Config::GetAllKeys<std::array<float,4>>(const std::string& name) const
{
  std::vector<std::string> result {};
  std::for_each(vectors4f_.begin(), vectors4f_.end(),
  [&name, &result](const std::pair<std::string, std::array<float,4>>& pair)
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
inline bool gdm::Config::Has<std::array<float,3>>(const std::string& name) const
{
  return vectors3f_.count(name);
}

template<>
inline bool gdm::Config::Has<std::array<float,4>>(const std::string& name) const
{
  return vectors4f_.count(name);
}

template<>
inline bool gdm::Config::Has<std::vector<float>>(const std::string& name) const
{
  return vectorsf_.count(name);
}
