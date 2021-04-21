// *************************************************************
// File:    data_factory.inl
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "data_factory.h"

#include "system/assert_utils.h"

// --public

template <class T>
gdm::Handle gdm::DataFactory<T>::GetHandle(const char* fpath)
{
  Handle handle = helpers::GenerateHandle(fpath);

  auto found = resources_.find(handle);
  ASSERTF(found != resources_.end(), "Trying to get absent handle");
  return found->first;
}

template <class T>
T& gdm::DataFactory<T>::Get(Handle handle)
{
  ASSERTF(Has(handle), "Trying to get absent resource");
  T& resource = resources_[handle];
  return resource;
}

template <class T>
std::vector<T*> gdm::DataFactory<T>::Get(const std::vector<Handle>& handles)
{
  std::vector<T*> result;
  for (auto handle : handles)
  {
    ASSERTF(Has(handle), "Trying to get absent resource");
    T& resource = resources_[handle];
    result.push_back(&resource);
  }
  return result;
}

template <class T>
bool gdm::DataFactory<T>::Has(Handle handle)
{
  auto found = resources_.find(handle);
  bool ret = (found != resources_.end());
  return ret;
}

template <class T>
bool gdm::DataFactory<T>::Has(const char* fpath)
{
  Handle handle = helpers::GenerateHandle(fpath);
  return Has(handle);
}
