# Project plan

## Objective and deliverables

Build a deterministic C++20 exchange simulator with a price/time-priority limit order
book, reproducible replay, measured performance and a bounded concurrent execution mode.
Extend it with additional execution policies and a simulated trading client after the
exchange core is verified.

The deliverable is a Linux command-line application and reusable matching library, with
automated tests, synthetic workloads, ordered execution reports and reproducible benchmark
results. The [system architecture](architecture.md) defines component boundaries and
runtime behaviour. This document specifies delivery milestones and release acceptance.

### Product scope

| Included | Outside the planned releases |
|---|---|
| One synthetic instrument and integer price ticks | Live exchange or broker connectivity |
| FIFO matching, partial execution and cancellation | Venue certification, auctions and hidden liquidity |
| Deterministic input replay and event output | Distributed matching or multiple engine writers |
| Bounded storage and SPSC transport after verification | Replicated persistence or crash recovery |
| Throughput, latency and resource measurements | Guaranteed nanosecond latency on arbitrary hardware |
| Market/IOC/FOK policies as a separate feature milestone | GUI, hosted services or public network gateway |
| Simulated client, pre-trade risk and account tracking | Real-money execution or profitability claims |

Linux, Clang, C++20, CMake and Ninja are the supported baseline. GNU Make wraps the
CMake presets for local use and CI. Use the installed standard library; additional
dependencies require a concrete need. The first release uses STL containers.

### Current implementation

The build, CLI scaffold, shared command/event types and FIFO `PriceLevel` are implemented.
The eight price-level scenarios pass locally in Debug, AddressSanitizer/UndefinedBehaviorSanitizer
and Release builds. CI is configured to run all registered checks through `make test`.

`OrderBook::apply()` remains a throwing stub. Replay, benchmarking, fixed-capacity storage,
queues, additional order policies and the trading client below are planned capabilities.

## Requirements

1. Process valid limit orders and cancellations in a deterministic sequence. Prioritise
   best price, then arrival order within a price; execute at the resting order's price.
2. Reject invalid values, duplicate active IDs and unknown cancellations without changing
   the book. Reuse an ID once its previous order is no longer active.
3. Preserve a consistent book after every command: positive remaining quantities, correct
   active-ID lookup, no empty price levels and no crossed resting best prices.
4. Produce ordered acknowledgements, rejections, trades and cancellation reports. Replay
   the same command sequence into the same initial state to reproduce those events.
5. Keep mutable book state under one owner. Concurrent transport must preserve ordering,
   reject or backpressure at capacity, and never silently discard commands or reports.
6. Demonstrate allocation and performance claims with measurements. A benchmark must name
   its workload, compiler, build flags, machine, measurement boundary and sample count.
7. Keep the single-threaded implementation as a functional reference. Storage and transport
   changes must produce the same logical events for the same command sequence.

The simulator deliberately accepts only positive prices and quantities. These are product
rules for the synthetic instrument, not claims about every financial instrument or venue.

## Delivery milestones

Milestones are dependency ordered. No calendar dates or throughput commitments are assigned
before a complete engine and baseline workload exist. Each milestone ends with a reviewed
change, passing checks and an update to this document's implementation status.

| Milestone | Deliverables | Dependency and acceptance gate |
|---|---|---|
| M0 — foundation | Build presets, CLI, shared types, FIFO price level, CI | Implemented. Supplied price-level and CLI checks pass in all three presets. |
| M1 — matching core | Bid/ask books, active-ID index, limit matching, cancellation, best-price inspection, ordered reports | M0. Both sides, FIFO, multi-level execution, rejection and ID-reuse tests pass; existing sample runs successfully. |
| M2 — replay and baseline | Strict replay reader, seeded workloads, deterministic event export, benchmark runner and first results | M1. Replays agree with direct calls; malformed input is diagnosed; documented commands reproduce functional and performance runs. This is the first usable release. |
| M3 — bounded storage | Capacity configuration, preallocated order slots and output scratch space, safe slot reuse, measured allocation behaviour | M2. Capacity exhaustion has no partial command effects; results match the STL reference; allocation instrumentation supports the stated processing-path claim. |
| M4 — concurrent transport | Bounded ingress and egress SPSC queues, single engine owner, backpressure, error propagation and drain/join shutdown | M3. Sequence/count stress tests and ThreadSanitizer checks pass; concurrent output matches the sequential reference; end-to-end measurements are published. |
| M5 — execution policies | Market orders, IOC and FOK with explicit remainder and terminal-report semantics | M4. Policy tests prove no resting market/IOC remainder and no partial execution of rejected FOK orders; replay format and reference behaviour are updated together. |
| M6 — simulated client | Ordered market updates, deterministic strategy scheduling, pre-trade risk, outstanding-order tracking, positions, cash, fees and marked P&L | M5. Repeatable scenarios cover fills, rejects, risk limits, cancellation and kill switch. Reports reconcile to exchange events. |

