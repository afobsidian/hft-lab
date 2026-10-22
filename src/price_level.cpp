#include "hft/price_level.hpp"
#include <stdexcept>

namespace hft {

void PriceLevel::add(const Order &) {
    // TODO(exercise 1): validate, then append an owning copy.
    throw std::logic_error("PriceLevel::add not implemented; see docs/first-exercise.md");
}

const Order *PriceLevel::front() const {
    // TODO(exercise 1): empty or oldest order?
    throw std::logic_error("PriceLevel::front not implemented");
}

Quantity PriceLevel::fill_front(Quantity) {
    // TODO(exercise 1): partial fill or remove the oldest order.
    throw std::logic_error("PriceLevel::fill_front not implemented");
}

bool PriceLevel::cancel(OrderId) {
    // TODO(exercise 1): find, erase and report whether the order existed.
    throw std::logic_error("PriceLevel::cancel not implemented");
}

} // namespace hft
