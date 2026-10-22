#include "hft/price_level.hpp"
#include <array>
#include <iostream>
#include <stdexcept>

namespace {

using namespace hft;

void expect(bool condition, const char *message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

Order buy(OrderId id, Quantity quantity = 10) { return {id, Side::Buy, 100, quantity}; }

void empty_operations() {
    PriceLevel level{Side::Buy, 100};
    expect(level.front() == nullptr, "new level must be empty");
    expect(level.fill_front(10) == 0, "empty fill must consume nothing");
    expect(!level.cancel(99), "unknown cancellation must return false");
}

void fifo_and_owned_copy() {
    PriceLevel level{Side::Buy, 100};
    auto order = buy(1);
    level.add(order);
    order.quantity = 99;
    level.add(buy(2, 5));
    expect(level.front() != nullptr, "first order must exist");
    expect(level.front()->id == 1, "oldest order must be first");
    expect(level.front()->quantity == 10, "level must own its order copy");
}

void partial_and_exact_fills() {
    PriceLevel level{Side::Buy, 100};
    level.add(buy(1));
    level.add(buy(2, 5));
    expect(level.fill_front(4) == 4, "partial fill must return consumed quantity");
    expect(level.front() != nullptr, "partially filled order must remain");
    expect(level.front()->id == 1 && level.front()->quantity == 6, "partial fill keeps priority");
    expect(level.fill_front(6) == 6, "exact fill must consume the remainder");
    expect(level.front() != nullptr && level.front()->id == 2, "next order must advance");
    expect(level.fill_front(5) == 5, "second order must fill");
    expect(level.front() == nullptr, "fully consumed level must be empty");
}

void oversized_fill_stops_at_front() {
    PriceLevel level{Side::Buy, 100};
    level.add(buy(1, 3));
    level.add(buy(2, 5));
    expect(level.fill_front(100) == 3, "fill_front must not sweep the next order");
    expect(level.front() != nullptr && level.front()->id == 2, "second order must remain");
    expect(level.front()->quantity == 5, "second order must be untouched");
}

void zero_fill() {
    PriceLevel level{Side::Buy, 100};
    level.add(buy(1));
    expect(level.fill_front(0) == 0, "zero fill must consume nothing");
    expect(level.front() != nullptr && level.front()->quantity == 10, "zero fill preserves order");
}

void cancellation_positions() {
    for (OrderId removed : {1, 2, 3}) {
        PriceLevel level{Side::Buy, 100};
        for (OrderId id : {1, 2, 3}) {
            level.add(buy(id));
        }
        expect(!level.cancel(99), "unknown ID must leave a populated level unchanged");
        expect(level.cancel(removed), "existing cancellation must succeed");
        expect(!level.cancel(removed), "repeated cancellation must fail");
        for (OrderId id : {1, 2, 3}) {
            if (id == removed) {
                continue;
            }
            expect(level.front() != nullptr && level.front()->id == id, "cancel preserves FIFO");
            expect(level.fill_front(10) == 10, "survivors retain their quantities");
        }
        expect(level.front() == nullptr, "all survivors must have been consumed");
    }
    PriceLevel level{Side::Buy, 100};
    level.add(buy(1));
    expect(level.cancel(1) && level.front() == nullptr, "cancelling the sole order empties level");
}

void invalid_additions_preserve_state() {
    const std::array invalid{
        buy(0),
        buy(2, 0),
        buy(1),
        Order{2, Side::Sell, 100, 10},
        Order{2, Side::Buy, 101, 10},
        Order{2, Side::Buy, 0, 10},
        Order{2, Side::Buy, -1, 10},
    };
    for (const auto &order : invalid) {
        PriceLevel level{Side::Buy, 100};
        level.add(buy(1));
        bool rejected = false;
        try {
            level.add(order);
        } catch (const std::invalid_argument &) {
            rejected = true;
        }
        expect(rejected, "invalid order must throw std::invalid_argument");
        expect(level.front() != nullptr && level.front()->id == 1, "invalid add preserves front");
        expect(level.fill_front(10) == 10, "invalid add preserves quantity");
        expect(level.front() == nullptr, "invalid add must not insert an order");
    }
}

void sell_level() {
    PriceLevel level{Side::Sell, 101};
    level.add({1, Side::Sell, 101, 7});
    expect(level.front() != nullptr && level.front()->side == Side::Sell, "sell order must rest");
    expect(level.fill_front(7) == 7 && level.front() == nullptr, "sell order must fill");
}

struct Case {
    const char *name;
    void (*run)();
};

} // namespace

int main() {
    const std::array cases{
        Case{"empty operations", empty_operations},
        Case{"FIFO and owned copy", fifo_and_owned_copy},
        Case{"partial and exact fills", partial_and_exact_fills},
        Case{"oversized fill", oversized_fill_stops_at_front},
        Case{"zero fill", zero_fill},
        Case{"cancellation positions", cancellation_positions},
        Case{"invalid additions", invalid_additions_preserve_state},
        Case{"sell level", sell_level},
    };
    int failures = 0;
    for (const auto &test : cases) {
        try {
            test.run();
            std::cout << "PASS " << test.name << '\n';
        } catch (const std::exception &error) {
            ++failures;
            std::cerr << "FAIL " << test.name << ": " << error.what() << '\n';
        }
    }
    std::cout << cases.size() - failures << '/' << cases.size() << " exercises passed\n";
    return failures == 0 ? 0 : 1;
}
