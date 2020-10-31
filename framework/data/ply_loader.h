// *************************************************************
// File:    ply_loader.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_PLY_LOADER_H
#define AH_GDM_PLY_LOADER_H

#include <map>
#include <tuple>
#include <vector>
#include <iostream>
#include <sstream>
#include <cassert>

namespace gdm::ply {

using SingleTuple = std::tuple<std::string, unsigned, unsigned>;
using SingleMap   = std::map<std::string, SingleTuple>;
using ListTuple   = std::tuple<std::string, unsigned>;
using ListMap     = std::map<std::string, ListTuple>;

struct Header
{
  int       pos_;
  int       count_;
  SingleMap single_props_;
  ListMap   list_props_;

}; // struct Header

  using Vector1s  = std::vector<std::string>;
  using cVector1s = const Vector1s;
  using Vector1i  = std::vector<int>;
  using Vector2c  = std::vector<std::vector<char>>;
  using Vector1d  = std::vector<float>;
  using Vector2d  = std::vector<Vector1d>;
  using Vector3d  = std::vector<Vector2d>;
  using Vector4d  = std::vector<Vector3d>;
  using cVector3d = const Vector3d;
  using cVector4d = const Vector4d;
  using Str       = std::string;
  using MapV      = std::map<std::string, Vector1s>;
  using MapSS     = std::map<std::string, std::string>;
  using MapSI     = std::map<std::string, int>;
  using MHeader   = std::map<std::string, Header>;
  using cMHeader  = const std::map<std::string, Header>;

  struct Loader
  {
    Loader(const char* fname);

    bool Load(std::istream&);
    void LoadHeader(std::istream&);
    auto GetHeader() const -> cMHeader&;
    auto GetLine(const std::string&, cVector1s&) -> Vector2d;
    auto GetList(const std::string&, cVector1s&) -> Vector2d;
    auto GetString(const std::string& name) -> std::string;
    auto GetFilePath() const -> const std::string&;
    auto GetRawData() const -> cVector4d&;
    auto GetRawData(const std::string&) -> cVector3d&;
    
    bool IsElementPresent(const std::string&);
    bool IsSinglePropertyPresent(const std::string&, const std::string&);
    bool IsListPropertyPresent(const std::string&, const std::string&);
    bool IsStringPresent(const std::string&);

  private:
    void LoadData(std::istream&);
    void ClearData();

    Str       fpath_;
    Vector2c  props_;
    Vector1i  sizes_;
    MHeader   head_;
    Vector4d  data_;
    MapSI     order_;
    MapV      filter_;
    MapSS     strings_;

  }; // struct ply::Loader

struct Except : std::runtime_error
{
  Except(const std::string& msg) : std::runtime_error(msg.c_str()) { }
  Except(const char* msg) : std::runtime_error(msg) { }

}; // struct ply::Except

namespace helpers {

  void RewindStream(std::istream&);
  void ThrowLoadDataError(int, int);
  bool IsSinglePropertiesPresent(Loader&, const std::string&, cVector1s&);
  bool IsListPropertiesPresent(Loader&, const std::string&, cVector1s&);

  std::ostream& operator<<(std::ostream& , const Header&);
  std::ostream& operator<<(std::ostream& , const MHeader&);
  
