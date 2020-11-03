// *************************************************************
// File:    assert_utils.inl
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "assert_utils.h"

#include <sstream>
#include <string>

# if !defined(NDEBUG)

inline gdm::AssertionError::AssertionError(const char* msg)
  : std::runtime_error(msg)
{
  MessageBoxA(nullptr, msg, "Assertf", MB_OK);
}

inline void gdm::AssertImpl(bool condition, const char* expression, const char* file, const char* func, int line)
{
  if (condition)
  {
    std::stringstream ss;
    ss << "Assert " << expression << " in " << file << ":" << line << " ( " << func << ") failed";
    throw AssertionError(ss.str().c_str());
  }
}
#endif