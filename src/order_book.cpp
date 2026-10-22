#include "hft/order_book.hpp"
#include <stdexcept>

namespace hft {

Result OrderBook::apply(const Command &) {
    // Invalid limit orders return one Rejected event without changing the book:
    // zero ID/quantity, nonpositive price, invalid side or an already-active ID.

    // Valid limit orders produce Accepted before any Trade events.

    // Buys match the lowest ask at or below their limit; sells match the highest
    // bid at or above their limit. Orders at the same price fill oldest first.

    // Each trade consumes the smaller remaining quantity and executes at the
    // resting price. Its maker ID is the resting order; its taker ID is incoming.

    // Partially filled resting orders keep their FIFO position. Fully filled
    // orders leave the active-ID index; empty price levels are removed.

    // Matching continues until the incoming quantity is filled or no eligible
    // price remains. Any remainder rests on its own side at its original limit.

    // Cancelling an active ID removes it from its level and index and returns
    // only Cancelled. An unknown ID returns only Rejected without changing state.
    // Empty levels are removed; IDs may be reused after cancellation or full fill.

    // Returned events preserve processing order; no zero-quantity orders remain.
    throw std::logic_error("OrderBook::apply not implemented; see exercise 2 in docs/exercises.md");
}

} // namespace hft
