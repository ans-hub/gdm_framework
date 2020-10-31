// *************************************************************
// File:    logger.inl
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "logger.h"

// --public

template<class ...Args>
void gdm::Logger::LogThread(const char* fmt, Args ...args)
{
  if (GetFlag(core::LOG_TO_STDOUT))
    PrintToStdout(fmt, std::forward<Args>(args)...);

  if (GetFlag(core::LOG_TO_FILE))
    LogToFile(fmt, std::forward<Args>(args)...);
}

template<class ...Args>
void gdm::Logger::PrintToStdout(const char* fmt, Args&& ...args)
{
  if (GetFlag(core::LOCK_IO) && io_lock_)
    io_lock_->lock();

  if (GetFlag(core::LOG_TIMESTAMP))
  {
    const auto sc = std::chrono::system_clock::now();
    double ts = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(sc.time_since_epoch()).count());
    printf("%.0f : ", ts);
  }

  printf(fmt, std::forward<Args>(args)...);

  if (GetFlag(core::LOCK_IO) && io_lock_)
    io_lock_->unlock();
}

template<class ...Args>
void gdm::Logger::LogToFile(const char* fmt, Args&& ...args) { }
