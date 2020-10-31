// *************************************************************
// File:    logger.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "logger.h"

// --private

unsigned gdm::Logger::flags_ = 0;
std::mutex* gdm::Logger::io_lock_ = nullptr;

// --public

void gdm::Logger::SetFlags(core::LoggerProps flags)
{
  flags_ = flags;
}

void gdm::Logger::SetMutex(std::mutex& mutex)
{
  io_lock_ = &mutex;
}

bool gdm::Logger::GetFlag(gdm::core::LoggerProps flag)
{
  return (flags_ & flag) == flag;
}
