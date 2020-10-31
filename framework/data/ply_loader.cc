// *************************************************************
// File:    ply_loader.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "ply_loader.h"

#include <fstream>
#include <cassert>
#include <string>

// --public

gdm::ply::Loader::Loader(const char* fpath)
  : fpath_{fpath}
{
  std::ifstream fss {fpath};
  assert(fss && "Couldn't load file");
  Load(fss);
}

bool gdm::ply::Loader::Load(std::istream& iss)
{
  ClearData();
  helpers::RewindStream(iss);
  
  std::string line {};
  std::getline(iss, line);

  if (line != "ply")
  {
    iss.setstate(std::ios::failbit);
    throw ply::Except("Load(): not a ply file");
  }

  LoadHeader(iss);
  LoadData(iss);

  return true;
}

void gdm::ply::Loader::LoadHeader(std::istream& iss)
{
  std::string line {""};                // to store line of istream
  std::string word {""};                // to store words of line
  std::string name {""};                // to store prop name of line
  std::string elem {""};                // to store elem name of line
  std::string type {""};                // to store type of property
  int order {0};                        // to store pos of curr elem in data_ arr
  int num {0};                          // to store input integer
  int pos {0};                          // current prop pos in prop array
  int prop {0};                         // current prop array of element

  while(std::getline(iss, line))
  {
    std::istringstream stream {line};   // to read word by word
    stream >> word;                     // get word
    
    if (word == "element")              // if first word is "element"
    {
      stream >> elem >> num;            // get line like "vertex 26"
      props_.emplace_back();            // allocate new properties describer
      sizes_.push_back(num);            // add elem count
      data_.emplace_back();             // add new element
      data_.back().resize(num);         // and resize it for store num of items
      head_[elem].count_ = num;         // set name and count of element
      order_[elem] = order;             // remember the order of element
      pos = 0;
      prop = 0;
      ++order;
    }

    else if (word == "property")
    {
      stream >> type;
      if (type == "list")
      {
        stream >> type >> type >> name;
        pos = 0;
        if (prop != 0) ++prop;
        head_[elem].list_props_[name] = std::make_tuple(type, prop);
        ++prop;
        props_.back().push_back('l');
      }
      else
      {
        stream >> name;
        head_[elem].single_props_[name] = std::make_tuple(type, prop, pos);
        ++pos;
        props_.back().push_back('d');
      }
    }
    else if (word == "custom")
    {
      stream >> type >> name;
      strings_[type] = name;
    }

    else if (word == "end_header")
      break;
  }
}

gdm::ply::cMHeader& gdm::ply::Loader::GetHeader() const
{
  return head_;
}

const std::string& gdm::ply::Loader::GetFilePath() const
{
  return fpath_;
}

gdm::ply::cVector4d& gdm::ply::Loader::GetRawData() const
{
  return data_;
}

gdm::ply::cVector3d& gdm::ply::Loader::GetRawData(const std::string& key)
{
  if (helpers::IsKeyAbsentInMap(key, order_))
    throw Except(std::string{"GetRawData(): elem absent - "} + std::string{key});
  return data_[order_[key]];
}

gdm::ply::Vector2d gdm::ply::Loader::GetLine(const std::string& elem, const Vector1s& filter)
{
  if (helpers::IsKeyAbsentInMap(elem, order_))
    throw Except(std::string{"GetLine(): elem absent - "} + std::string{elem});
  
  for (const auto& prop : filter)
    if (helpers::IsKeyAbsentInMap(prop, head_[elem].single_props_))
    {
      std::string msg = std::string{"GetLine(): in element "} + std::string{elem} + std::string{" prop absent - "} + std::string{prop};
      throw Except(msg);
    }

  auto& curr = data_[order_[elem]];
  Vector2d res (curr.size());
  
  for (std::size_t i = 0; i < curr.size(); ++i)
  {
    res[i].reserve(filter.size());
    for (auto& f : filter)
    {
      std::size_t prop_arr = std::get<1>(head_[elem].single_props_[f]); // prop_arr pos
      std::size_t prop_pos = std::get<2>(head_[elem].single_props_[f]); // elem_pos in prop_arr
      res[i].push_back(curr[i][prop_arr][prop_pos]);
    }
  }
  return res;
}

