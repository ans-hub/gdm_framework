// *************************************************************
// File:    exceptions.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GDM_WIN_EXCEPTION_H
#define GDM_WIN_EXCEPTION_H

#include <stdexcept>
#include <cerrno>
#include <cstring>

namespace gdm {

struct WinException : std::runtime_error
{
  WinException(const char* msg, int num)
    : std::runtime_error(msg)
    , msg_{nullptr}
  {
    msg_ = (char*)malloc(strlen(msg) + strlen(std::strerror(num) + 1));
    strcpy(msg_, msg);
  }
    
  ~WinException() { if (msg_) free(msg_); }

  virtual const char* what() const throw() { return msg_; }

private:
  char* msg_;

}; // struct WinException

}  // namespace gdm

#endif  // GDM_WIN_EXCEPTION_H