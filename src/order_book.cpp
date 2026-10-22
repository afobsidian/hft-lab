#include "hft/order_book.hpp"
#include <stdexcept>

namespace hft {

Result OrderBook::apply(const Command &) {
    // TODO(stage 2): own the bid/ask levels and match at the resting order's price.
    throw std::logic_error("OrderBook::apply not implemented; start with docs/first-exercise.md");
}

} // namespace hft
