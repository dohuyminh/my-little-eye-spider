#include <gtest/gtest.h>

#include "crawler/components/frontier/multiqueue_containers.h"

using namespace crawler::components;

// ============================================================================
// QueueType Concept Tests
// ============================================================================

namespace queue_type_concept_tests {

// Compile-time tests: verify that our wrappers satisfy the QueueType concept
static_assert(QueueType<StdQueueWrapper<int>>);
static_assert(QueueType<MoodyCamelConcurrentQueueWrapper<int>>);

// Test with std::string
static_assert(QueueType<StdQueueWrapper<std::string>>);
static_assert(QueueType<MoodyCamelConcurrentQueueWrapper<std::string>>);

// Test that non-queue types don't satisfy the concept (negative test)
struct NotAQueue {};
static_assert(!QueueType<NotAQueue>);

} // namespace queue_type_concept_tests

// ============================================================================
// StdQueueWrapper Tests
// ============================================================================

namespace std_queue_wrapper_tests {

TEST(StdQueueWrapperTest, EnqueueDequeueSingleElement) {
  StdQueueWrapper<int> queue;

  queue.enqueue(42);
  auto result = queue.dequeue();

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 42);
}

TEST(StdQueueWrapperTest, DequeueEmptyReturnsNullopt) {
  StdQueueWrapper<int> queue;

  auto result = queue.dequeue();

  EXPECT_FALSE(result.has_value());
}

TEST(StdQueueWrapperTest, EnqueueDequeueMultipleElements) {
  StdQueueWrapper<int> queue;

  queue.enqueue(1);
  queue.enqueue(2);
  queue.enqueue(3);

  EXPECT_EQ(queue.dequeue().value(), 1);
  EXPECT_EQ(queue.dequeue().value(), 2);
  EXPECT_EQ(queue.dequeue().value(), 3);
  EXPECT_FALSE(queue.dequeue().has_value());
}

TEST(StdQueueWrapperTest, EnqueueDequeueLvalueReference) {
  StdQueueWrapper<int> queue;
  int value = 100;

  queue.enqueue(value);  // lvalue
  auto result = queue.dequeue();

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 100);
}

TEST(StdQueueWrapperTest, EnqueueDequeueRvalueReference) {
  StdQueueWrapper<int> queue;

  queue.enqueue(std::move(200));  // rvalue
  auto result = queue.dequeue();

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 200);
}

TEST(StdQueueWrapperTest, EnqueueBulk) {
  StdQueueWrapper<int> queue;
  std::vector<int> values = {1, 2, 3, 4, 5};

  queue.enqueue_bulk(std::move(values));

  EXPECT_EQ(queue.dequeue().value(), 1);
  EXPECT_EQ(queue.dequeue().value(), 2);
  EXPECT_EQ(queue.dequeue().value(), 3);
  EXPECT_EQ(queue.dequeue().value(), 4);
  EXPECT_EQ(queue.dequeue().value(), 5);
  EXPECT_FALSE(queue.dequeue().has_value());
}

TEST(StdQueueWrapperTest, EnqueueBulkLvalue) {
  StdQueueWrapper<int> queue;
  std::vector<int> values = {10, 20, 30};

  queue.enqueue_bulk(values);  // lvalue reference

  EXPECT_EQ(queue.dequeue().value(), 10);
  EXPECT_EQ(queue.dequeue().value(), 20);
  EXPECT_EQ(queue.dequeue().value(), 30);
}

TEST(StdQueueWrapperTest, DequeueBulk) {
  StdQueueWrapper<int> queue;
  std::vector<int> values = {1, 2, 3, 4, 5};
  queue.enqueue_bulk(std::move(values));

  auto result = queue.dequeue_bulk(3);

  EXPECT_EQ(result.size(), 3);
  EXPECT_EQ(result[0], 1);
  EXPECT_EQ(result[1], 2);
  EXPECT_EQ(result[2], 3);
}

