#pragma once

#include "hft/types.hpp"

namespace hft {

class OrderBook {
  public:
    // Apply one limit order or cancellation. Expected invalid input returns Rejected
    // without changing state; a valid order returns Accepted followed by trades.
    // Choose best price, then FIFO; trades execute at the resting order's price.
    // The implementation stub contains the complete exercise 2 checklist.
    Result apply(const Command &command);
};

} // namespace hft
