// *************************************************************
// File:    thread.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_THREAD_H
#define AH_GDM_THREAD_H

#include <thread>
#include <atomic>

namespace gdm {

namespace core {

  enum EPriority : int
  {
#if defined(_WIN32) || defined(_WIN64) 
    IDLE = -15,
    LOWEST = -2,
    BELOW_NORMAL = -1,
    NORMAL = 0,
    ABOVE_NORMAL = 1,
    HIGHEST = 2,
    TIME_CRITICAL = 15
#else
    IDLE = 1,
    LOWEST = 10,
    BELOW_NORMAL = 15,
    NORMAL = 30,
    ABOVE_NORMAL = 42,
    HIGHEST = 60,
    TIME_CRITICAL = 80
#endif
  }; // enum EPriority

  using Priority = int;
  using ThreadId = std::thread::id;

  constexpr const int v_max_thread_id = 32;

} // namespace core

struct Thread
{
  using uint_t = unsigned;
  using dword_t = unsigned long long;
  
public:
  Thread();
  Thread(const std::thread& rhs) = delete;
  Thread& operator=(const std::thread& rhs) = delete;
  Thread(Thread&& rhs) = default;
  Thread& operator=(std::thread&& rhs) = delete;
  Thread(const Thread& rhs) = delete;
  template <class Fx, class... Args>
  explicit Thread(Fx&& fx, Args&&... args);

public:
  void Join();
  void Detach();
  
  bool SetAffinity(dword_t mask);
  bool SetProcessor(uint_t core);
  bool SetPriority(core::Priority type);
  bool SetRunning(bool running);
  
  auto GetAffinity() const -> dword_t;
  auto GetProcessor() const -> uint_t;
  auto GetPriority() const -> core::Priority;
  bool IsRunning() const;

public:
  static auto GetId() -> int;

private:
  void Init();
  void Deinit();

private:
  bool is_running_;
  mutable std::thread thread_;
  std::thread::native_handle_type native_handle_;

}; // struct Thread

} // namespace gdm

#include "thread.inl"

#endif // AH_GDM_THREAD_H