TEST(StdQueueWrapperTest, DequeueBulkMoreThanAvailable) {
  StdQueueWrapper<int> queue;
  queue.enqueue(1);
  queue.enqueue(2);

  auto result = queue.dequeue_bulk(10);  // Request more than available

  EXPECT_EQ(result.size(), 2);
  EXPECT_EQ(result[0], 1);
  EXPECT_EQ(result[1], 2);
}

TEST(StdQueueWrapperTest, DequeueBulkEmpty) {
  StdQueueWrapper<int> queue;

  auto result = queue.dequeue_bulk(5);

  EXPECT_TRUE(result.empty());
}

TEST(StdQueueWrapperTest, EnqueueDequeueStrings) {
  StdQueueWrapper<std::string> queue;

  queue.enqueue(std::string("hello"));
  queue.enqueue(std::string("world"));

  EXPECT_EQ(queue.dequeue().value(), "hello");
  EXPECT_EQ(queue.dequeue().value(), "world");
}

} // namespace std_queue_wrapper_tests

// ============================================================================
// MoodyCamelConcurrentQueueWrapper Tests
// ============================================================================

namespace moody_camel_concurrent_queue_wrapper_tests {

TEST(MoodyCamelConcurrentQueueWrapperTest, EnqueueDequeueSingleElement) {
  MoodyCamelConcurrentQueueWrapper<int> queue;

  queue.enqueue(42);
  auto result = queue.dequeue();

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 42);
}

TEST(MoodyCamelConcurrentQueueWrapperTest, DequeueEmptyReturnsNullopt) {
  MoodyCamelConcurrentQueueWrapper<int> queue;

  auto result = queue.dequeue();

  EXPECT_FALSE(result.has_value());
}

TEST(MoodyCamelConcurrentQueueWrapperTest, EnqueueDequeueMultipleElements) {
  MoodyCamelConcurrentQueueWrapper<int> queue;

  queue.enqueue(1);
  queue.enqueue(2);
  queue.enqueue(3);

  EXPECT_EQ(queue.dequeue().value(), 1);
  EXPECT_EQ(queue.dequeue().value(), 2);
  EXPECT_EQ(queue.dequeue().value(), 3);
  EXPECT_FALSE(queue.dequeue().has_value());
}

TEST(MoodyCamelConcurrentQueueWrapperTest, EnqueueDequeueLvalueReference) {
  MoodyCamelConcurrentQueueWrapper<int> queue;
  int value = 100;

  queue.enqueue(value);  // lvalue
  auto result = queue.dequeue();

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 100);
}

TEST(MoodyCamelConcurrentQueueWrapperTest, EnqueueDequeueRvalueReference) {
  MoodyCamelConcurrentQueueWrapper<int> queue;

  queue.enqueue(std::move(200));  // rvalue
  auto result = queue.dequeue();

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 200);
}

TEST(MoodyCamelConcurrentQueueWrapperTest, EnqueueBulk) {
  MoodyCamelConcurrentQueueWrapper<int> queue;
  std::vector<int> values = {1, 2, 3, 4, 5};

  queue.enqueue_bulk(std::move(values));

  EXPECT_EQ(queue.dequeue().value(), 1);
  EXPECT_EQ(queue.dequeue().value(), 2);
  EXPECT_EQ(queue.dequeue().value(), 3);
  EXPECT_EQ(queue.dequeue().value(), 4);
  EXPECT_EQ(queue.dequeue().value(), 5);
  EXPECT_FALSE(queue.dequeue().has_value());
}

TEST(MoodyCamelConcurrentQueueWrapperTest, EnqueueBulkLvalue) {
  MoodyCamelConcurrentQueueWrapper<int> queue;
  std::vector<int> values = {10, 20, 30};

  queue.enqueue_bulk(values);  // lvalue reference

  EXPECT_EQ(queue.dequeue().value(), 10);
  EXPECT_EQ(queue.dequeue().value(), 20);
  EXPECT_EQ(queue.dequeue().value(), 30);
}

