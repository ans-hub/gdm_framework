// *************************************************************
// File:    critical_section.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "critical_section.h"

namespace gdm {

gdm::CriticalSection::CriticalSection()
  : CriticalSection(0x00000400)
{ }

gdm::CriticalSection::CriticalSection(unsigned spin_time)
  : spin_time_{spin_time}
{
  InitializeCriticalSectionAndSpinCount(&critical_section_, spin_time_);
}

gdm::CriticalSection::~CriticalSection()
{
  DeleteCriticalSection(&critical_section_);
}

void gdm::CriticalSection::TryLock()
{
  TryEnterCriticalSection(&critical_section_);
}

void gdm::CriticalSection::Lock()
{
  EnterCriticalSection(&critical_section_);
}

void gdm::CriticalSection::Unlock()
{
  LeaveCriticalSection(&critical_section_);
}

} // namespace gdm