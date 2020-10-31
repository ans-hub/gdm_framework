// *************************************************************
// File:    data_storage.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_DATA_STORAGE_H
#define AH_GDM_DATA_STORAGE_H

#include <unordered_map>

namespace gdm {

using StorageHandle = Hash;

template<class T>
struct DataStorage
{
  DataStorage();
  ~DataStorage();

  template<class...Args>
  auto Create(Hash name, Args&&...args) -> StorageHandle;
  void Release(Hash name);
  bool Has(Hash name);
  auto Get(Hash name) -> T&;
  auto Get(Hash name) const -> const T&;

private:
  std::unordered_map<Hash, T*> data_;

}; // struct DataStorage

} // namespace gdm

#include "data_storage.inl"

#endif // AH_GDM_DATA_STORAGE_H