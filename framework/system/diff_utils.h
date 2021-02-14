// *************************************************************
// File:    diff_utils.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_DIFF_UTILS_H
#define AH_GDM_DIFF_UTILS_H

#include <span>

#ifndef UNUSED
#define UNUSED(x) (&reinterpret_cast<const int&>(x))
#endif

namespace gdm {

  template<class T, class IterBegin = decltype(std::begin(std::declval<T>())),
                    class IterEnd = decltype(std::end(std::declval<T>()))>
  inline constexpr auto Enumerate(T&& iterable)
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

  template<class...T>
  struct ZipSpan
  {
    struct Iterator
    {
      Iterator(const ZipSpan& zip_span, size_t index) : zip_span_(zip_span), index_(index) { }

      auto operator++() -> Iterator& { ++index_; return *this; }
      auto operator--() -> Iterator&  { --index_; return *this; }
      bool operator==(const Iterator& other) { return index_ == other.index_ && &zip_span_ == &other.zip_span_; }
      bool operator!=(const Iterator& other) { return !(*this == other); }
      auto operator*() -> std::tuple<T&...> { return Get(std::make_integer_sequence<int, sizeof...(T)>{}); }

    private:
      template <int...indices>
      auto Get(std::integer_sequence<int, indices...>) const { return std::tie(std::get<indices>(zip_span_.spans_)[index_]...); }

    private:
      const ZipSpan& zip_span_;
      size_t index_;
    };

    ZipSpan(std::span<T>... spans);
    ZipSpan(const ZipSpan&) = delete;
    ZipSpan(ZipSpan&&) = delete;

    Iterator begin() const;
    Iterator end() const;

  private:
      std::tuple<std::span<T>...> spans_;

  }; // struct ZipSpan

  template<class...T>
  void CheckSpansSizes(std::span<T>...spans);

} // namespace gdm

namespace gdm::range
{
  template<class...T>
  auto ZipSpan(const T&... args)
  {
    return ::gdm::ZipSpan(std::span(args)...);
  }

  template<class...T>
  auto ZipSpan(T&... args)
  {
    return ::gdm::ZipSpan(std::span(args)...);
  }
}

#include "diff_utils.inl"

#endif  // AH_GDM_DIFF_UTILS_H