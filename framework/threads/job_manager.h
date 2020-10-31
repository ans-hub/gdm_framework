// *************************************************************
// File:    job_manager.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_JOB_MANAGER_H
#define AH_GDM_JOB_MANAGER_H

#include <vector>
#include <queue>
#include <mutex>
#include <future>
#include <functional>
#include <condition_variable>

#include "threads/thread.h"
#include "threads/job_queue.h"

namespace gdm {

namespace core {

  enum EJobManagerProps : unsigned
  {
    PRINT_LOG = 1 << 1,
    SAVE_LOG = 1 << 2
  
  }; // enum EJobManagerProps
  
  using JobManagerProps = unsigned;

} // namespace core

struct JobManager
{
  JobManager(core::JobManagerProps flags = 0); 
  ~JobManager();

  auto GetJobQueue() -> JobQueue&;
  void WaitOnBarrier();
  void WaitOnBarrierTS();

private:
  unsigned flags_;
  unsigned main_cpu_;
  unsigned cpu_count_;
  std::vector<Thread> thread_pool_;
  std::vector<int> thread_status_;
  JobQueue queue_;

private:
  struct RuntimeStat
  {
    std::atomic<int> running_jobs;
    std::atomic<int> workers_stopped;
  } static s_stat;

private:
  auto GetJob(bool& no_jobs) -> Job;
  void ExecuteJob(Job& job);
  void WakeUpThreads();
  void SleepThisThread();
  auto GetWorker(unsigned worker_num) -> Thread&;

private:
  static void WorkerFunc(void* job_manager_ptr, unsigned worker_num);

}; // struct JobManager

namespace mx
{
  static std::mutex io_lock {};
  static std::mutex wait_jobs {};
}

namespace cv
{
  static std::condition_variable wait_jobs {};
  static bool wait_jobs_proof = false;
}

} // namespace gdm

#endif // AH_GDM_JOB_MANAGER_H
