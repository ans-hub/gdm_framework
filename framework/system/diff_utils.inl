// *************************************************************
// File:    diff_utils.inl
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "diff_utils.h"

#include "assert_utils.h"

//--public

template<class...T>
void gdm::CheckSpansSizes(std::span<T>...spans)
{
    const size_t sizes[] = {spans.size()...};

    for (size_t size : sizes)
    {
        if (size != sizes[0])
        {
          ASSERTF(false, "Spans sizes should be equal");
        }
    }
}

template<class...T>
gdm::ZipSpan<T...>::ZipSpan(std::span<T>...spans)
{
  CheckSpansSizes(spans...);
  spans_ = std::make_tuple(spans...);
}

template<class...T>
typename gdm::ZipSpan<T...>::Iterator gdm::ZipSpan<T...>::begin() const
{
  return Iterator(*this, 0);
}

template<class...T>
typename gdm::ZipSpan<T...>::Iterator gdm::ZipSpan<T...>::end() const
{
  return Iterator(*this, std::get<0>(spans_).size());
}
