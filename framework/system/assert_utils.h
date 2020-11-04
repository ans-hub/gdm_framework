// *************************************************************
// File:    assert_utils.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_MACRO_H
#define AH_GDM_MACRO_H

#include <cassert>
#include <cstring>
#include <cstdio>
#include <cstdlib>

#include <exception>
#include <stdexcept>

#ifdef _WIN32
#include <tchar.h>
#include <windows.h>
#endif

#include "error_utils.h"

#pragma warning(disable:4996)

namespace gdm {

#if !defined(NDEBUG)
struct AssertionError : public std::runtime_error
{
  AssertionError(const char* msg);
};

void AssertImpl(bool condition, const char* expr, const char* file, const char* func, int line);
#endif

} // namespace gdm

#ifdef _WIN32

# if !defined(NDEBUG)

#   define CA_SZ 4096   // ##__VA_ARGS__ prevents comma before VA_ARGS if it empty
#   define CTXT(x) x
#   define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

#   define LOGF(M, ...) { char s[CA_SZ]; sprintf(s, CTXT(M), ##__VA_ARGS__); OutputDebugStringA(s); }
#   define LOGFS(fmt, ...) { char s[CA_SZ]; sprintf(s, CTXT("LOG: %s(): ") CTXT(fmt), __func__, ##__VA_ARGS__); OutputDebugStringA(s); }
#   define LOGFM(fmt, ...) { char s[CA_SZ]; sprintf(s, CTXT("LOG: %s:%d %s(): ") CTXT(fmt), __FILENAME__, __LINE__, __func__, ##__VA_ARGS__); OutputDebugStringA(s); }
#   define LOGFF(fmt, ...) { char s[CA_SZ]; sprintf(s, CTXT("LOG: %s:%d %s(): ") CTXT(fmt), __FILE__, __LINE__, __func__, ##__VA_ARGS__); OutputDebugStringA(s); }
#   define ENSUREF(A, M, ...) { if (!(A)) { char s[CA_SZ]; sprintf(s, CTXT(M), ##__VA_ARGS__); MessageBoxA(nullptr, s, CTXT("Ensure"), MB_OK); } }
#   define ASSERTF(A, M, ...) if (!(A)) { char s[CA_SZ]; memset(s, 0, CA_SZ); sprintf(s, CTXT("%s\nMessage: ") CTXT(M), CTXT(#A), ##__VA_ARGS__); throw ::gdm::AssertionError(s); }
#   define ASSERT(A) { if (!(A)) throw ::gdm::AssertionError(#A); }
#   define SASSERT(A) { static_assert(A); }
#   define VEREFY(A) { if (!(A)) { char s[CA_SZ]; sprintf(s, #A); MessageBoxA(nullptr, s, CTXT("Verify",) MB_OK); std::abort(); } }
#   define VEREFYF(A, M, ...) { if (!(A)) { char s[CA_SZ]; sprintf(s, CTXT(M), ##__VA_ARGS__); MessageBoxA(nullptr, s, CTXT("Verify"), MB_OK); std::abort(); } }
# else
#   define LOGF(...)
#   define LOGFS(...)
#   define LOGFM(...)
#   define LOGFF(...)
#   define ENSUREF(...)
#   define ASSERTF(...)
#   define ASSERT(...)
#   define SASSERT(...)
#   define VEREFY(...)
#   define VEREFYF(...)
# endif // NDEBUG

#else // LINUX

#   define CA_SZ 4096
#   define CTXT(x) x
#   define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

// todo: still need to check

#   define LOGF(M, ...) { char s[CA_SZ]; sprintf(s, CTXT(M), ##__VA_ARGS__); print(s); }
#   define LOGFS(fmt, ...) { char s[CA_SZ]; sprintf(s, CTXT("LOG: %s(): ") CTXT(fmt), __func__, ##__VA_ARGS__); print(s); }
#   define LOGFM(fmt, ...) { char s[CA_SZ]; sprintf(s, CTXT("LOG: %s:%d %s(): ") CTXT(fmt), __FILENAME__, __LINE__, __func__, ##__VA_ARGS__); print(s); }
#   define LOGFF(fmt, ...) { char s[CA_SZ]; sprintf(s, CTXT("LOG: %s:%d %s(): ") CTXT(fmt), __FILE__, __LINE__, __func__, ##__VA_ARGS__); print(s); }
#   define ENSUREF(A, M, ...) { if (!(A)) { char s[CA_SZ]; sprintf(s, CTXT(M), ##__VA_ARGS__); print(s); }
#   define ASSERTF(A, M, ...) if (!(A)) { char s[CA_SZ]; memset(s, 0, CA_SZ); sprintf(s, CTXT("%s\nMessage: ") CTXT(M), CTXT(#A), ##__VA_ARGS__); assert(s, CTXT(__FILE__), (unsigned)__LINE__); }
#   define ASSERT(A) { assert(A); }
#   define SASSERT(A) { static_assert(A); }
#   define VEREFY(A) { if (!(A)) { char s[CA_SZ]; sprintf(s, #A); print(nullptr, s, CTXT("Verefy",) MB_OK); std::abort(); } }
#   define VEREFYF(A, M, ...) { if (!(A)) { char s[CA_SZ]; sprintf(s, CTXT(M), ##__VA_ARGS__); print(s); std::abort(); } }

#endif

#include "assert_utils.inl"

#endif  // AH_GDM_MACRO_H