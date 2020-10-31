// *************************************************************
// File:    job_manager.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#pragma warning(disable: 4805 4477) // int->bool, sprintf 

#include "job_manager.h"

#include <assert.h>
#include <time.h>
#include <chrono>
#include <string.h>

#include "threads/scoped_locks.h"

#include "system/profiler.h"
#include "system/event_point.h"

// -- log macro

#define FMT_SMPL "#%d %s\t\n", worker_num, GDM_EVENT_STR
#define FMT_STAT "#%d %s %d %lu\t\n", worker_num, GDM_EVENT_STR, s_stat.running_jobs.load(), mgr.queue_.pending_jobs_.size()
#define FMT_STAT2 "#%d %s %d %lu\t\n", s_tls.worker_num, GDM_EVENT_STR, s_stat.running_jobs.load(), queue_.pending_jobs_.size()

// --private

struct TLS
{
  char worker_name[256] = ""; 
  unsigned worker_num = 0;
} thread_local s_tls;

gdm::JobManager::RuntimeStat gdm::JobManager::s_stat = {};

// --public

gdm::JobManager::JobManager(core::JobManagerProps flags)
  : main_cpu_{0}
  , cpu_count_{std::thread::hardware_concurrency()}
  , flags_{flags}
  , queue_{}
  , thread_status_(std::thread::hardware_concurrency() - 1,0)
{
  s_stat.running_jobs = 0;
  s_stat.workers_stopped = 0;

  int done = 1;
  thread_pool_.reserve(cpu_count_-1);
  for (unsigned i = 0; i < cpu_count_-1; ++i)
  {
    thread_pool_.emplace_back(WorkerFunc, this, i);
    done = thread_pool_.back().SetProcessor(i);
    done &= thread_pool_.back().SetPriority(core::ABOVE_NORMAL);
    thread_pool_.back().Detach();
  }
  if(!done)
      printf("can't change threads property (priority, processor, etc)\n");
}

gdm::JobManager::~JobManager()
{
  for (unsigned i = 0; i < thread_pool_.size(); ++i)
    thread_pool_[i].SetRunning(false);
  while(s_stat.workers_stopped != thread_pool_.size()) { }
}

gdm::JobQueue& gdm::JobManager::GetJobQueue()
{
  return queue_;
}

void gdm::JobManager::WaitOnBarrier()
{
  GDM_EVENT_POINT("WaitOnBarrier", GDM_CPU_G("WorkerGrp", core::COLOR_MEDIUMAQUAMARINE) GDM_LOG_E());

  std::promise<void> signal {};
  queue_.PushBarrier([&signal](){ signal.set_value(); });
  WakeUpThreads();
  signal.get_future().wait();
}

void gdm::JobManager::WaitOnBarrierTS()
{
  GDM_EVENT_POINT("WaitOnBarrier", GDM_CPU_G("WorkerGrp", core::COLOR_MEDIUMAQUAMARINE) GDM_LOG_E());
  std::promise<void> signal;
  queue_.PushBarrierTS([&signal](){ signal.set_value(); });
  WakeUpThreads();
  signal.get_future().wait();
}

// --private

gdm::Job gdm::JobManager::GetJob(bool& no_jobs)
{
  GDM_EVENT_POINT("GetJob", GDM_LOG_E());  
  bool locked = false;
  GDM_TRY_LOCK_FOR(100, locked, queue_.GetMutex(), "mx:wt", core::COLOR_WHITESMOKE);
  if(!locked)
    return {};

  Job job;
  if (!queue_.pending_jobs_.empty()){
    job = queue_.pending_jobs_.front();
    if(job.type_ != Job::BARRIER || s_stat.running_jobs == 0){
      queue_.pending_jobs_.pop();
      no_jobs = false;
      ++s_stat.running_jobs;
    }
  }
  return job;
}

void gdm::JobManager::ExecuteJob(Job& job)
{
  job.Execute();
  --s_stat.running_jobs;
}

void gdm::JobManager::WakeUpThreads()
{
  GDM_EVENT_POINT("wakeup", GDM_CPU_G("WorkerGrp", core::COLOR_MEDIUMAQUAMARINE) GDM_LOG_E());
  {
    GDM_LOCK_GUARD(mx::wait_jobs, "mx:wu", core::COLOR_WHITESMOKE);
    for (int& status : thread_status_)
      status = 1;
  }
  cv::wait_jobs.notify_all();
  // cv::wait_jobs.notify_one(); // todo: why
}

void gdm::JobManager::SleepThisThread()
{
  GDM_EVENT_POINT("sleep", GDM_LOG_E());
  GDM_UNIQUE_LOCK_VAR(mx::wait_jobs, lock, "mx::wt", core::COLOR_WHITESMOKE);

  // prevents sleep when all threads here but while all on wait for mutex,
  // somewho post jobs (sort of tradeoff)
  if(!queue_.pending_jobs_.empty())
    return;

  thread_status_[s_tls.worker_num] = 0;
  cv::wait_jobs.wait(lock, [&](){ return thread_status_[s_tls.worker_num] == 1; });
}

gdm::Thread& gdm::JobManager::GetWorker(unsigned worker_num)
{
  return thread_pool_[worker_num];
}

// --private static

void gdm::JobManager::WorkerFunc(void* job_manager_ptr, unsigned worker_num)
{
  JobManager& mgr = *(static_cast<JobManager*>(job_manager_ptr));
  std::queue<Job>& queue = mgr.queue_.pending_jobs_;
  snprintf(s_tls.worker_name, sizeof(s_tls.worker_name), "Worker_%d", worker_num);
  s_tls.worker_num = worker_num;

  GDM_PROFILE_THIS_THREAD_ENABLE(s_tls.worker_name);

  while (mgr.GetWorker(worker_num).IsRunning())
  {
    GDM_EVENT_POINT("loop", GDM_CPU_G("WorkerGrp", core::COLOR_DARKORANGE) GDM_LOG(FMT_STAT));
    Job job;
    bool no_jobs = true;
    {
      GDM_EVENT_POINT("get", GDM_CPU_G("WorkerGrp", core::COLOR_MEDIUMAQUAMARINE) GDM_LOG(FMT_STAT));
      job = mgr.GetJob(no_jobs);
    }
    if (no_jobs){
      GDM_EVENT_POINT("sleep", GDM_CPU_G("WorkerGrp", core::COLOR_PERU) GDM_LOG(FMT_STAT));
      mgr.SleepThisThread();
    }
    else{
      GDM_EVENT_POINT("exec", GDM_CPU_G("WorkerGrp", core::COLOR_DARKVIOLET) GDM_LOG(FMT_STAT));
      mgr.ExecuteJob(job);
#if 0
      // todo: unnecessary since currently we are not sleep while running_jobs
      GDM_EVENT_POINT("wakeup", GDM_CPU_G("WorkerGrp", core::COLOR_INDIANRED2) GDM_LOG(FMT_STAT));
      mgr.WakeUpThreads();
#endif
    }
  }
  GDM_EVENT_POINT("term", GDM_LOG(FMT_SMPL));
  ++mgr.s_stat.workers_stopped;
}
