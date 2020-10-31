// *************************************************************
// File:    job_queue.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "job_queue.h"

#include <assert.h>

#include "threads/scoped_locks.h"

// --public

gdm::Job::Job()
  : type_{EType::UNDEFINED}
{ }

gdm::Job::Job(CbSingle&& func, Job::EType type)
  : type_{type}
  , cb_entry_point_single_{func}
{
  assert(type == EType::SINGLE || type == EType::BARRIER);
}

gdm::Job::Job(CbBatch&& func, int from, int size)
  : type_{EType::BATCH}
  , cb_entry_point_batch_{func}
  , cb_data_{from, size}
{ }

bool gdm::Job::Execute()
{
  switch (type_)
  {
    case Job::SINGLE  : cb_entry_point_single_.operator()(); break;
    case Job::BARRIER : cb_entry_point_single_.operator()(); break;
    case Job::BATCH   : cb_entry_point_batch_.operator()(cb_data_[0], cb_data_[1]); break;
    default : assert(false && "Undefined job"); return false;
  }
  return true;
}

gdm::Job::EType gdm::Job::GetType() const
{
  return type_;
}

// --public JobQueue

gdm::JobQueue::JobQueue()
  : pending_jobs_{}
  , lock_{}
{ }

void gdm::JobQueue::PushJob(Job::CbSingle func)
{
  pending_jobs_.push(Job{std::move(func), Job::SINGLE});
}

void gdm::JobQueue::PushBatch(Job::CbBatch func, int batch_size, std::size_t data_size)
{
  assert(batch_size > 0);
  
  int curr_data_size = static_cast<int>(data_size);
  int curr_batch_size = static_cast<int>(batch_size);

  for (int i = 0; i < curr_data_size; i += curr_batch_size)
  {
    int batch_overflow = i + curr_batch_size > curr_data_size ? i + curr_batch_size - curr_data_size : 0;
    curr_batch_size -= batch_overflow;
    pending_jobs_.push(Job{std::move(func), i, curr_batch_size});
  }
}

void gdm::JobQueue::PushBarrier(Job::CbSingle func)
{
  pending_jobs_.push(Job(std::move(func), Job::BARRIER));
}

void gdm::JobQueue::PushJobTS(Job::CbSingle func)
{
  GDM_UNIQUE_LOCK(lock_, "mx::pj", core::COLOR_WHITESMOKE);
  PushJob(func);
}

void gdm::JobQueue::PushBatchTS(Job::CbBatch func, int batch_size, std::size_t data_size)
{
  GDM_UNIQUE_LOCK(lock_, "mx::pb", core::COLOR_WHITESMOKE);
  PushBatch(func, batch_size, data_size);
}

void gdm::JobQueue::PushBarrierTS(Job::CbSingle func)
{
  GDM_UNIQUE_LOCK(lock_, "mx::pb", core::COLOR_WHITESMOKE);
  PushBarrier(func);
}
