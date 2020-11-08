// *************************************************************
// File:    vk_deleter.handle
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GM_VK_DELETER_H
#define GM_VK_DELETER_H

namespace gdm::vk {

template<class ResourceHandle>
struct VkDeleter
{
  template<class ResourceHandle>
  using VkDeleteFunction = std::function<void(ResourceHandle t)>;

  VkDeleter()
    : delete_fn_{}
    , handle_{VK_NULL_HANDLE}
  { }

  VkDeleter(VkDeleteFunction<ResourceHandle> delete_fn, ResourceHandle handle)
    : delete_fn_{delete_fn}
    , handle_{handle}
  { }

  VkDeleter(const VkDeleter& other) = delete;

  VkDeleter(VkDeleter&& other)
    : handle_{}
    , delete_fn_{}
  {
    std::swap(handle_, other.handle_);
    other.delete_fn_.swap(delete_fn_);
  }

  VkDeleter& operator=(VkDeleter&& other)
  {
    std::swap(handle_, other.handle_);
    delete_fn_.swap(other.delete_fn_);
    return *this;
  }

  ~VkDeleter()
  {
    if (delete_fn_)
      delete_fn_(handle_);
  }
  
  operator ResourceHandle() { return handle_; }
  operator ResourceHandle() const { return handle_; }

private:
  VkDeleteFunction<ResourceHandle> delete_fn_;
  ResourceHandle handle_; 
};

} // namespace gdm::vk

#endif // GM_VK_DELETER_H
