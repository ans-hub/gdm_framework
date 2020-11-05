// *************************************************************
// File:    diff_utils.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_DIFF_MACRO_H
#define AH_GDM_DIFF_MACRO_H

#define UNUSED(x) (&reinterpret_cast<const int&>(x))

namespace gdm {

template<class T, class IterBegin = decltype(std::begin(std::declval<T>())),
                  class IterEnd = decltype(std::end(std::declval<T>()))>
constexpr auto Enumerate(T&& iterable)
{
  struct Iterator
  {
    std::size_t i;
    IterBegin iter;
    bool operator !=(const Iterator& other) const { return iter != other.iter; }
    void operator ++() { ++i; ++iter; }
    auto operator *() const { return std::tie(i, *iter); }
  };
  struct Wrapper
  {
    T iterable;
    auto begin() { return Iterator{0, std::begin(iterable) }; }
    auto end() { return Iterator{0, std::end(iterable) }; }
  };
  return Wrapper{std::forward<T>(iterable)};
}

} // namespace gdm

#endif  // AH_GDM_DIFF_MACRO_H