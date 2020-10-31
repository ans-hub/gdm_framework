// *************************************************************
// File:    semaphore_test.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

// https://preshing.com/20120515/memory-reordering-caught-in-the-act/

#include <thread>
#include <vector>
#include <iostream>
#include <random>

#include "threads/semaphore.h"

int X, Y;
int r1, r2;

gdm::Semaphore sem_begin1(0);
gdm::Semaphore sem_begin2(0);
gdm::Semaphore sem_end(0);

inline int GetRand(int from, int to)
{
  thread_local static std::random_device rd;
  thread_local static std::mt19937 rng(rd());
  thread_local std::uniform_int_distribution<int> u;
  return u(rng, decltype(u)::param_type{from, to});
}

static void fn1()
{
  for (;;)
  {
      sem_begin1.Wait();
      while (GetRand(0, 128) % 8 != 0) {}
      X = 1;
      __asm volatile("mfence" : : : "memory");
      r1 = Y;
      sem_end.Post();
  }
  return;
};

static void fn2()
{
  for (;;)
  {
      sem_begin2.Wait();
      while (GetRand(0, 128) % 8 != 0) {}
      X = 1;
      __asm __volatile__("mfence" ::: "memory");
      r1 = Y;
      sem_end.Post();
  }
  return;
};

int main()
{  
  std::thread thread1(&fn1);
  std::thread thread2(&fn2);
  
  int detected = 0;
  for (int iterations = 1; ; iterations++)
  {
    X = 0;
    Y = 0;
    sem_begin1.Post();
    sem_begin2.Post();
    
    sem_end.Wait();
    sem_end.Wait();

    assert(sem_begin1.Get() == 0);
    assert(sem_begin2.Get() == 0);
    assert(sem_end.Get() == 0);

    if (r1 == 0 && r2 == 0)
    {
      detected++;
      printf("%d reorders detected after %d iterations\n", detected, iterations);
    }
  }
  
  return 0;
} 