  template<class T>
  std::vector<T> GetPlyData(ply::Loader& ply, const std::string& name,
                            cVector1s& fields, bool is_line);
  template<class T> // map
    bool IsKeyAbsentInMap(const std::string& str, const T& map);
  template<class T, class K>
    std::vector<K> ConvertRawTo2DUserData(const std::vector<T>& v);
  template<class T, class K>
    std::vector<K> ConvertRawTo3DUserData(const std::vector<T>& v);
  template<class T>
    std::vector<T> LoadXYZ(ply::Loader& ply);
  template<class T>
    std::vector<T> LoadRGB(ply::Loader& ply);
  template<class T>
    std::vector<T> LoadFaces(ply::Loader& ply);
  template<class T>
    std::vector<T> LoadUV(ply::Loader& ply);
  template<class T>
    std::vector<T> LoadNormals(ply::Loader& ply);
  std::string GetMaterialName(ply::Loader& ply);

} // namespace helpers

template<class T>
std::vector<T> helpers::GetPlyData(ply::Loader& ply, const std::string& name,
                                    cVector1s& fields, bool is_line)
{
  ply::Vector2d data {};
  if (is_line)
    data = ply.GetLine(name, fields);
  else
    data = ply.GetList(name, fields);
  
  std::vector<T> res {};
  for(const auto& vec1 : data)
    for(const auto& item : vec1)
      res.push_back(static_cast<T>(item));
  return res;
}

template<class T> // map
bool helpers::IsKeyAbsentInMap(const std::string& str, const T& map)
{
  if (map.find(str) == map.end())
    return true;
  else
    return false;
}

template<class T, class K>
inline std::vector<K> helpers::ConvertRawTo2DUserData(const std::vector<T>& v)
{
  assert(v.size() % 2 == 0);

  std::vector<K> data {};
  data.reserve(v.size() / 2);
  for (std::size_t i = 0; i < v.size(); i += 2)
    data.emplace_back(K{v[i], v[i+1]});
  return data;
}

template<class T, class K>
inline std::vector<K> helpers::ConvertRawTo3DUserData(const std::vector<T>& v)
{
  assert(v.size() % 3 == 0);

  std::vector<K> data {};
  data.reserve(v.size() / 3);
  for (std::size_t i = 0; i < v.size(); i += 3)
    data.emplace_back(K{v[i], v[i+1], v[i+2]});
  return data;
}

template<class T>
std::vector<T> helpers::LoadXYZ(ply::Loader& ply)
{
  std::vector<float> raw {};
  if (ply::helpers::IsSinglePropertiesPresent(ply, "vertex", {"x", "y", "z"}))
    raw = ply::helpers::GetPlyData<float>(ply, "vertex", {"x", "y", "z"}, true);
  return helpers::ConvertRawTo3DUserData<float,T>(raw);
}

template<class T>
std::vector<T> helpers::LoadRGB(ply::Loader& ply)
{
  std::vector<float> raw =
    ply::helpers::GetPlyData<float>(ply, "vertex", {"r", "g", "b"}, true);
  return helpers::ConvertRawTo3DUserData<float,T>(raw);
}

template<class T>
std::vector<T> helpers::LoadFaces(ply::Loader& ply)
{
  std::vector<unsigned int> raw {};
  if (ply.IsListPropertyPresent("face", "vertex_indices"))
    raw = ply::helpers::GetPlyData<unsigned int>(ply, "face", {"vertex_indices"}, false);
  return helpers::ConvertRawTo3DUserData<unsigned int,T>(raw);
}

template<class T>
std::vector<T> helpers::LoadUV(ply::Loader& ply)
{
  std::vector<float> raw {};
  if (ply::helpers::IsSinglePropertiesPresent(ply, "vertex", {"u", "v"}))
    raw = ply::helpers::GetPlyData<float>(ply, "vertex", {"u", "v"}, true);
  else if (ply::helpers::IsSinglePropertiesPresent(ply, "vertex", {"s", "t"}))
    raw = ply::helpers::GetPlyData<float>(ply, "vertex", {"s", "t"}, true);
  return helpers::ConvertRawTo2DUserData<float,T>(raw);
}

template<class T>
std::vector<T> helpers::LoadNormals(ply::Loader& ply)
{
  std::vector<float> raw {};
  if (ply::helpers::IsSinglePropertiesPresent(ply, "vertex", {"nx", "ny", "nz"}))
    raw = ply::helpers::GetPlyData<float>(ply, "vertex", {"nx", "ny", "nz"}, true);
  return helpers::ConvertRawTo3DUserData<float,T>(raw);
}

} // namespace gdm::ply

#endif  // AH_GDM_PLY_LOADER_H

// Explanation:

// #1: ply-file contains of "header_section" and "data_section". All data
// described in header described in format like: element type, number of items
// in this element and its properties (may be single or lists)

// I.e., more imortant fields of header section:
//   element vertex 26
//   property float x
//   property float y
//   element face 12
//   property list uchar uint ind
//   property uint non
//   property list uchar uint oth

// Element "vertex" contains 26 items and has two single properties
// Element "face" contains 12 items and has 2 list properies and 1 single

// In data section first goes elements described first. I.e., one of the
// element "vertex" (i.e. line 8) would looks like this:
//  2.52 1.24
// This means, that property x = 2.52, property y = 1.24

// And one of the "face" (i.e. line 2) element would looks like this:
//  3 9 8 7 100 4 2 1 1 1
// This means, that current item of element "face" has following properties:
// - list property "ind" contains of 3 elements (9,8,7)
// - single property "non" == 100
// - list property "oth" contains of 4 elements (2,1,1,1)

// #2: terms inside Loader class:
//  elem - element such as "vertex", "face", "edge", another
//  item - item of current element (declared in header, i.e. "vertex 20")
//  prop - properties of item (may be single or list properties)
//  val  - concrete value of propertie

// #3: how example described above would looks in Loader terms:
//  data_[0][8][0]  ==  std::vector(size == 2) contains {2.52, 1.24}
//  data_[1][2][0]  ==  std::vector(size == 3) contains {9,8,7}
//  data_[1][2][1]  ==  std::vector(size == 1) contains {100}
//  data_[1][2][0]  ==  std::vector(size == 4) contains {2,1,1,1}

// #4: data member explanation
//  data_  - stores loaded data in format data_[elem][item][prop][val]
//  props_ - contains sequence of properties type (do define when we should
//           push new propertie vector if we have list propery)
//  sizes_ - contains count of elements 

// #6: although construction of 4d vector is looks creepy, but all memory
// allocations made 

// # Todo:   l.SetFilter("vertex", {"x", "y", "z"});