#include "hft/order_book.hpp"
#include <stdexcept>

namespace hft {

Result OrderBook::apply(const Command &) {
    // EXERCISE 2: implement one command against this book; return Result{events}.
    // Name the parameter `command` when implementing. Command is a std::variant:
    // either Order{id, side, price, quantity} or Cancel{id}. Use std::visit or
    // std::get_if to distinguish them. Matching is single-threaded here.
    //
    // STATE TO ADD IN order_book.hpp:
    // - Separate std::map<Price, PriceLevel> containers for bids and asks.
    //   With ascending keys, begin() is the lowest ask; rbegin() the highest bid.
    // - A book-wide active-ID index: ID -> side and price of its resting level.
    // - Public best_bid()/best_ask() returning std::optional<Price>; nullopt if empty.
    // Use try_emplace(price, side, price) to construct a missing PriceLevel;
    // map::operator[] needs a default constructor, which PriceLevel does not have.
    //
    // LIMIT ORDER:
    // 1. Before mutation, reject ID == 0, price <= 0, quantity == 0, an invalid
    //    Side value, or an ID already resting ANYWHERE (including the other side).
    //    Return one Rejected{id, reason}; expected invalid input must not throw.
    // 2. Append Accepted{id}. Work with a local copy of the incoming order so
    //    you can reduce its remaining quantity without changing the caller's data.
    // 3. A buy matches the lowest ask while ask <= buy limit; a sell matches the
    //    highest bid while bid >= sell limit. Check the opposite map is not empty.
    // 4. Take that level's front() order. Save its ID, price and quantity BEFORE
    //    fill_front(): a full fill destroys it, invalidating the borrowed pointer.
    //    Consume min(incoming remaining, resting remaining), subtract the amount
    //    from incoming remaining, and append Trade{maker_id, taker_id, price, qty}.
    //    Maker = resting ID; taker = incoming ID; price = RESTING price.
    // 5. Remove fully filled makers from the ID index. Erase an empty level and
    //    obtain a fresh best-level iterator; do not reuse an erased iterator.
    //    Repeat until incoming quantity is zero or the next price is ineligible.
    // 6. If quantity remains, append it to its OWN side at its ORIGINAL limit
    //    price and index its ID. Otherwise the incoming ID must not remain active.
    //
    // CANCELLATION:
    // Look up the ID globally. If absent, return only Rejected{id, reason}.
    // Otherwise call that level's cancel(id), remove the ID index entry and any
    // empty level, and return only Cancelled{id}. Surviving orders keep FIFO order.
    // IDs may be reused after cancellation or complete execution.
    //
    // AFTER EVERY COMMAND: no empty levels, no zero-quantity resting orders,
    // index matches resting IDs, and best_bid < best_ask when both sides exist.
    // Return events in processing order; no console output or wall-clock priority.
    // Keep market orders, IOC/FOK, threads and custom allocation out of this task.
    // Allocation failure may stop this baseline; recovery is not required here.
    //
    // DEMO CHECK: sell #1 at 100 x10; sell #2 at 100 x5; buy #3 at 101 x12.
    // The buy returns Accepted{3}, Trade{1,3,100,10}, Trade{2,3,100,2}.
    // Only sell #2 remains, at 100 x3. Cancelling #2 leaves both sides empty.
    // Add tests in tests/order_book_tests.cpp and register with CTest's exercise
    // label. Run make test, then PRESET=sanitize and PRESET=release.
    throw std::logic_error("OrderBook::apply not implemented; see exercise 2 in docs/exercises.md");
}

} // namespace hft
