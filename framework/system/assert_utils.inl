// *************************************************************
// File:    assert_utils.inl
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "assert_utils.h"

#include <sstream>
#include <string>

#include "debugging.h"

# if !defined(NDEBUG)

namespace gdm::_private
{
  struct AssertionError : public std::runtime_error
  {
    AssertionError(const char* msg) : std::runtime_error(msg)
    {
      MessageBoxA(nullptr, msg, "Assertf", MB_OK);
    }
  };
} // namespace gdm::_private

inline void gdm::AssertImpl(const char* msg)
{
  if (debug::IsDebuggerAttached())
    debug::DebugBreak();

  throw gdm::_private::AssertionError(msg);
}

#endif