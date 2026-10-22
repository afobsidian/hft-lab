# Exercise 1: implement a FIFO price level

Implement the four TODO methods in `src/price_level.cpp`. Read
`include/hft/price_level.hpp` and `tests/price_level_tests.cpp` first.
The constructor and storage are already supplied; do not implement the matching engine yet.

A **price level** groups resting orders on one side at one price. FIFO means the oldest
order trades first. The full book will later choose the best price before consulting
that level. Completing this component teaches ownership, const access, iterators,
container invalidation, unsigned arithmetic and preserving state when validation fails.

## Contract

| Method | Required behaviour |
|---|---|
| `add(order)` | Copy the order to the back. Require nonzero ID, positive price and quantity, matching side/price, and an ID not already resting in this level. Otherwise throw `std::invalid_argument` without changing anything. |
| `front() const` | Return a borrowed pointer to the oldest order, or `nullptr` if empty. Do not return a pointer to a temporary or remove anything. |
| `fill_front(quantity)` | Return `min(requested, oldest.remaining)`. Subtract that amount from the oldest order; erase it only when exhausted. Empty level or zero request returns zero. Never continue into the next order. |
| `cancel(id)` | Remove the matching order, return true; otherwise return false without changes. Surviving orders keep their relative order. |

Only valid `Side` enum values are used in this exercise. Global ID uniqueness belongs to
the book in stage 2; this component checks only its own resting orders. An ID may be reused
after its previous order is removed. No references into the caller's order are retained.

`front()` pointers remain valid until that particular order is removed or the level is
destroyed. Its pointed-to quantity can change after a partial fill. Never read a pointer
after cancelling or fully filling its order. No method is thread-safe.

`std::list<Order>` is intentional: start with clear ownership and stable nodes. Lookup
and duplicate checking are O(n), and insertion allocates. Neither zero allocation nor
constant-time cancellation is a requirement at this stage.

## Work in small steps

1. Implement `front()`; get the empty-level behaviour right.
2. Implement `add()`, checking every condition before insertion.
3. Implement `fill_front()`; compute the consumed amount before subtraction to avoid underflow.
4. Implement `cancel()` with an iterator; erase only the matching node.

Hints: look up `empty`, `front`, `push_back`, `pop_front` and `erase` on `std::list`,
plus `std::min`. A short loop is enough for ID lookup. No pointers to allocate manually,
inheritance, locks or extra dependencies are needed.

```text
add A(quantity=10), add B(quantity=5)
front => A(10)
fill_front(4) => 4; front => A(6)
fill_front(6) => 6; front => B(5)
fill_front(99) => 5; front => empty
```

## Check your work

```sh
cmake --preset debug
cmake --build --preset debug
ctest --preset debug -L exercise

cmake --preset sanitize
cmake --build --preset sanitize
ctest --preset sanitize

cmake --preset release
cmake --build --preset release
ctest --preset release
```

The executable reports eight scenarios independently; initially all eight fail with
"not implemented". Checks use exceptions rather than `assert`, so Release cannot disable
them. The scenarios cover empty operations, FIFO and owning copies, partial/exact fills,
oversized fills, zero fills, cancellation positions, invalid additions and sell levels.

Done means all scenarios pass in all three presets, including sanitizers, and you can
explain why a partial fill keeps priority, why validation precedes mutation, and when a
borrowed pointer becomes invalid. Add one edge-case test of your own (for example ID
reuse after cancellation or maximum quantity), then remove `-L scaffold` from CI so the
completed exercise stays checked.

The demo still exits with an OrderBook TODO until stage 2. Next, work through
[the learning plan](learning-plan.md); do not skip straight to atomics.
