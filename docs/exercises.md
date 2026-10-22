# Exercises

## Exercise 1: FIFO price level — complete

- [x] Validate ID, price, quantity, side and duplicate IDs before changing state.
- [x] Store owning order copies in FIFO order.
- [x] Return the oldest order, or `nullptr` for an empty level.
- [x] Handle zero, partial, exact and oversized fills without touching the next order.
- [x] Cancel by ID while preserving the order of survivors.
- [x] Pass all eight supplied scenarios in Debug, sanitizer and Release builds.

Implemented in `src/price_level.cpp`. Key skills: ownership, const pointers, iterators,
invalidation, validation before mutation and avoiding unsigned underflow.

## Exercise 2: build the limit order book

**Goal:** implement `OrderBook::apply()` in `src/order_book.cpp`, reusing your `PriceLevel`.
The book chooses the best price; each level chooses its oldest order. Completing this
exercise makes the existing `make run` demo work.

Read `include/hft/order_book.hpp`, `include/hft/types.hpp` and the sample commands in
`src/main.cpp`. Keep the existing `Command`, `Result` and event types. Work on one synthetic
instrument, one thread, limit orders and cancellation only.

### Storage and interfaces

- Store bids and asks separately in ordered maps of `Price` to `PriceLevel`.
  `std::map` sorts ascending by default: the first ask is best, the last bid is best.
- Track active order IDs in a book-wide index mapping each ID to its side and price.
  `std::unordered_map` is fine. This lets cancellation find the correct level; the
  level's own cancellation remains a linear scan.
- Add `std::optional<Price> best_bid() const` and `best_ask() const` for testing.
  An empty side returns `std::nullopt`. Keep the containers private.

Hint: `PriceLevel` has no default constructor. Look up `map::try_emplace` to construct a
level with its side and price; `operator[]` tries to default-construct a missing value.

### Required behaviour

| Command or situation | Expected behaviour |
|---|---|
| Invalid order | Return one `Rejected` event for zero ID, nonpositive price, zero quantity, invalid side or an ID already active anywhere in the book. Leave state unchanged. |
| Valid order | Emit `Accepted` first, followed by any `Trade` events in matching order. |
| Incoming buy | Match the lowest ask while its price is <= the buy limit. |
| Incoming sell | Match the highest bid while its price is >= the sell limit. |
| Multiple orders at one price | Fill the oldest first; a partial resting fill keeps its priority. |
| Trade | Use the resting order's ID as `maker_id`, incoming ID as `taker_id`, resting price and consumed quantity. |
| Incoming quantity remains | Rest the remainder on its own side at its original limit price. |
| Resting order fills completely | Remove its active ID; remove the level if it becomes empty. |
| Cancel an active ID | Remove it from the level and index; emit only `Cancelled`. Remove an empty level. |
| Cancel an unknown ID | Emit only `Rejected`; leave state unchanged. |
| Reuse a completed/cancelled ID | Accept it: uniqueness applies to currently resting orders. |

Rejection messages should explain the problem; tests need not require exact wording.
Expected invalid input produces `Rejected`, not an exception from `PriceLevel::add()`.
Check validity before matching so a bad order cannot consume somebody else's liquidity.
As in the baseline plan, allocation failure may terminate the run; recovery is out of scope.

### Work in small steps

1. **Rest orders:** start with empty sides, add non-crossing orders and implement best-price
   inspection. Add tests before moving on.
2. **Reject invalid orders:** include duplicate IDs across different sides and prices.
   Verify rejection leaves the original order available.
3. **Cancel:** use the active-ID index, reuse `PriceLevel::cancel()` and clean up empty levels.
4. **Match one level:** consume the oldest opposite-side order, emit a trade and update
   both quantities. Repeat until the incoming quantity is gone or that level is empty.
5. **Match multiple prices:** continue to the next best eligible level, then rest any
   incoming remainder. Add the full demo scenario.

Be careful with lifetime: `fill_front()` can destroy the order returned by `front()`.
Save its ID, price and quantity before filling; do not read the pointer afterwards.
Erasing a map level also invalidates that level's iterator. Stop when an opposite side
is empty before accessing `begin()` or `rbegin()`.

### Worked example

```text
ADD sell #1: price 100, quantity 10  -> Accepted{1}
ADD sell #2: price 100, quantity 5   -> Accepted{2}
ADD buy  #3: limit 101, quantity 12  -> Accepted{3}
                                       Trade{maker=1, taker=3, price=100, quantity=10}
                                       Trade{maker=2, taker=3, price=100, quantity=2}
CANCEL #2                          -> Cancelled{2}
```

After the buy, only sell #2 remains, with quantity 3 at price 100. After cancellation,
both sides are empty. The buy limit of 101 does **not** change the execution price of 100.

### Write the checks

Create `tests/order_book_tests.cpp` using the small test style from
`tests/price_level_tests.cpp`. Register its executable and CTest entry in `CMakeLists.txt`
with the `exercise` label. Use checks that remain enabled in Release, not plain `assert`.

Cover these scenarios:

- [ ] Empty book and correct best bid/ask after non-crossing additions.
- [ ] Invalid orders and duplicate IDs across levels leave state unchanged.
- [ ] Exact-price and better-price crossings work on both sides.
- [ ] Best price wins even when a worse-priced order arrived earlier.
- [ ] Equal-price orders fill FIFO; partial fills preserve priority.
- [ ] One incoming order consumes several resting orders and price levels.
- [ ] Matching stops at the limit; any incoming remainder rests at its original price.
- [ ] Cancel first, middle, last and unknown IDs; remove empty levels.
- [ ] Reuse IDs after full execution and cancellation.
- [ ] The worked example emits exactly the listed events, in order.
- [ ] Replaying the same sequence on a fresh book produces identical events.

```sh
make test
make test PRESET=sanitize
make test PRESET=release
make run
```

CI now runs `make test`, so registered book tests will run automatically too. Exercise 2
is complete when these checks pass, the demo matches the worked example, and you can explain
price priority, time priority, maker/taker IDs and safe container erasure.

Defer market orders, IOC/FOK, threading, custom allocation and performance optimisation.
The [learning roadmap](learning-plan.md) explains where those belong.
