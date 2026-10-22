# HFT Lab

A C++20 learning project: build a deterministic exchange matching engine, measure it,
improve its storage and concurrency, then connect a simulated trading client.

Project documentation: [delivery plan](docs/project-plan.md) and
[system architecture](docs/architecture.md). These describe the application independently
of the [learning roadmap](docs/learning-plan.md) and [exercises](docs/exercises.md).

**Status: exercise 1 complete; matching engine is exercise 2.** The FIFO price level passes
all eight supplied scenarios. CI runs the scaffold and completed price-level checks;
the order book remains unimplemented. There are no performance results yet.

## Start here

1. Follow the build steps below.
2. Open [the exercises](docs/exercises.md): review exercise 1's completed checklist and
   start exercise 2, the limit order book.
3. Follow the [learning roadmap](docs/learning-plan.md) for later stages.

The main loop lives in [src/main.cpp](src/main.cpp). Matching remains a stage 2 TODO;
completing the price-level exercise alone will not make the demo match orders.

## Toolchain

Clang with C++20 support, CMake 3.21+, Ninja and the system C++ standard library.
No third-party C++ libraries are required. The sanitizer build also needs compiler-rt.

```sh
# Fedora
sudo dnf install clang compiler-rt clang-tools-extra cmake ninja-build gcc-c++

# Ubuntu 24.04
sudo apt-get update
sudo apt-get install clang libclang-rt-dev clang-tools cmake ninja-build g++
```

The presets select `clang++`; they use the system standard library rather than requiring
libc++. Run `clang-format -i` only on files you intend to format.

## Build and run

With GNU Make installed, the shortcuts below wrap the same CMake presets used by CI:

```sh
make                           # Debug build
make scaffold                  # Build and run scaffold checks
make exercise                  # Build and run the registered exercise checks
make test                      # All registered checks (also used by CI)
make test PRESET=sanitize      # Build and run all checks with sanitizers
make build PRESET=release      # Optimised build
make run                       # Run the demo; initially reports unimplemented matching
make clean                     # Clean the selected preset's configured build
```

`PRESET` defaults to `debug`; use `debug`, `sanitize` or `release`. `make clean` requires
that preset to have been configured. To run CMake directly:

```sh
cmake --preset debug
cmake --build --preset debug
ctest --preset debug -L scaffold
./build/debug/hft_lab --help

# Initially fails with a clear OrderBook::apply "not implemented" message (exit 1).
./build/debug/hft_lab --demo

# Run the registered exercise checks (PriceLevel is complete).
ctest --preset debug -L exercise
```

Use the same commands with `sanitize` or `release` in place of `debug`. The sanitizer
preset enables AddressSanitizer and UndefinedBehaviorSanitizer. The Release preset has
optimisation enabled and sanitizers disabled. Test checks remain active in Release.

Plain `ctest --preset debug` runs **both** labels. CI runs `make test` for every preset;
new exercise tests participate once you register them in CMake.

## Main-loop sketch

```cpp
OrderBook book;
for (const Command& command : sample_orders) {
    // Stage 2: validate before mutation, match, then return ordered events.
    Result result = book.apply(command);
    for (const Event& event : result.events) {
        print(event);
    }
}
```

The fixed, typed sample avoids making a text parser your first task. Later stages replace
the sample with deterministic replay input and eventually a bounded SPSC queue. One engine
thread always owns the book. Console output is a teaching aid, not part of a benchmark.

Stage 2's sample is: sell 10 at 100, sell 5 at 100, buy 12 with limit 101, cancel the
second sell. Expected trades: 10 against order 1, then 2 against order 2, both at 100;
cancellation removes order 2's remaining 3. No orders remain.

## Project boundaries

One synthetic instrument, integer price ticks, positive integer quantities and FIFO
priority at each price. This is an exchange simulator first; trading strategies arrive
later. It does not connect to an exchange, place real orders or claim production latency.

MIT licensed. The learning plan adapts the original *HFT Portfolio Project Guide* by
making correctness and measurement explicit before custom storage and concurrency.