TEST(MoodyCamelConcurrentQueueWrapperTest, DequeueBulk) {
  MoodyCamelConcurrentQueueWrapper<int> queue;
  std::vector<int> values = {1, 2, 3, 4, 5};
  queue.enqueue_bulk(std::move(values));

  auto result = queue.dequeue_bulk(3);

  EXPECT_EQ(result.size(), 3);
  EXPECT_EQ(result[0], 1);
  EXPECT_EQ(result[1], 2);
  EXPECT_EQ(result[2], 3);
}

TEST(MoodyCamelConcurrentQueueWrapperTest, DequeueBulkMoreThanAvailable) {
  MoodyCamelConcurrentQueueWrapper<int> queue;
  queue.enqueue(1);
  queue.enqueue(2);

  auto result = queue.dequeue_bulk(10);  // Request more than available

  EXPECT_EQ(result.size(), 2);
  EXPECT_EQ(result[0], 1);
  EXPECT_EQ(result[1], 2);
}

TEST(MoodyCamelConcurrentQueueWrapperTest, DequeueBulkEmpty) {
  MoodyCamelConcurrentQueueWrapper<int> queue;

  auto result = queue.dequeue_bulk(5);

  EXPECT_TRUE(result.empty());
}

TEST(MoodyCamelConcurrentQueueWrapperTest, EnqueueDequeueStrings) {
  MoodyCamelConcurrentQueueWrapper<std::string> queue;

  queue.enqueue(std::string("hello"));
  queue.enqueue(std::string("world"));

  EXPECT_EQ(queue.dequeue().value(), "hello");
  EXPECT_EQ(queue.dequeue().value(), "world");
}

} // namespace moody_camel_concurrent_queue_wrapper_tests

// ============================================================================
// MultiQueueContainers Tests with StdQueueWrapper
// ============================================================================

namespace multi_queue_containers_std_tests {

TEST(MultiQueueContainersStdTest, ConstructorCreatesCorrectNumberOfQueues) {
  MultiQueueContainers<StdQueueWrapper<int>> container(3);
  // No exception means success
}

TEST(MultiQueueContainersStdTest, EnqueueDequeueSingleQueue) {
  MultiQueueContainers<StdQueueWrapper<int>> container(2);

  container.enqueue(0, 42);
  auto result = container.dequeue(0);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 42);
}

TEST(MultiQueueContainersStdTest, MultipleQueuesAreIndependent) {
  MultiQueueContainers<StdQueueWrapper<int>> container(3);

  container.enqueue(0, 100);
  container.enqueue(1, 200);
  container.enqueue(2, 300);

  EXPECT_EQ(container.dequeue(0).value(), 100);
  EXPECT_EQ(container.dequeue(1).value(), 200);
  EXPECT_EQ(container.dequeue(2).value(), 300);
}

TEST(MultiQueueContainersStdTest, EnqueueDequeueLvalue) {
  MultiQueueContainers<StdQueueWrapper<int>> container(2);
  int value = 500;

  container.enqueue(0, value);  // lvalue
  auto result = container.dequeue(0);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 500);
}

TEST(MultiQueueContainersStdTest, EnqueueDequeueRvalue) {
  MultiQueueContainers<StdQueueWrapper<int>> container(2);

  container.enqueue(0, std::move(600));  // rvalue
  auto result = container.dequeue(0);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 600);
}

TEST(MultiQueueContainersStdTest, EnqueueBulk) {
  MultiQueueContainers<StdQueueWrapper<int>> container(2);
  std::vector<int> values = {1, 2, 3};

  container.enqueue_bulk(0, std::move(values));

  EXPECT_EQ(container.dequeue(0).value(), 1);
  EXPECT_EQ(container.dequeue(0).value(), 2);
  EXPECT_EQ(container.dequeue(0).value(), 3);
}

TEST(MultiQueueContainersStdTest, EnqueueBulkLvalue) {
  MultiQueueContainers<StdQueueWrapper<int>> container(2);
  std::vector<int> values = {10, 20, 30};

  container.enqueue_bulk(0, values);  // lvalue

  EXPECT_EQ(container.dequeue(0).value(), 10);
  EXPECT_EQ(container.dequeue(0).value(), 20);
  EXPECT_EQ(container.dequeue(0).value(), 30);
}

