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

#include <atomic>
#include <cassert>

//--private

namespace gdm::_private
{
  static std::atomic<int> v_thread_ids[core::v_max_thread_id] = {};
  static const std::hash<std::thread::id> hash_thread_id = {};

  static int MapThreadIdToInteger()
  {
    const std::thread::id thread_id = std::this_thread::get_id();
    const int clamped_thread_id = hash_thread_id(thread_id) % INT_MAX;
    const int reserved_word = INT_MAX;

    int new_index = -1;
    int existed_index = -1;

    for(int i = 0; i < core::v_max_thread_id && existed_index == -1; ++i)
    {
      int expected = 0;
      if (new_index == -1 && v_thread_ids[i].compare_exchange_strong(expected, reserved_word))
        new_index = i;
      if (v_thread_ids[i] == clamped_thread_id)
        existed_index = i;
    }

    assert(new_index != -1 || existed_index != -1);
    
    if (existed_index == -1)
    {
      v_thread_ids[new_index] = clamped_thread_id;
      return new_index;
    }
    else
    {
      v_thread_ids[new_index] = -1;
      return existed_index;
    }
  }

  static void UnmapThreadId()
  {
    const std::thread::id thread_id = std::this_thread::get_id();
    const int clamped_thread_id = hash_thread_id(thread_id) % INT_MAX;

    for(int i = 0; i < core::v_max_thread_id; ++i)
    {
      if (v_thread_ids[i] == clamped_thread_id)
        v_thread_ids[i] = 0;
    }
  }

  struct ThreadIdWrapper
  {
    ThreadIdWrapper() { index = _private::MapThreadIdToInteger(); }
    ~ThreadIdWrapper() { _private::UnmapThreadId(); }

    int index = -1;
  };
}

// --public

gdm::Thread::Thread()
  : is_running_{true}
  , thread_{}
  , native_handle_{}
{ }

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
  ::SetThreadAffinityMask(thread_.native_handle(), static_cast<DWORD_PTR>(mask));
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

//--static

auto gdm::Thread::GetId() -> int
{
  thread_local _private::ThreadIdWrapper v_thread_id_wrapper;

  return v_thread_id_wrapper.index;
}
