#pragma once

#include "hft/types.hpp"
#include <unordered_map>

namespace hft {

class OrderBook {
  public:
    // Apply one command; return ordered events. Contract: docs/exercises.md, exercise 2.
    Result apply(const Command &command);

  private:
    struct OrderLocation {
        Side side;
        Price price;
    };

    std::unordered_map<OrderId, OrderLocation> active_orders_;
};

} // namespace hft
