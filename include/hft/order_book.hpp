#pragma once

#include "hft/types.hpp"

namespace hft {

class OrderBook {
  public:
    // Apply one command; return ordered events. Contract: docs/exercises.md, exercise 2.
    Result apply(const Command &command);
};

} // namespace hft
