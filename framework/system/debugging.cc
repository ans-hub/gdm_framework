// *************************************************************
// File:    debugging.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "debugging.h"

#include <Windows.h>

void gdm::debug::DebugBreak()
{
#if defined(LS_POSIX)
	__asm int 0x03;
#elif defined(_WIN32)
	__debugbreak();
#endif
}

bool gdm::debug::IsDebuggerAttached()
{
#if defined(_WIN32)
	return ::IsDebuggerPresent() == TRUE;
#else
	return false;
#endif
}