TEST(MultiQueueContainersStdTest, DequeueBulk) {
  MultiQueueContainers<StdQueueWrapper<int>> container(2);
  std::vector<int> values = {1, 2, 3, 4, 5};
  container.enqueue_bulk(0, std::move(values));

  auto result = container.dequeue_bulk(0, 3);

  EXPECT_EQ(result.size(), 3);
  EXPECT_EQ(result[0], 1);
  EXPECT_EQ(result[1], 2);
  EXPECT_EQ(result[2], 3);
}

TEST(MultiQueueContainersStdTest, DequeueBulkMoreThanAvailable) {
  MultiQueueContainers<StdQueueWrapper<int>> container(2);
  container.enqueue(0, 1);
  container.enqueue(0, 2);

  auto result = container.dequeue_bulk(0, 10);

  EXPECT_EQ(result.size(), 2);
  EXPECT_EQ(result[0], 1);
  EXPECT_EQ(result[1], 2);
}

TEST(MultiQueueContainersStdTest, InvalidQueueIndexThrowsEnqueue) {
  MultiQueueContainers<StdQueueWrapper<int>> container(2);

  EXPECT_THROW(container.enqueue(5, 42), std::invalid_argument);
}

TEST(MultiQueueContainersStdTest, InvalidQueueIndexThrowsDequeue) {
  MultiQueueContainers<StdQueueWrapper<int>> container(2);

  EXPECT_THROW(container.dequeue(5), std::invalid_argument);
}

TEST(MultiQueueContainersStdTest, InvalidQueueIndexThrowsEnqueueBulk) {
  MultiQueueContainers<StdQueueWrapper<int>> container(2);
  std::vector<int> values = {1, 2, 3};

  EXPECT_THROW(container.enqueue_bulk(5, std::move(values)), std::invalid_argument);
}

TEST(MultiQueueContainersStdTest, InvalidQueueIndexThrowsDequeueBulk) {
  MultiQueueContainers<StdQueueWrapper<int>> container(2);

  EXPECT_THROW(container.dequeue_bulk(5, 10), std::invalid_argument);
}

TEST(MultiQueueContainersStdTest, StringElements) {
  MultiQueueContainers<StdQueueWrapper<std::string>> container(2);

  container.enqueue(0, std::string("hello"));
  container.enqueue(1, std::string("world"));

  EXPECT_EQ(container.dequeue(0).value(), "hello");
  EXPECT_EQ(container.dequeue(1).value(), "world");
}

} // namespace multi_queue_containers_std_tests

// ============================================================================
// MultiQueueContainers Tests with MoodyCamelConcurrentQueueWrapper
// ============================================================================

namespace multi_queue_containers_moody_camel_tests {

TEST(MultiQueueContainersMoodyCamelTest, ConstructorCreatesCorrectNumberOfQueues) {
  MultiQueueContainers<MoodyCamelConcurrentQueueWrapper<int>> container(3);
  // No exception means success
}

TEST(MultiQueueContainersMoodyCamelTest, EnqueueDequeueSingleQueue) {
  MultiQueueContainers<MoodyCamelConcurrentQueueWrapper<int>> container(2);

  container.enqueue(0, 42);
  auto result = container.dequeue(0);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 42);
}

TEST(MultiQueueContainersMoodyCamelTest, MultipleQueuesAreIndependent) {
  MultiQueueContainers<MoodyCamelConcurrentQueueWrapper<int>> container(3);

  container.enqueue(0, 100);
  container.enqueue(1, 200);
  container.enqueue(2, 300);

  EXPECT_EQ(container.dequeue(0).value(), 100);
  EXPECT_EQ(container.dequeue(1).value(), 200);
  EXPECT_EQ(container.dequeue(2).value(), 300);
}

TEST(MultiQueueContainersMoodyCamelTest, EnqueueDequeueLvalue) {
  MultiQueueContainers<MoodyCamelConcurrentQueueWrapper<int>> container(2);
  int value = 500;

  container.enqueue(0, value);  // lvalue
  auto result = container.dequeue(0);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 500);
}