gdm::ply::Vector2d gdm::ply::Loader::GetList(const std::string& elem, cVector1s& filter)
{
  assert(filter.size() == 1);

  if (helpers::IsKeyAbsentInMap(elem, order_))
    throw Except(std::string{"GetLine(): elem absent - "} + std::string{elem});

  if (helpers::IsKeyAbsentInMap(filter.back(), head_[elem].list_props_))
    throw Except(std::string{"GetLine(): property absent - "} + std::string{filter.back()});

  auto& curr = data_[order_[elem]];
  Vector2d res (curr.size());

  for (std::size_t i = 0; i < curr.size(); ++i)
  {
    std::size_t prop_arr = std::get<1>(head_[elem].list_props_[filter.back()]); // prop_arr pos
    res[i] = curr[i][prop_arr];
  }
  return res;
}

std::string gdm::ply::Loader::GetString(const std::string& name)
{
  if (IsStringPresent(name))
    return strings_[name];
  return {""};
}
// --private

// Reads data after header (istream pos shoudld stay at the data pos)
// First would meets elements which were described first in the header.
//
// I.e.:
//  header section: "vertex 20", "face 12" (element and items count)
//  data in stream: first 20 lines is vertexes, next 12 lines is faces
//  data in class:  data_[0] is 20 vertexes, data_[1] is 12 faces
//
// Each data_[elem][item] contains sub arrays, which represents properties.
// I.e. 1:  header section: "vertex 10" +6 single properties
//          data in class:  data_[0][item][0][0..5]
// I.e. 2:  header section: "face 6" +1 list, +2 single properties, +1 list
//          data in class:  data_[1][item][0][size of first list]
//                          data_[1][item][1][0..1]
//                          data_[2][item][2][size of second list]
// I.e. 3:  header section: "edge 8" +1 list
//          data in class:  data_[2][item][0][size of first list]

void gdm::ply::Loader::LoadData(std::istream& iss)
{
  std::string line {""};  // user to store line of stream
  std::string word {""};  // used to store word from line 
  int    ival {0};        // used to store integer value from stream
  double dval {0};        // used to store double value from stream
  int    elem {0};        // element type index (vertex, face, etc...)
  int    item {0};        // item index of curr element 

  int total_sizes {0};    // needs to check is data lines is equal sizes
  for (const auto& sz : sizes_)
    total_sizes += sz;

  while(std::getline(iss, line))
  {
    std::istringstream stream {line};   // to read word by word
    
    if (sizes_[elem] == 0) {            // if we have read all data of curr elem
      ++elem;                           //  then go to the next element
      item = 0;
    }
    
    // Reserve max space to store arrays of properties values

    data_[elem][item].reserve(props_[elem].size());

    // Now we have data line. We should read it and understand which type of
    // property we read now. Basing on this knowledge we read all words in line

    char last_prop {'l'};
    for (auto& prop : props_[elem])
    {
      // If this is the "list", then add new property array to data_[elem][item]

      if (prop == 'l')
      {                                 
        last_prop = 'l';
        if (!(stream >> ival))          // get num of sub properties in curr list
          helpers::ThrowLoadDataError(elem, item);

        data_[elem][item].emplace_back();
        data_[elem][item].back().reserve(ival);
        
        while (ival)                    // reads all sub properties in the list
        {
          if (!(stream >> dval))
            helpers::ThrowLoadDataError(elem, item);

          data_[elem][item].back().push_back(static_cast<float>(dval));
          --ival;
        }
      }
      
      // If this is the single property and previous was a "list", then add
      // new property array to data_[elem][item], else just fill back()

      else if (prop == 'd')
      {
        if (last_prop == 'l') {
            data_[elem][item].emplace_back();
            data_[elem][item].back().reserve(props_[elem].size());
          last_prop = 'd';
        }
        if (!(stream >> dval))
          helpers::ThrowLoadDataError(elem, item);
        data_[elem][item].back().push_back(static_cast<float>(dval));
      }
    }

    // Go to the next item and reduce curr elem sizes

    ++item;
    --sizes_[elem];
    --total_sizes;
  }

  if (total_sizes != 0)
    throw Except("LoadData(): sizes in header is not equal fact");
}

