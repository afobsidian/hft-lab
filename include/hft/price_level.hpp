#pragma once

#include "hft/types.hpp"
#include <list>

namespace hft {

class PriceLevel {
  public:
    PriceLevel(Side side, Price price) : side_(side), price_(price) {}

    // Reject invalid additions with std::invalid_argument; preserve the queue.
    void add(const Order &order);

    // Borrowed pointer: valid until that order is filled, cancelled or the level dies.
    const Order *front() const;

    // Consume only the oldest order, even when the requested quantity is larger.
    Quantity fill_front(Quantity quantity);
    bool cancel(OrderId id);

  private:
    Side side_; // Exercise 1 will use these for validation.
    Price price_;
    // shortcut: linear cancellation, add an ID index when profiling warrants it.
    std::list<Order> orders_;
};

} // namespace hft
