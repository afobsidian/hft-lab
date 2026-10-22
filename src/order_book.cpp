#include "hft/order_book.hpp"
#include <stdexcept>

namespace hft {

Result OrderBook::apply(const Command &command) {

    Result result = {};
    if (const auto* order = std::get_if<Order>(&command))
    {
        // Invalid limit orders return one Rejected event without changing the book:
        // zero ID/quantity, nonpositive price, invalid side or an already-active ID.
        const bool invalid_side =
            order->side != Side::Buy && order->side != Side::Sell;
        if(order->id == 0 || order->quantity <= 0 || order->price <= 0 || invalid_side)
        {
            result.events.push_back(Rejected{order->id, "Rejected due to invalid order"});
            return result;
        }

        if(active_orders_.contains(order->id))
        {
            result.events.push_back(Rejected{order->id, "Order ID is already active"});
            return result;
        }

        // Valid limit orders produce Accepted before any Trade events.
        result.events.push_back(Accepted{order->id});

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
    }
}

} // namespace hft