### Work packages

**M1:** implement state ownership and non-crossing insertion first, then rejection and
cancellation, then one-level and multi-level matching. Keep validation ahead of mutation.
Test order state and event order, not only trade counts.

**M2:** separate parsing from application. Add deterministic command/event sequencing and
versioned text output. Build benchmarks from pre-generated commands so parsing, disk and
console output are excluded from core matching measurements. Measure them separately for
replay and end-to-end runs.

**M3:** profile before replacing storage. Introduce fixed-capacity slots and generation
handles; reserve every resource required to complete a command before mutation. Keep
ordered price maps unless profiles justify a bounded dense price range. Translate rejection
codes to strings outside the allocation-free processing path. Report the cancellation
complexity actually achieved; intrusive links alone do not make ID lookup constant time.

**M4:** verify queue ownership, publication and slot reuse before measuring throughput.
Record queue occupancy and saturation. CPU affinity is an optional, explicit benchmark
setting, with the actual applied affinity and failures reported. Add padding only to shared
queue metadata where it avoids false sharing; do not align every order to a full cache line.

**M5:** define policy validation and event changes before implementing them. FOK uses an
eligibility/quantity preflight and either executes completely or leaves the book unchanged.
Compare aggregate quantities with a sufficiently wide checked accumulator. Add any terminal
remainder report before exposing the policy in replay input.

**M6:** begin with a deterministic single-threaded event scheduler and a simple strategy.
All strategy orders pass through risk checks and the normal command path. A separate client
thread, if subsequently added, must not become a second producer on an SPSC queue.

## Verification and release gates

### Functional checks

- Test empty state; non-crossing insertion; crossings on either side; execution price;
  FIFO; partial and multi-level fills; duplicate IDs; invalid commands; cancellation and ID reuse.
- Compare logical event streams and final state against the reference engine on seeded
  workloads. Record the seed and input when an invariant or comparison fails.
- Test malformed/truncated replay lines, numeric overflow, extra fields and unknown commands.
  Parse errors identify a line and stop the run with a nonzero status.
- For bounded storage, exhaust each resource and verify rejection before mutation. For
  concurrent transport, stress full/empty queues, wraparound, slow consumers and shutdown.
- Keep Debug, sanitizer and Release functional checks in CI. Add ThreadSanitizer separately
  at M4; run allocation checks and performance jobs separately from sanitizer jobs.

### Performance checks

Report both matching-core throughput and end-to-end throughput; name latency boundaries
explicitly. Measure latency distributions in a separate sampled run to limit instrumentation
distortion. Start with p50/p90/p99/p99.9; include more extreme percentiles only with an
adequate sample count and repeated observations.

Include warm-up, repetitions, clock source, command mix, active-order count, price spread,
queue sizes, output mode, CPU, OS, compiler and flags. Measure an instrumentation baseline.
Do not use a shared CI runner as a stable latency gate. Once a dedicated reference machine
and noise envelope are recorded, agree a regression threshold there before enforcing it.

### Release checklist

- [ ] All release capabilities have functional checks and documented CLI commands.
- [ ] Sanitizers report no errors in the exercised paths; tests remain enabled in Release.
- [ ] Replay format/version, limits, overload behaviour and failure statuses are documented.
- [ ] Any performance claim links to a workload, environment description and result artifact.
- [ ] No command or execution report can be silently lost during supported operation.
- [ ] Deferred features and known operational limits are recorded in the release notes.

## Risks and decisions

| Risk | Response |
|---|---|
| Optimisation changes matching behaviour | Preserve the reference implementation and compare event streams before merging. |
| Hidden allocation in reports or ID lookup | Instrument the complete claimed path, including rejection handling and output preparation. |
| A slow output consumer blocks the engine | Apply bounded backpressure and expose occupancy; measure this condition explicitly. |
| Finite capacity causes a partially processed order | Reserve resources before mutation; do not reject after emitting irreversible fills. |
| Scheduling or timestamps alter replay results | Use command sequence for priority and deterministic scheduling for simulation. |
| Kernel scheduling and shared hardware distort latency | Publish the environment and repeated measurements; pinning is not a real-time guarantee. |
| Process or disk failure loses recent state/events | Fail visibly; no durable acknowledgement or crash-recovery guarantee is offered. |
| Features outrun verified infrastructure | Keep the first usable release at M2; later milestones ship only after their own gates. |

This plan adapts the original matching-engine blueprint by adding behavioural contracts,
replay, explicit capacity and failure semantics, and measurable acceptance gates. It moves
custom storage and concurrency behind correctness and profiling, and treats execution
policies as separate functionality rather than variations hidden inside limit insertion.
