# Learning plan: exchange first, simulated trader later

The goal is to understand the engineering behind low-latency systems and produce a
portfolio with reproducible evidence. Correctness comes first; faster incorrect matching
is not progress. Each stage should leave a runnable check and a short explanation of what
you learned. Implement the stages yourself rather than asking for complete solutions.

## 1. One FIFO price level

See the completed [exercise 1 checklist](exercises.md#exercise-1-fifo-price-level--complete).
Own orders in `std::list`, validate before insertion,
fill only the oldest order and cancel without disturbing surviving priority.

**Completed:** the eight supplied scenarios pass in Debug, sanitizer and Release builds.
Optional follow-up: add your own ID-reuse or maximum-quantity test.

## 2. Deterministic limit order book

Follow [exercise 2](exercises.md#exercise-2-build-the-limit-order-book) for the contract,
worked example and implementation checkpoints.

Implement `OrderBook::apply`. Use ordered maps of price levels, with highest bid and lowest
ask first. Reuse `PriceLevel`; add a book-wide active-ID index. Linear cancellation within
a level is acceptable initially; do not claim O(1) cancellation.

Start with one instrument and good-till-cancelled limit orders plus cancellation:

- Validate nonzero IDs, positive price/quantity and globally unique active IDs before mutation.
- Match only when buy limit >= best ask, or sell limit <= best bid.
- Choose best price first, then oldest order; execute at the **resting** order's price.
- Emit acceptance before trades. Partial resting orders retain priority; unfilled incoming
  quantity rests at its limit. Fully filled orders leave the active-ID index.
- Remove empty levels. Unknown cancellation produces a rejection, leaving state unchanged.
- ID reuse is allowed after an order is no longer active. Input sequence determines time
  priority; wall-clock timestamps do not decide it.

Add basic best-bid/best-ask inspection so tests can check state without exposing storage.
Expected input rejection is a `Rejected` event; programming/unimplemented errors remain
exceptions. Allocation failure is fatal to the run: do not claim recovery or atomicity
under out-of-memory conditions in this baseline.

**Complete when:** tests cover both sides, non-crossing/crossing orders, best-price selection,
FIFO at equal price, multiple levels, partial fills, cancellations, duplicate IDs and the
README sample. Check after each command: quantities are positive, no empty levels remain,
IDs agree with resting orders, and best bid < best ask when both exist. Repeating a command
sequence must produce identical events. Treat these tests as the reference for optimisation.

Then add market orders with unfilled remainder cancelled; introduce IOC and FOK as separate
policies. FOK must check available eligible quantity before executing anything. Defer order
amendment, auctions, hidden liquidity, multiple instruments and venue-specific rules.

## 3. Replay and measure a baseline

Add a strict text replay reader: one `ADD id BUY|SELL price quantity` or `CANCEL id` per
line. Report malformed/overflowing fields with line numbers; do not process a partially
parsed command. Use synthetic, seeded workloads and separate parsing from matching.

Use `std::chrono::steady_clock` for a first benchmark. Pre-generate commands, warm up,
disable console output during timed work and consume results so the work cannot disappear.
Measure throughput in a separate batch run from sampled per-command latency. Report
sample count, p50/p90/p99/p99.9, workload, compiler flags, CPU and OS. Include measurement
overhead; do not infer meaningful extreme percentiles from a tiny sample. No promised
nanosecond target and no comparison between unlike benchmark setups.

**Complete when:** replay produces the same events as direct commands, malformed input
tests pass, and a documented command reproduces a baseline. Profile a Release build with
Linux `perf` when available; distinguish matching time from file I/O and event printing.

## 4. Storage and allocation

Use profiles to choose the next change: preallocated order slots and index handles,
an ID index, then denser price-level storage only if the supported tick range is bounded.
Keep the STL implementation as a test oracle. Avoid defaulting every order to a whole
cache line; measure layout and access patterns first.

Explicitly define full-capacity behaviour before mutation. An order that cannot rest must
be rejected without partial side effects; use preflight/reservation where needed. Reuse
slots without stale handles (for example generation counters). Preallocate output space
as well as order storage before claiming a zero-allocation processing path.

**Complete when:** the reference scenarios agree, capacity/reuse/stale-handle tests pass,
allocation measurement supports the stated claim, and the same benchmark shows the change's
effect. If a change does not help, record that result rather than inventing a speedup.

## 5. Bounded SPSC ingestion

Separate a producer from the engine using a fixed-capacity single-producer/single-consumer
queue. The engine remains the sole owner of book state. Start with value commands, then
study slot lifetime, index wraparound, acquire/release publication and false sharing.

```text
producer: parse/generate -> validate representation -> enqueue (retry if full)
engine:   dequeue -> book.apply -> collect events
shutdown: producer publishes done -> engine drains queue -> joins -> summary
```

Do not silently drop commands on a full queue. Explicitly signal producer failure and
shutdown; the consumer drains before exiting. Add an output queue only when there is a
separate output consumer; specify its overflow handling too. CPU affinity comes after
correctness, with errors reported and portable single-threaded mode preserved.

**Complete when:** empty/full/wraparound tests pass; threaded tests preserve exact sequence
and count across many wraps and during shutdown. Run ThreadSanitizer in a **separate**
build from AddressSanitizer, and compare end-to-end results against the sequential engine.
Inspect whether the chosen atomics are lock-free before making that claim.

## 6. Add a simulated trader

Publish ordered book/trade events to a simulated client. Start with a simple deterministic
quote-following strategy; route its orders through risk checks into the same exchange.
Separate external synthetic participant IDs from the strategy's IDs.

```text
market event -> strategy intent -> pre-trade risk -> simulated exchange
execution report -> outstanding orders + position + cash -> next decision
```

Check maximum order size and worst-case position including outstanding orders. Track
fills, fees, cash, inventory and marked P&L with documented arithmetic and marks. Add a
kill switch that blocks new orders and requests cancellation of the strategy's resting
orders. Use deterministic event scheduling; process responses without recursive callbacks.

**Complete when:** scenarios demonstrate fills, partial fills, rejects, risk limits and
kill-switch behaviour. Replays produce identical positions and P&L. Simulated profitability
is an experiment, not evidence of executable returns in a real market.

## Useful references

- [CME matching algorithms](https://www.cmegroup.com/education/matching-algorithm-overview):
  FIFO is one matching policy, not a universal exchange rule.
- [Clang AddressSanitizer](https://clang.llvm.org/docs/AddressSanitizer.html) and
  [UndefinedBehaviorSanitizer](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html).
- [Clang ThreadSanitizer](https://clang.llvm.org/docs/ThreadSanitizer.html) for the threading stage.
- [C++ standard draft: containers](https://eel.is/c++draft/containers) and
  [atomics](https://eel.is/c++draft/atomics) for contracts and memory ordering.

Keep measurements, known limits and design decisions in the repository as they become
real. A clear, tested baseline and an honest performance comparison make a better portfolio
than a collection of unverified low-latency claims.
