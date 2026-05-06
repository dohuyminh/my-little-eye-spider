#pragma once

#include <moodycamel/concurrentqueue.h>

#include <condition_variable>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <tuple>

#include "i_back_router.h"
#include "i_back_selector.h"
#include "i_front_prioritizer.h"
#include "i_front_selector.h"
#include "types/runnable.h"

namespace crawler {

namespace components {

template <typename T, typename Tuple>
concept TupleInvocableUpdate = []<std::size_t... I>(std::index_sequence<I...>) {
  return requires(T& t, Tuple pkt) {
    { t.update(std::get<I>(pkt)...) } -> std::same_as<void>;
  };
}(std::make_index_sequence<std::tuple_size_v<Tuple>>{});

template <typename T>
concept StatefulFrontierComponent = requires {
  typename T::StateUpdatePacket;
} && TupleInvocableUpdate<T, typename T::StateUpdatePacket>;

template <typename T, bool IsStateful = StatefulFrontierComponent<T>>
struct PacketTypeImpl {
  using type = std::tuple<>;
};

template <typename T>
struct PacketTypeImpl<T, true> {
  using type = typename T::StateUpdatePacket;
};

template <typename T>
using PacketType = typename PacketTypeImpl<T>::type;

template <FrontPrioritizerType FPType, FrontSelectorType FSType,
          BackRouterType BRType, BackSelectorType BSType>
class UpdateQueueBus : public types::Runnable {
 public:
  using FPPacketType = PacketType<FPType>;
  using FSPacketType = PacketType<FSType>;
  using BRPacketType = PacketType<BRType>;
  using BSPacketType = PacketType<BSType>;
  using UpdatePacketType =
      std::tuple<FPPacketType, FSPacketType, BRPacketType, BSPacketType>;

  UpdateQueueBus(std::shared_ptr<FPType> frontPrioritizer,
                 std::shared_ptr<FSType> frontSelector,
                 std::shared_ptr<BRType> backRouter,
                 std::shared_ptr<BSType> backSelector,
                 std::size_t updateBatchSize)
      : frontPrioritizer_(frontPrioritizer),
        frontSelector_(frontSelector),
        backRouter_(backRouter),
        backSelector_(backSelector),
        updateBatchSize_(updateBatchSize) {
    if (updateBatchSize_ == 0) {
      throw std::invalid_argument(
          "Frontier Update Bus: Batch size must be a positive integer");
    }
  }

  void runImpl() override {
    std::vector<UpdatePacketType> pkts;

    {
      std::unique_lock<std::mutex> lock(updateMutex_);
      updateCV_.wait(lock, [&]() { return !isRunning() || canUpdate(); });

      if (!isRunning()) {
        return;
      }

      updateBus_.try_dequeue_bulk(std::back_inserter(pkts), updateBatchSize_);
    }

    for (const auto& [fpUpdateArgs, fsUpdateArgs, brUpdateArgs, bsUpdateArgs] :
         pkts) {
      if constexpr (StatefulFrontierComponent<FPType>) {
        std::apply(
            [&](auto&&... args) {
              frontPrioritizer_->update(std::forward<decltype(args)>(args)...);
            },
            fpUpdateArgs);
      }

      if constexpr (StatefulFrontierComponent<FSType>) {
        std::apply(
            [&](auto&&... args) {
              frontSelector_->update(std::forward<decltype(args)>(args)...);
            },
            fsUpdateArgs);
      }

      if constexpr (StatefulFrontierComponent<BRType>) {
        std::apply(
            [&](auto&&... args) {
              backRouter_->update(std::forward<decltype(args)>(args)...);
            },
            brUpdateArgs);
      }

      if constexpr (StatefulFrontierComponent<BSType>) {
        std::apply(
            [&](auto&&... args) {
              backSelector_->update(std::forward<decltype(args)>(args)...);
            },
            bsUpdateArgs);
      }
    }
  }

  virtual void insertUpdatePacket(const UpdatePacketType& pkt) {
    updateBus_.enqueue(pkt);
  }

  virtual bool canUpdate() = 0;

 private:
  moodycamel::ConcurrentQueue<UpdatePacketType> updateBus_;

  std::shared_ptr<FPType> frontPrioritizer_;
  std::shared_ptr<FSType> frontSelector_;
  std::shared_ptr<BRType> backRouter_;
  std::shared_ptr<BSType> backSelector_;

  std::size_t updateBatchSize_{1};

  std::mutex updateMutex_;
  std::condition_variable updateCV_;
};

}  // namespace components

}  // namespace crawler
