# HFT Lab

A C++20 learning project: build a deterministic exchange matching engine, measure it,
improve its storage and concurrency, then connect a simulated trading client.

**Status: runnable scaffold, not a working matching engine.** The first exercise is
deliberately unimplemented. Green CI means the scaffold builds and its CLI checks pass;
it does not mean the order book or exercises work. There are no performance results yet.

## Start here

1. Follow the build steps below.
2. Open [your first exercise](docs/first-exercise.md): implement one FIFO price level.
3. Follow the [learning roadmap](docs/learning-plan.md) after its eight scenarios pass.

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
libc++. `clangd` can use `build/debug/compile_commands.json` (configure its compile commands
directory in your editor). Run `clang-format -i` only on files you intend to format.

## Build and run

```sh
cmake --preset debug
cmake --build --preset debug
ctest --preset debug -L scaffold
./build/debug/hft_lab --help

# Initially fails with a clear OrderBook::apply "not implemented" message (exit 1).
./build/debug/hft_lab --demo

# Initially fails: this is your implementation assignment.
ctest --preset debug -L exercise
```

Use the same commands with `sanitize` or `release` in place of `debug`. The sanitizer
preset enables AddressSanitizer and UndefinedBehaviorSanitizer. The Release preset has
optimisation enabled and sanitizers disabled. Test checks remain active in Release.

Plain `ctest --preset debug` runs **both** labels, so it initially fails. CI deliberately
runs only `-L scaffold`; once exercise 1 passes, change CI to run both labels.

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
