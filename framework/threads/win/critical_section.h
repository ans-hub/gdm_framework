// *************************************************************
// File:    critical_section.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_CRITICAL_SECTION_H
#define AH_GDM_CRITICAL_SECTION_H

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

namespace gdm {

struct CriticalSection
{
  CriticalSection();
  CriticalSection(unsigned spin_time);
  ~CriticalSection();

  void Lock();
  void TryLock();
  void Unlock();

private:
  RTL_CRITICAL_SECTION critical_section_;
  unsigned spin_time_;

}; // struct CriticalSection

}  // namespace gdm

#endif // AH_GDM_CRITICAL_SECTION
