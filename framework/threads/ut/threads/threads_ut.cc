// *************************************************************
// File:    threads_ut.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

// https://eli.thegreenplace.net/2016/c11-threads-affinity-and-hyperthreading/

// win: start /affinity 0x1 "proc.exe"
// linux: taskset -5,6 (set for 5 and 6 cpu)

// (1<<4)-1 -> 0b01111 - all 4 cpus
// (1<<4)   -> 0b10000 - 5th cpu

#ifdef _WIN32
#include <windows.h>
#endif

#include <vector>
#include <iostream>

#include "3rdparty/catch/catch.hpp"

#include "threads/thread.h"
#include "threads/spin_lock.h"

struct S
{
  void Call(char ch)
  {
    while(true)
    {
      (void)ch;
      std::cout << "T5\n";
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "End\n";
  }
};

TEST_CASE("Threads")
{
  const int max_iters = 128;
  gdm::SpinLock cs;
  
  auto print_repeat = ([&cs, &max_iters](){
    int iter = 0;
    while(iter < max_iters)
    {
      cs.Lock();
      std::cout << "Thread " << gdm::Thread::GetId() << " on CPU " << ::GetCurrentProcessorNumber() << ", "
                << "priority " << ::GetThreadPriority(::GetCurrentThread()) << '\n'; // sched_getcpu()
      ++iter;
      cs.Unlock();
    }
  });
  
  auto print_once = ([&cs](){
    cs.Lock();
    std::cout << "Thread " << gdm::Thread::GetId() << " " << std::this_thread::get_id() << " on CPU " << ::GetCurrentProcessorNumber() << ", "
              << "priority " << ::GetThreadPriority(::GetCurrentThread()) << '\n'; // sched_getcpu()
    cs.Unlock();
  });

  SECTION("Lambda")
  {
    gdm::Thread t3 (print_repeat);
    gdm::Thread t4 (print_repeat);
    t3.SetAffinity(1ull << 3);
    t4.SetAffinity((1ull << 2) - 1);
    t3.Join();
    t4.Join();
  }

  SECTION("Member function")
  {
    S s;
    gdm::Thread t5 (&S::Call, s, 'q');
    t5.Detach();
  }

  SECTION("ThreadId")
  {
    const int thread_cnt = 8;
    int iter = 0;
    while(iter < max_iters)
    {
      std::vector<gdm::Thread> threads;
      for (int i = 0; i < thread_cnt; ++i)
        threads.emplace_back(print_once);
      for (int i = 0; i < thread_cnt; ++i)
        threads[i].Join();
      ++iter;
    }
  }
}
