// *************************************************************
// File:    error_utils.inl
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "system/error_utils.h"

#include <system_error>

inline std::string gdm::error::LastError(int error_code)
{
  return std::system_category().message(error_code);
}
