#pragma once

#include "postgresql_adapter.h"
#include "postgresql_work.h"
#include "services/ring_buffer.h"
#include "types/runnable.h"

#include <moodycamel/concurrentqueue.h>
#include <condition_variable>
#include <shared_mutex>

namespace crawler::database {

class PostgresWritesExecutor : public types::Runnable {
public:

  explicit PostgresWritesExecutor(
      PostgreSQLAdapter& adapter, 
      services::concurrency::LockFreeRingBuffer<std::shared_ptr<PostgresDatabaseWork>>& worksQueue, 
      const std::size_t processingBatchSize);

  void runImpl() override;

private:
  PostgreSQLAdapter& adapter_;
  services::concurrency::LockFreeRingBuffer<std::shared_ptr<PostgresDatabaseWork>>& worksQueue_;  

  const std::size_t processingBatchSize_;
};


class PostgresReadExecutor : public types::Runnable {
public:

  explicit PostgresReadExecutor(
      PostgreSQLAdapter& adapter,
      moodycamel::ConcurrentQueue<std::shared_ptr<PostgresSelectWork>>& worksQueue, 
      const std::size_t processingBatchSize);

  void runImpl() override;
    
private:
  PostgreSQLAdapter& adapter_;
  moodycamel::ConcurrentQueue<std::shared_ptr<PostgresSelectWork>>& worksQueue_;

  const std::size_t processingBatchSize_;
};


class PostgreSQLWorkQueue {
public:

  PostgreSQLWorkQueue(
      std::string_view connectionString, 
      std::size_t numConnections, 
      std::size_t processingBatchSize = 1);

  void logWork(std::shared_ptr<PostgresDatabaseWork> work);

private:
  
  moodycamel::ConcurrentQueue<std::shared_ptr<PostgresSelectWork>> readWorks_;
  std::unordered_map<std::string, services::concurrency::LockFreeRingBuffer<std::shared_ptr<PostgresDatabaseWork>>> writeWorks_;
  
  std::unique_ptr<types::RAIIRunnable> readExecutor_{nullptr};
  std::vector<types::RAIIRunnable> executors_;  
  const std::size_t processingBatchSize_{1};    

  PostgreSQLAdapter adapter_;

  std::shared_mutex resourcesMutex_;
  std::condition_variable cv_;
};

}
