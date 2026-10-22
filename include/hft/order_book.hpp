#pragma once

#include "hft/types.hpp"

namespace hft {

class OrderBook {
  public:
    // Stage 2: validate and match a command; return events in processing order.
    Result apply(const Command &command);
};

} // namespace hft
