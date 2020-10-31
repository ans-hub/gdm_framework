// *************************************************************
// File:    data_storage.inl
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "data_storage.h"

#include <system/assert_utils.h>

// --public

template<class T>
inline gdm::DataStorage<T>::DataStorage()
  : data_{}
{ }

template<class T>
inline gdm::DataStorage<T>::~DataStorage()
{
  for(auto[name,ptr] : data_)
    GMDelete(ptr);
}

template<class T>
template<class...Args>
inline auto gdm::DataStorage<T>::Create(Hash name, Args&&...args) -> StorageHandle
{
  ASSERTF(!Has(name), "Create already created resource");
  data_[name] = GMNew T{std::forward<Args>(args)...};
  return StorageHandle(name);
}

template<class T>
inline void gdm::DataStorage<T>::Release(Hash name)
{
  ASSERTF(Has(name), "Release not created resource");
  T* ptr = Get(name);
  GMDelete(ptr);
  data_.erase(name);
}

template<class T>
inline bool gdm::DataStorage<T>::Has(Hash name)
{
  auto found = data_.find(name);
  return found != data_.end();
}

template<class T>
inline auto gdm::DataStorage<T>::Get(Hash name) -> T&
{
  ASSERTF(Has(name), "Get not created resource");
  return *data_[name];
}

template<class T>
inline auto gdm::DataStorage<T>::Get(Hash name) const -> const T&
{
  ASSERTF(Has(name), "Get not created resource");
  return *data_[name];
}
