// *************************************************************
// File:    bits_utils.inl
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "bits_utils.h"

inline bool gdm::bits::IsBigEndian()
{
  unsigned int v = 0x01;
  return (1 != reinterpret_cast<char*>(&v)[0]);
}

inline auto gdm::bits::Flip(const unsigned short& v) -> unsigned short
{
  return ((v >> 8) | (v << 8));
}

inline auto gdm::bits::Flip(const unsigned int& v) -> unsigned int
{
  return (
        ((v & 0xFF000000) >> 0x18) |
        ((v & 0x000000FF) << 0x18) |
        ((v & 0x00FF0000) >> 0x08) |
        ((v & 0x0000FF00) << 0x08)
  );
}

template<class T>
inline bool gdm::bits::HasFlag(unsigned int flags, T flag)
{ 
  return (flags & static_cast<unsigned int>(flag)) == static_cast<unsigned int>(flag);
}