void gdm::ply::Loader::ClearData()
{
  props_ = Vector2c {};
  sizes_ = Vector1i {};
  data_ = Vector4d {};
  head_ = MHeader {};
  order_ = MapSI {};
  strings_ = MapSS {};
  filter_.clear();
}

bool gdm::ply::Loader::IsElementPresent(const std::string& elem)
{
  return !helpers::IsKeyAbsentInMap(elem, strings_);
} 

bool gdm::ply::Loader::IsSinglePropertyPresent(
  const std::string& elem, const std::string& prop)
{
  return !helpers::IsKeyAbsentInMap(prop, head_[elem].single_props_);
}

bool gdm::ply::Loader::IsListPropertyPresent(const std::string& elem, const std::string& prop)
{
  return !helpers::IsKeyAbsentInMap(prop, head_[elem].list_props_);
}

bool gdm::ply::Loader::IsStringPresent(const std::string& str)
{
  return !helpers::IsKeyAbsentInMap(str, strings_);
} 

void gdm::ply::helpers::RewindStream(std::istream& iss)
{
  iss.clear();
  iss.seekg(0);
}

void gdm::ply::helpers::ThrowLoadDataError(int elem_type, int item_num)
{
  std::ostringstream oss {};
  oss << "LoadData(): invalid data in "
      << "elem " << elem_type << ' ' 
      << "item " << item_num;
  throw Except(oss.str().c_str());
}

bool gdm::ply::helpers::IsSinglePropertiesPresent(Loader& ply, const std::string& elem, const Vector1s& props)
{
  for (const auto& prop : props)
    if (!ply.IsSinglePropertyPresent(elem, prop))
      return false;
  return true;
}

bool gdm::ply::helpers::IsListPropertiesPresent(Loader& ply, const std::string& elem, const Vector1s& props)
{
  for (const auto& prop : props)
    if (!ply.IsListPropertyPresent(elem, prop))
      return false;
  return true;
}

std::string gdm::ply::helpers::GetMaterialName(Loader& ply)
{
  std::string tex_fname {};
  if (ply.IsStringPresent("material"))
    tex_fname = ply.GetString("material");
  return tex_fname;
}

std::ostream& gdm::ply::helpers::operator<<(std::ostream& oss, const MHeader& header)
{
  for (const auto& elem : header)
  {
    oss << "Element: " << elem.first << ", ";
    oss << elem.second;
  }
  return oss;
}

std::ostream& gdm::ply::helpers::operator<<(std::ostream& oss, const Header& h)
{
  oss << "count: " << h.count_ << '\n';

  if (!h.single_props_.empty())
    oss << "  Single properies: \n";
 
  for (const auto& prop : h.single_props_)
  {
    oss << "    name: " << prop.first << ", "; 
    oss << "type: " << std::get<0>(prop.second) << ", ";
    oss << "arr: " << std::get<1>(prop.second) << ", ";
    oss << "pos: " << std::get<2>(prop.second) << '\n';
  }

  if (!h.list_props_.empty())
    oss << "  List properies: \n";

  for (const auto& prop : h.list_props_)
  {
    oss << "    name: " << prop.first << ", "; 
    oss << "type: " << std::get<0>(prop.second) << ", ";
    oss << "arr: " << std::get<1>(prop.second) << '\n';
  }
  return oss;
}
