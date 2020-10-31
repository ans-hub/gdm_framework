// *************************************************************
// File:    error_utils.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_ERROR_UTILS_H
#define AH_GDM_ERROR_UTILS_H

namespace gdm::error
{
  std::string LastError(int error_code);

} // namespace gdm::error

#include "error_utils.inl"

#endif  // AH_GDM_ERROR_UTILS_H