// *************************************************************
// File:    job_queue.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_JOB_QUEUE_H
#define AH_GDM_JOB_QUEUE_H

#include <vector>
#include <queue>
#include <mutex>
#include <future>
#include <functional>
#include <condition_variable>

#include "threads/thread.h"

namespace gdm {

struct JobManager;

struct Job
{
  using CbSingle = std::function<void()>;
  using CbBatch = std::function<void(int from, int count)>;
  using CbBarrier = std::function<bool()>;
  
  enum EType { UNDEFINED, SINGLE, BATCH, BARRIER };

  Job();
  Job(CbSingle&& func, Job::EType type);
  Job(CbBatch&& func, int from, int size);

  bool Execute();
  auto GetType() const -> EType;

private:
  EType type_;
  CbSingle cb_entry_point_single_;
  CbBatch cb_entry_point_batch_;
  int cb_data_[2];

private:
  friend struct JobManager;

}; // struct Job

struct JobQueue
{
  JobQueue();
  JobQueue(const JobQueue& queue) = delete;

  void PushJob(Job::CbSingle func);
  void PushBatch(Job::CbBatch func, int batch_size, std::size_t data_size);
  void PushBarrier(Job::CbSingle func = {[](){ return true; }});
  
  void PushJobTS(Job::CbSingle func);
  void PushBatchTS(Job::CbBatch func, int batch_size, std::size_t data_size);
  void PushBarrierTS(Job::CbSingle func = {[](){ return true; }});
  
  auto GetMutex() -> std::timed_mutex& { return lock_; }

private:
  std::queue<Job> pending_jobs_;
  std::timed_mutex lock_;

private:
  friend struct JobManager;

}; // struct JobQueue

} // namespace gdm

#endif // AH_GDM_JOB_QUEUE_H
