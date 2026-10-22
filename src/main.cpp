#include "hft/order_book.hpp"
#include <array>
#include <exception>
#include <iostream>
#include <string_view>
#include <type_traits>

namespace {

void print(const hft::Event &event) {
    std::visit(
        [](const auto &value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, hft::Accepted>) {
                std::cout << "ACCEPT " << value.id << '\n';
            } else if constexpr (std::is_same_v<T, hft::Rejected>) {
                std::cout << "REJECT " << value.id << ' ' << value.reason << '\n';
            } else if constexpr (std::is_same_v<T, hft::Cancelled>) {
                std::cout << "CANCEL " << value.id << '\n';
            } else {
                std::cout << "TRADE maker=" << value.maker_id << " taker=" << value.taker_id
                          << " price=" << value.price << " qty=" << value.quantity << '\n';
            }
        },
        event);
}

int demo() {
    using namespace hft;
    const std::array<Command, 4> commands{
        Order{1, Side::Sell, 100, 10},
        Order{2, Side::Sell, 100, 5},
        Order{3, Side::Buy, 101, 12},
        Cancel{2},
    };

    OrderBook book;
    for (const Command &command : commands) {
        const Result result = book.apply(command);
        for (const Event &event : result.events) {
            print(event);
        }
    }
    return 0;
}

} // namespace

int main(int argc, char **argv) {
    const std::string_view mode = argc == 2 ? argv[1] : "";
    if (mode == "--help") {
        std::cout << "Usage: hft_lab --help | --demo\n"
                     "C++20 HFT learning scaffold. Matching is a stage 2 exercise.\n";
        return 0;
    }
    if (mode != "--demo") {
        std::cerr << "Usage: hft_lab --help | --demo\n";
        return 2;
    }
    try {
        return demo();
    } catch (const std::exception &error) {
        std::cerr << "Demo stopped: " << error.what() << '\n';
        return 1;
    }
}
