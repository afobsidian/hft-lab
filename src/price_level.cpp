#include "hft/price_level.hpp"
#include <stdexcept>
#include <type_traits>
#include <format>

namespace hft {

void PriceLevel::add(const Order &order) {
    if (order.id == 0 || order.price <= 0 || order.quantity <= 0 || order.side != side_ || order.price != price_)
    {
        throw std::invalid_argument("Invalid order for this Price Level");
    }

    for (auto it = orders_.begin(); it != orders_.end(); ++it)
    {
        if (it->id == order.id)
        {
            throw std::invalid_argument(std::format("Order with id: {} already exists", order.id));
        }
    }

    orders_.push_back(order);
}

const Order *PriceLevel::front() const {
    if (orders_.empty())
    {
        return nullptr;
    }
    return &orders_.front();
}

Quantity PriceLevel::fill_front(Quantity quantity) {
    if (orders_.empty() || quantity <= 0 )
    {
        return 0;
    }
    Order &order = orders_.front();
    if (order.quantity <= quantity)
    {
        Quantity partial_quantity = order.quantity;
        orders_.pop_front();
        return partial_quantity;
    } else {
        order.quantity -= quantity;
        return quantity;
    }

}

bool PriceLevel::cancel(OrderId id) {
    for (auto it = orders_.begin(); it != orders_.end(); ++it)
    {
        if (it->id == id)
        {
            orders_.erase(it);
            return true;
        }
    }
    return false;
}

} // namespace hft
