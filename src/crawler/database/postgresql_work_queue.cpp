#include "postgresql_work_queue.h"

#include <iterator>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>

#include "database/postgresql_adapter.h"
#include "database/postgresql_work.h"
#include "types/runnable.h"

namespace crawler::database {

PostgresWritesExecutor::PostgresWritesExecutor(
    PostgreSQLAdapter& adapter, 
    services::concurrency::LockFreeRingBuffer<std::shared_ptr<PostgresDatabaseWork>>& worksQueue, 
    const std::size_t processingBatchSize) :
  adapter_{adapter}, 
  worksQueue_{worksQueue},
  processingBatchSize_{processingBatchSize}
{}

void PostgresWritesExecutor::runImpl() {
  auto worksBatch = worksQueue_.popBatch(processingBatchSize_);
  for (auto& work: worksBatch) {
    auto status = adapter_.execute(work->toSQLCommand(), true);
    if (status.has_value()) {
      work->returnData().setResult(status.value());
    } else {
      work->returnData().setException(status.error()); 
    }
  }
}

PostgresReadExecutor::PostgresReadExecutor(
    PostgreSQLAdapter& adapter, 
    moodycamel::ConcurrentQueue<std::shared_ptr<PostgresSelectWork>>& worksQueue, 
    const std::size_t processingBatchSize) : 
  adapter_{adapter}, 
  worksQueue_{worksQueue},
  processingBatchSize_{processingBatchSize}
{}

void PostgresReadExecutor::runImpl() {
  std::vector<std::shared_ptr<PostgresSelectWork>> workBatch;
  workBatch.reserve(processingBatchSize_);
  worksQueue_.try_dequeue_bulk(std::back_inserter(workBatch), processingBatchSize_);

  for (auto& work: workBatch) {
    auto status = adapter_.execute(work->toSQLCommand());
    if (status.has_value()) {
      work->returnData().setResult(status.value());
    } else {
      work->returnData().setException(status.error()); 
    }
  }
}

PostgreSQLWorkQueue::PostgreSQLWorkQueue(
    std::string_view connectionString, 
    std::size_t numConnections,
    std::size_t processingBatchSize) : 
  processingBatchSize_(processingBatchSize), 
  adapter_(connectionString, numConnections)
{
  if (processingBatchSize_ == 0) {
    throw std::invalid_argument("[crawler::database::PostgreSQLWorkQueue] queries must be processed at least 1 per batch");
  }
}

void PostgreSQLWorkQueue::logWork(std::shared_ptr<PostgresDatabaseWork> work) {
  
  // check if the work is read or not
  auto selectWork = std::dynamic_pointer_cast<PostgresSelectWork>(work); 
  if (selectWork) {
    // check if reader is initialized
    bool isInitialized{false};
    {
      std::shared_lock<std::shared_mutex> readLock{resourcesMutex_};        
      isInitialized = readExecutor_ != nullptr; 
    }

    if (!isInitialized) [[unlikely]] {
      {
        std::unique_lock<std::shared_mutex> writeLock{resourcesMutex_};
        if (readExecutor_ == nullptr) {
          readExecutor_ = std::make_unique<types::RAIIRunnable>(
              std::make_unique<PostgresReadExecutor>(adapter_, readWorks_, processingBatchSize_)
          );      
        }
      }
    }

    readWorks_.try_enqueue(selectWork); 
  }
  else {
    
    auto tableName = work->tableName().data();

    // check if there exists a corresponding ring buffer and task executor (consumer)
    // for the database writes
    bool isInitialized{false};
    {
      std::shared_lock<std::shared_mutex> readLock{resourcesMutex_};
      isInitialized = writeWorks_.count(tableName);
    }

    if (!isInitialized) [[unlikely]] {
      {
        std::unique_lock<std::shared_mutex> writeLock{resourcesMutex_};
        if (!writeWorks_.count(tableName)) {
          writeWorks_.emplace(tableName, 1024uz);        
        }
      }
    }

    {
      std::shared_lock<std::shared_mutex> readLock{resourcesMutex_};
      writeWorks_.at(tableName).push(work);
    }
  }

}

}
