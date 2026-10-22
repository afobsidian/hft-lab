#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace hft {

using OrderId = std::uint64_t;
using Price = std::int64_t; // Integer ticks, not floating-point currency.
using Quantity = std::uint32_t;

enum class Side { Buy, Sell };

struct Order {
    OrderId id;
    Side side;
    Price price;
    Quantity quantity;
};

struct Cancel {
    OrderId id;
};
using Command = std::variant<Order, Cancel>;

struct Accepted {
    OrderId id;
};
struct Rejected {
    OrderId id;
    std::string reason;
};
struct Cancelled {
    OrderId id;
};
struct Trade {
    OrderId maker_id;
    OrderId taker_id;
    Price price;
    Quantity quantity;
};
using Event = std::variant<Accepted, Rejected, Cancelled, Trade>;
struct Result {
    std::vector<Event> events;
};

} // namespace hft