TEST(MultiQueueContainersMoodyCamelTest, EnqueueDequeueRvalue) {
  MultiQueueContainers<MoodyCamelConcurrentQueueWrapper<int>> container(2);

  container.enqueue(0, std::move(600));  // rvalue
  auto result = container.dequeue(0);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 600);
}

TEST(MultiQueueContainersMoodyCamelTest, EnqueueBulk) {
  MultiQueueContainers<MoodyCamelConcurrentQueueWrapper<int>> container(2);
  std::vector<int> values = {1, 2, 3};

  container.enqueue_bulk(0, std::move(values));

  EXPECT_EQ(container.dequeue(0).value(), 1);
  EXPECT_EQ(container.dequeue(0).value(), 2);
  EXPECT_EQ(container.dequeue(0).value(), 3);
}

TEST(MultiQueueContainersMoodyCamelTest, EnqueueBulkLvalue) {
  MultiQueueContainers<MoodyCamelConcurrentQueueWrapper<int>> container(2);
  std::vector<int> values = {10, 20, 30};

  container.enqueue_bulk(0, values);  // lvalue

  EXPECT_EQ(container.dequeue(0).value(), 10);
  EXPECT_EQ(container.dequeue(0).value(), 20);
  EXPECT_EQ(container.dequeue(0).value(), 30);
}

TEST(MultiQueueContainersMoodyCamelTest, DequeueBulk) {
  MultiQueueContainers<MoodyCamelConcurrentQueueWrapper<int>> container(2);
  std::vector<int> values = {1, 2, 3, 4, 5};
  container.enqueue_bulk(0, std::move(values));

  auto result = container.dequeue_bulk(0, 3);

  EXPECT_EQ(result.size(), 3);
  EXPECT_EQ(result[0], 1);
  EXPECT_EQ(result[1], 2);
  EXPECT_EQ(result[2], 3);
}

TEST(MultiQueueContainersMoodyCamelTest, DequeueBulkMoreThanAvailable) {
  MultiQueueContainers<MoodyCamelConcurrentQueueWrapper<int>> container(2);
  container.enqueue(0, 1);
  container.enqueue(0, 2);

  auto result = container.dequeue_bulk(0, 10);

  EXPECT_EQ(result.size(), 2);
  EXPECT_EQ(result[0], 1);
  EXPECT_EQ(result[1], 2);
}

TEST(MultiQueueContainersMoodyCamelTest, InvalidQueueIndexThrowsEnqueue) {
  MultiQueueContainers<MoodyCamelConcurrentQueueWrapper<int>> container(2);

  EXPECT_THROW(container.enqueue(5, 42), std::invalid_argument);
}

TEST(MultiQueueContainersMoodyCamelTest, InvalidQueueIndexThrowsDequeue) {
  MultiQueueContainers<MoodyCamelConcurrentQueueWrapper<int>> container(2);

  EXPECT_THROW(container.dequeue(5), std::invalid_argument);
}

TEST(MultiQueueContainersMoodyCamelTest, InvalidQueueIndexThrowsEnqueueBulk) {
  MultiQueueContainers<MoodyCamelConcurrentQueueWrapper<int>> container(2);
  std::vector<int> values = {1, 2, 3};

  EXPECT_THROW(container.enqueue_bulk(5, std::move(values)), std::invalid_argument);
}

TEST(MultiQueueContainersMoodyCamelTest, InvalidQueueIndexThrowsDequeueBulk) {
  MultiQueueContainers<MoodyCamelConcurrentQueueWrapper<int>> container(2);

  EXPECT_THROW(container.dequeue_bulk(5, 10), std::invalid_argument);
}

TEST(MultiQueueContainersMoodyCamelTest, StringElements) {
  MultiQueueContainers<MoodyCamelConcurrentQueueWrapper<std::string>> container(2);

  container.enqueue(0, std::string("hello"));
  container.enqueue(1, std::string("world"));

  EXPECT_EQ(container.dequeue(0).value(), "hello");
  EXPECT_EQ(container.dequeue(1).value(), "world");
}

} // namespace multi_queue_containers_moody_camel_tests
