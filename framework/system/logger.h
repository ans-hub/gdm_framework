// *************************************************************
// File:    logger.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_LOGGER_H
#define AH_GDM_LOGGER_H

#include <mutex>

namespace gdm {

namespace core {

  enum ELoggerProps : unsigned
  {
    LOG_TO_STDOUT = 1 << 1,
    LOG_TIMESTAMP = 1 << 2,
    LOCK_IO = 1 << 3,
    LOG_TO_FILE = 1 << 4

  }; // enum ELoggerProps

  using LoggerProps = unsigned;

} // namespace core

struct Logger
{
  static void LogThread();
  template<class ...Args>
  static void LogThread(const char* fmt, Args ...args);
  
public:
  static void SetFlags(core::LoggerProps flags);
  static void SetMutex(std::mutex& mutex);
  static bool GetFlag(core::LoggerProps flag);

private:
  template<class ...Args>
  static void PrintToStdout(const char* fmt, Args&& ...args);
  template<class ...Args>
  static void LogToFile(const char* fmt, Args&& ...args);

private:
  static unsigned flags_;
  static std::mutex* io_lock_;

}; // struct Logger

} // namespace gdm

#include "logger.inl"

#endif // AH_GDM_LOGGER_H
