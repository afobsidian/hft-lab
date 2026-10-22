#include "hft/order_book.hpp"
#include <stdexcept>

namespace hft {

Result OrderBook::apply(const Command &) {
    // Exercise 2: process limit orders and cancellations using PriceLevel.
    // Add bid/ask maps, a global active-ID index and optional best_bid()/best_ask().
    // Reject before mutation; emit Accepted before trades for valid orders.
    // Match best price then FIFO at the resting price; rest any limit remainder.
    // Remove filled/cancelled IDs and empty levels; avoid invalidated pointers.
    // Register book tests in CMake; run make test in all three presets.
    throw std::logic_error("OrderBook::apply not implemented; see exercise 2 in docs/exercises.md");
}

} // namespace hft
