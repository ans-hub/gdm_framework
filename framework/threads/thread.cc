// *************************************************************
// File:    thread.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

// https://eli.thegreenplace.net/2016/c11-threads-affinity-and-hyperthreading/

#include "thread.h"

#if defined(_WIN32) || defined(_WIN64) 
#include <Windows.h>
#elif defined(__unix__)
#include <pthread.h>
#include <sched.h>
#endif

#include <cassert>

// --private

static int s_thread_number_ctor = -1;

// --public

gdm::Thread::Thread()
  : is_running_{true}
  , thread_{}
  , native_handle_{}
  , thread_id_{-1}
{
  Init();
}

void gdm::Thread::Join()
{
  if (thread_.joinable())
    thread_.join();
}

void gdm::Thread::Detach()
{
  thread_.detach();
}

bool gdm::Thread::SetAffinity(dword_t mask)
{
#if defined(_WIN32) || defined(_WIN64) 
  SYSTEM_INFO system_info;
  ::GetSystemInfo(&system_info);
  ::SetThreadAffinityMask(thread_.native_handle(), mask);
  return true;
#elif defined(__unix__)
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(mask, &cpuset);
  pthread_t curr_thread = thread_.native_handle();
  return pthread_setaffinity_np(curr_thread, sizeof(cpu_set_t), &cpuset); 
#endif
}

gdm::Thread::dword_t gdm::Thread::GetAffinity() const
{
#if defined(_WIN32) || defined(_WIN64) 
  dword_t curr_mask = 0;
  dword_t temp_mask = 1;
  while(temp_mask)
  {
    dword_t curr_mask = ::SetThreadAffinityMask(thread_.native_handle(), temp_mask);
    if (curr_mask)
    {
      ::SetThreadAffinityMask(thread_.native_handle(), curr_mask);
      return curr_mask;
    }
    else
    {
      dword_t err = ::GetLastError();
      if (err != ERROR_INVALID_PARAMETER)
        return 0;
    }
    temp_mask <<= 1;
  }
  return 0;
#elif defined(__unix__)
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  pthread_t curr_thread = thread_.native_handle();
  return pthread_getaffinity_np(curr_thread, sizeof(cpu_set_t), &cpuset); 
#endif
}

bool gdm::Thread::SetProcessor(unsigned core)
{
#if defined(_WIN32) || defined(_WIN64) 
  SYSTEM_INFO system_info;
  ::GetSystemInfo(&system_info);
  ::SetThreadIdealProcessor(thread_.native_handle(), core % system_info.dwNumberOfProcessors);
#endif

  return true;
}

gdm::Thread::uint_t gdm::Thread::GetProcessor() const
{
#if defined(_WIN32) || defined(_WIN64) 
  PPROCESSOR_NUMBER processor = nullptr;
  ::GetThreadIdealProcessorEx(thread_.native_handle(), processor);
  return processor->Number;
#else
  return 0;
#endif
}

bool gdm::Thread::SetPriority(core::Priority type)
{
#if defined(_WIN32) || defined(_WIN64) 
  return ::SetThreadPriority(thread_.native_handle(), static_cast<int>(type));
#else
  bool result = false;
  sched_param param;
  param.sched_priority = static_cast<decltype(param.sched_priority)>(type);
  int policy = SCHED_FIFO;
  result = pthread_setschedparam(thread_.native_handle(), policy, &param) != -1;  // also change schedulling policy
  param.sched_priority = 0;
  pthread_getschedparam(thread_.native_handle(), &policy, &param);
  result &= (param.sched_priority == static_cast<decltype(param.sched_priority)>(type)) && (policy == SCHED_FIFO);
  return result;
#endif
}

gdm::core::Priority gdm::Thread::GetPriority() const
{
#if defined(_WIN32) || defined(_WIN64) 
  return static_cast<core::Priority>(::GetThreadPriority(thread_.native_handle()));
#else
  return static_cast<Priority>(0);
#endif
}
bool gdm::Thread::SetRunning(bool running)
{
  return is_running_ = running;
}

bool gdm::Thread::IsRunning() const
{
  return is_running_;
}

// --private

void gdm::Thread::Init()
{
  thread_id_ = ++s_thread_number_ctor;
}
