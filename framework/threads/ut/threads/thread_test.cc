// *************************************************************
// File:    thread_test.cc
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

#include "threads/thread.h"
#include "threads/spin_lock.h"

#include <vector>
#include <iostream>

struct S
{
  void Call(char ch)
  {
    while(true)
    {
      (void)ch;
      std::cout << "T5\n";
      std::this_thread::sleep_for(std::chrono::seconds(1));
      break;
    }
    std::cout << "End\n";
  }
};

int main()
{
  gdm::SpinLock cs;
  
  auto func = ([&cs](int thread_num){
    while(1)
    {
      cs.Lock();
      std::cout << "Thread " << thread_num << " on CPU " << ::GetCurrentProcessorNumber() << ", "
                << "priority " << ::GetThreadPriority(::GetCurrentThread()) << '\n'; // sched_getcpu()
      cs.Unlock();
    }
  });
  gdm::Thread t3 (func, 1);
  gdm::Thread t4 (func, 2);

  S s;
  gdm::Thread t5 (&S::Call, s, 'q');
  t5.Detach();

  t3.SetAffinity(1ull << 3);
  t4.SetAffinity((1ull << 2) - 1);
  t3.Join();
  t4.Join();
  return 0;
} 
