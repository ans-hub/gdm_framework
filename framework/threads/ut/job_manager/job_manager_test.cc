// *************************************************************
// File:    job_manager_test.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include <vector>
#include <numeric>
#include <algorithm>

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>

#include "system/logger.h"
#include "system/profiler.h"
#include "threads/job_manager.h"

static gdm::JobManager* g_mgr;
static std::vector<int> g_cases_array; // array for tests

enum LogPriority { MAIN, FUNC, TEST };

enum SleepSubject { GENERAL, WORKERS };

struct TestSettings
{
  bool never_worker_sleep = false;
  bool never_main_sleep = false;
  bool lock_io = false;
  bool log_workers = false;
  bool log_main = false;
  bool log_func = false;
  bool log_test = false;
  int iterations_count = 1;
  int array_size = 25600000;
  int batch_size = 102400;
} g_test_settings;

static unsigned long TIME_NOW()
{
  return static_cast<unsigned long>(std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::system_clock::now().time_since_epoch()).count());
}

static constexpr unsigned HASH(const char* str, unsigned hash = 5381) // Djb2
{
  return *str ? HASH(str+1, hash * 33 + static_cast<unsigned>(*str)) : hash;
}

static void SLEEP_MS(SleepSubject subj, unsigned ms)
{
  if (subj == GENERAL)
    ms = g_test_settings.never_main_sleep ? 0 : ms;
  else if (subj == WORKERS)
    ms = g_test_settings.never_worker_sleep ? 0 : ms;
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

static void LOG(LogPriority priority, char const* const format, ...)
{
  if (priority == LogPriority::MAIN && !g_test_settings.log_main)
    return;
  if (priority == LogPriority::FUNC && !g_test_settings.log_func)
    return;
  if (priority == LogPriority::TEST && !g_test_settings.log_test)
    return;

  if (g_test_settings.lock_io)
    gdm::mx::io_lock.lock();

  printf("\t");
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
  
  if (g_test_settings.lock_io)
    gdm::mx::io_lock.unlock();
}

static int DATA_SUM(const std::vector<int>& array)
{
  return std::accumulate(array.begin(), array.end(), 0);
}

static void DATA_FILL(std::vector<int>& array, int val_)
{
  std::for_each(array.begin(), array.end(), [&val_](int& val){ val = val_; });
}

int case_1_diff_jobs_semantic()
{
  LOG(FUNC, "case_1_diff_jobs_semantic()\n");

  struct Temp
  {
    void DoIt(int i) { LOG(TEST, "some_job_func_%d();\n", i); }
  } some_runtime_class;

  auto func_1 = [](){ LOG(TEST, "some_job func_1();\n"); };
  auto func_2 = [&some_runtime_class](){ some_runtime_class.DoIt(9); };
  
  {
    CPU_PROFILE_SCOPE("ThreadMain", "ex_1", core::COLOR_AUTO);
    gdm::JobQueue& queue = g_mgr->GetJobQueue();

    queue.PushJobTS(func_1);
    queue.PushJobTS(func_2);
    g_mgr->WaitOnBarrierTS();
  }

  return 0;
}

int case_2_batch_jobs_semantic()
{
  LOG(FUNC, "case_2_batch_jobs_semantic()\n");  

  auto func_rotate = [](int begin, int length)
  {
    for (auto i = begin; i < begin + length; ++i) g_cases_array[i] = 2;
    SLEEP_MS(WORKERS, 100);
    LOG(TEST, "some_batch_func(): %d, %d finishedl;\n", begin, length);
  };

  
  DATA_FILL(g_cases_array, 1);
  LOG(FUNC, "batch data before: %d\n", DATA_SUM(g_cases_array));  
  int total_summ = DATA_SUM(g_cases_array);

  {
    CPU_PROFILE_SCOPE("ThreadMain", "ex_2", core::COLOR_AUTO);
    gdm::JobQueue& queue = g_mgr->GetJobQueue();
    queue.PushBatchTS(func_rotate, g_test_settings.batch_size, g_cases_array.size());
    g_mgr->WaitOnBarrierTS();
  }

  total_summ = DATA_SUM(g_cases_array);
  LOG(FUNC, "batch data after:  %d\n\n", total_summ);
  assert(total_summ == g_test_settings.array_size * 2);

  return total_summ;
}

int case_3_jobs_with_deps()
{
  LOG(FUNC, "case_3_jobs_with_deps()\n");

  auto fill = [](int begin, int length){
    for (int i = begin; i < begin + length; ++i) g_cases_array[i] = 1; SLEEP_MS(WORKERS, 100); };
  auto set = [](int begin, int length){
    for (int i = begin; i < begin + length; ++i) g_cases_array[i] += 1; SLEEP_MS(WORKERS, 100); };
  auto test = [](){
    int sum = DATA_SUM(g_cases_array); LOG(TEST, "some_test_fn(): %d\n", sum); assert(sum == g_test_settings.array_size * 2); };
  auto mov = [](int begin, int length){
    for (int i = begin; i < begin + length; i += 2) g_cases_array[i] += 1; SLEEP_MS(WORKERS, 100); }; 
  auto rot = [](int begin, int length){
    for (int i = begin + 1; i < begin + length; i += 2) g_cases_array[i] += 1; SLEEP_MS(WORKERS, 100); };
  auto print = [](){
    int sum = DATA_SUM(g_cases_array); LOG(TEST, "total_count_fn(): %d\n", sum); assert(sum == g_test_settings.array_size * 3); };

  LOG(FUNC, "batch data before: %d\n", DATA_SUM(g_cases_array));  
  int total_summ = DATA_SUM(g_cases_array);

  {
    CPU_PROFILE_SCOPE("ThreadMain", "ex_3", core::COLOR_AUTO);
    gdm::JobQueue& queue = g_mgr->GetJobQueue();
    std::unique_lock<std::timed_mutex> lock(queue.GetMutex());

    queue.PushBatch(fill, g_test_settings.batch_size, g_cases_array.size());
    queue.PushBarrier();
    queue.PushBatch(set, g_test_settings.batch_size, g_cases_array.size());
    queue.PushBarrier();
    queue.PushJob(test);
    queue.PushBarrier();
    queue.PushBatch(mov, g_test_settings.batch_size, g_cases_array.size());
    queue.PushBatch(rot, g_test_settings.batch_size, g_cases_array.size());
    queue.PushBarrier();
    queue.PushJob(print);
  }
  g_mgr->WaitOnBarrierTS();

  total_summ = DATA_SUM(g_cases_array);
  LOG(FUNC, "batch data after:  %d\n\n", total_summ);
  assert(total_summ == g_test_settings.array_size * 3);

  return total_summ;
}

int case_4_simulate_post_from_diff_sources()
{
  LOG(FUNC, "case_4_simulate_post_from_diff_sources()\n");

  std::vector<int> array(g_cases_array.size(), 1);

  auto fill = [&](int begin, int length){
    for (int i = begin; i < begin + length; ++i) array[i] = 1; SLEEP_MS(WORKERS, 100); };
  auto set = [&](int begin, int length){
    for (int i = begin; i < begin + length; ++i) array[i] += 1; SLEEP_MS(WORKERS, 100); };
  auto test = [&](){
    int sum = DATA_SUM(array); LOG(TEST, "some_test_fn(): %d\n", sum); assert(sum == g_test_settings.array_size * 2); };
  auto mov = [&](int begin, int length){
    for (int i = begin; i < begin + length; i += 2) array[i] += 1; SLEEP_MS(WORKERS, 100); }; 
  auto rot = [&](int begin, int length){
    for (int i = begin + 1; i < begin + length; i += 2) array[i] += 1; SLEEP_MS(WORKERS, 100); };
  auto print = [&](){
    int sum = DATA_SUM(array); LOG(TEST, "total_count_fn(): %d\n", sum); assert(sum == g_test_settings.array_size * 3); };

  LOG(FUNC, "batch data before: %d\n", DATA_SUM(array));
  int total_summ = DATA_SUM(array);

  {
    CPU_PROFILE_SCOPE("ThreadMain", "ex_4", core::COLOR_AUTO);
    gdm::JobQueue& queue = g_mgr->GetJobQueue();
    std::unique_lock<std::timed_mutex> lock(queue.GetMutex());

    queue.PushBatch(fill, g_test_settings.batch_size, g_cases_array.size());
    queue.PushBarrier();
    queue.PushBatch(set, g_test_settings.batch_size, g_cases_array.size());
    queue.PushBarrier();
    queue.PushJob(test);
    queue.PushBarrier();
    queue.PushBatch(mov, g_test_settings.batch_size, g_cases_array.size());
    queue.PushBatch(rot, g_test_settings.batch_size, g_cases_array.size());
    queue.PushBarrier();
    queue.PushJob(print);
  }
  g_mgr->WaitOnBarrierTS();

  total_summ = DATA_SUM(array);
  LOG(FUNC, "batch data after:  %d\n\n", total_summ);
  assert(total_summ == g_test_settings.array_size * 3);

  return total_summ;
}

void case_4_simulate_post_from_diff_sources_separate_thread()
{
  static int cnt;
  ++cnt;
  if(cnt > 1)
    return;

  static std::thread t{[](){
    GDM_PROFILE_THIS_THREAD_ENABLE("main_dup");
    unsigned dt = 0;
    unsigned last_time = 0;
    for(int i = 0; i < g_test_settings.iterations_count; ++i)
    {
      dt += TIME_NOW() - last_time;
      last_time = TIME_NOW();
      case_4_simulate_post_from_diff_sources();
      GDM_PROFILER_FRAME();
    }
  }};
  t.detach();
}

int main(int argc, const char** argv)
{
  g_mgr = new gdm::JobManager{};

  g_test_settings.never_worker_sleep = argc > 1 ? atoi(argv[1]): true;
  g_test_settings.never_main_sleep = argc > 2 ? atoi(argv[2]): true;
  g_test_settings.lock_io = argc > 3 ? atoi(argv[3]) : false;
  g_test_settings.log_workers = argc > 4 ? atoi(argv[4]) : false;
  g_test_settings.log_main = argc > 5 ? atoi(argv[5]) >> 1 : true;
  g_test_settings.log_func = argc > 5 ? atoi(argv[5]) >> 2 : false;
  g_test_settings.log_test = argc > 5 ? atoi(argv[5]) >> 3 : false;
  g_test_settings.iterations_count = argc > 6 ? atoi(argv[6]) : 256 * 256 * 256;
  g_test_settings.array_size = 4150;
  g_test_settings.batch_size = 256;

  GDM_PROFILING_ENABLE();
  GDM_PROFILE_THIS_THREAD_ENABLE("main");

  if (g_test_settings.log_workers)
  {
    if (g_test_settings.lock_io)
    {
      gdm::Logger::SetMutex(gdm::mx::io_lock);
      gdm::Logger::SetFlags(gdm::core::LOG_TO_STDOUT | gdm::core::LOG_TIMESTAMP | gdm::core::LOCK_IO);
    }
    else
      gdm::Logger::SetFlags(gdm::core::LOG_TO_STDOUT | gdm::core::LOG_TIMESTAMP);
  }

  g_cases_array = std::vector<int>(g_test_settings.array_size, 1);

  unsigned long dt = 0;
  unsigned long last_time = TIME_NOW();  
  int i = 0;
  for (; i < g_test_settings.iterations_count; ++i)
  {
    CPU_PROFILE_SCOPE("ThreadMain", "loop", core::COLOR_DARKORANGE);

    dt += TIME_NOW() - last_time;
    last_time = TIME_NOW();
    if (dt >= 1000)
      LOG(MAIN, "ITERATION #%d : ", i);

    int total = 0;
#if 1
    // first scenario - sequenced adding jobs
    total += case_1_diff_jobs_semantic();
    SLEEP_MS(GENERAL, 100);
    total += case_2_batch_jobs_semantic();
    SLEEP_MS(GENERAL, 100);
    total += case_3_jobs_with_deps();
    SLEEP_MS(GENERAL, 100);
    total += case_4_simulate_post_from_diff_sources();
    SLEEP_MS(GENERAL, 100);
    assert(total == g_test_settings.array_size * 8);
#ifdef NDEBUG
    if(total != g_test_settings.array_size * 8)
      return -1;
#endif
#endif

#if 0
    // second scenario - adding jobs from different threads
    total += case_4_simulate_post_from_diff_sources();
    case_4_simulate_post_from_diff_sources_separate_thread();
    SLEEP_MS(GENERAL, 100);
#ifdef NDEBUG
    if(total != g_test_settings.array_size * 3)
      return -1;
#endif
#endif

    if (dt >= 1000)
    {
      LOG(MAIN, " == %u\t%d\n\n", DATA_SUM(g_cases_array), total);
      dt = 0;
    }
    GDM_PROFILER_FRAME();
  }

  GDM_PROFILER_SHUTDOWN();
  LOG(MAIN, "COMPLETED %d\n", i);

  return 0;
} 
