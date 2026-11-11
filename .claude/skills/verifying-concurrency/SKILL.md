---
name: verifying-concurrency
description: Use when a branch adds or changes a lock, mutex, park/unpark, signal, join counter, future state, spin lock, lock-free queue, or any code with std::atomic in Engine.Async or Engine.Scheduler, when the tests lens reports T8, or when the owner asks whether a primitive is "formally proven", "model checked", or "verified".
---

# Verifying Concurrency

A stress test shows a primitive works on this machine today. A model checker shows every interleaving the model allows is safe. Both are required for a primitive; a review lens is not a substitute for either.

## Tiers and when each applies

| Tier | Tool | Checks | Required for |
|------|------|--------|--------------|
| 1 | gtest stress in `<Bundle>.Test` | the real code, real scheduler, a few thousand interleavings | every primitive |
| 2a | TLA+ / PlusCal + TLC (`<Primitive>.tla` + `.cfg` beside the source) | the protocol: safety, lost wakeups, deadlock, under sequential consistency | every primitive with a wait or park, and every counting protocol whose invariant is arithmetic across interleavings (Arci) |
| 2b | Relacy (`verify/relacy/`) | the C++ memory orders of the actual operations, data races, deadlock, livelock | every primitive with a relaxed, acquire, or release order |
| 3 | TSan on a Linux/WSL build | races the stress test hits at runtime | optional; WSL is not installed on the dev box |
| 4 | Lean | an invariant of the abstract state machine | not scheduled; only for a primitive the whole scheduler rests on |

TLA+ cannot see memory orders and Relacy cannot see the scheduler, so 2a and 2b are not alternatives.

## Queue (stacked local master, 2026-11-11)

| Primitive | Source | 1 stress | 2a TLA+ | 2b Relacy |
|-----------|--------|----------|---------|-----------|
| FiberMutex | `Engine.Async/Primitive/FiberMutex.cpp` | yes | `FiberMutex.tla`, passes N=3, mutation-checked | pending |
| FiberSharedMutex | `Engine.Async/Primitive/FiberSharedMutex.cpp` | yes | `FiberSharedMutex.tla`, passes N=3 with `NoLostWakeup` liveness in 5.5 min (2.3M states); invariant-only symmetric run 4 s; 4 mutations checked at N=3 (re-check under lock, transient-reader handoff, dropped reader wakeup, and the original barging + readers-first protocol, which fails `NoLostWakeup` in 2 min); either fairness fix alone still passes at N=3 | pending |
| UnfairSpinLock | `Engine.Async/Primitive/SpinLock.hpp` | yes | n/a (no wait) | pending |
| ParkQueue + park/unpark handshake | `Engine.Scheduler/Waitable/ParkQueue.hpp`, `Fiber/Park.cpp`, `Worker.cpp` | via users | inside every model above | pending |
| Signal | `Engine.Scheduler/Waitable/Signal.cpp` | yes | `Signal.tla`, passes N=3 under the latch destroy rule (9k states, 1 s); mutation: the earlier "safe once wait() returned" rule asserts on a destroyed-object access | pending |
| JoinCounter | `Engine.Scheduler/Waitable/JoinCounter.cpp` | yes | `JoinCounter.tla`, passes K=3, mutation-checked | pending |
| FutureState / Promise then-chain | `Engine.Async/Future/Schedule.hpp` (`complete`, `store`), `Promise.hpp`, `Future.hpp` | yes | `FutureState.tla`, passes K=3 (47k states, 1 s) and K=4 (3M states, 2 min); mutations: store ignoring a prior claim deadlocks, exchange before writing `cont` or before `done.set()` both fail `PendingIsReady` | pending |
| Arci<T> | `Engine.Common/Managed/AtomicRefCountedIntrusive.hpp` | yes | `Arci.tla`, passes 3 agents x 2 slots in 5.7 min (55M states, symmetric); mutations: dropped undo after failed flush CAS and unpinned copy both fail `CountConsistent`; a dropped slot re-read after a failed CAS is a lock-freedom bug TLC cannot see | pending (the `release`/`acquire` pairing at line 201 is marked unverified) |

Update this table in the same commit that adds a model.

Resolved finding (2026-11-11, from `FiberSharedMutex.tla` at N=3): writers could starve. `unlock()` admitted queued readers before queued writers, and the `lock()` slow path took a free lock ahead of writers already queued; with one writer and one reader looping, a second parked writer was never woken (TLC cycle of 34 states). Fixed in the code: the slow path parks whenever `_writers` is non-empty and `unlock()` alternates between the queues (`_readersLast`). The model's `NoLostWakeup` liveness property now holds at N=3; reverting both fixes fails it in 2 min, reverting only one still passes at N=3 (each fix alone closes the cycle at that size). Regression test: `FiberSharedMutex.SecondWriterAcquiresUnderWriterAndReaderLoop`. Benchmark `Heliogrim-Performance.Test --gtest_also_run_disabled_tests --gtest_filter=LockBenchmark.*` showed no measurable cost (Debug build).

Resolved finding (2026-11-11, from `Signal.tla`, issue #174): the `Signal.hpp` comment "destroying a Signal is safe once isSet() returned true or wait() returned" was too weak. A second waiter that passed the `_state` fast check but had not yet taken the `_waiters` lock touched the destroyed object. The header now states the `std::latch` rule (destroy only when every waiter returned from `wait()`), and the model keeps only that rule with the weak one as its mutation. Callers checked with clangd: `EditorEngine`/`GameEngine` stack signals with one waiter, `JoinCounter` whose creator is the only waiter, `Await.cpp` forwarding; none shares a Signal between waiters that destroy it. A Signal any waiter may destroy would need an in-flight counter and is a different primitive.

## 2a. TLA+ recipe

Tooling: `java` is on PATH (JDK 26). Fetch `tla2tools.jar` once into `verify/` (gitignored):
```powershell
curl.exe -sL -o verify\tla2tools.jar https://github.com/tlaplus/tlaplus/releases/latest/download/tla2tools.jar
```

1. One module per primitive, next to its source: `src/<Bundle>/.../<Primitive>.tla` written in PlusCal, `<Primitive>.cfg` beside it (the module name must equal the file name, so `AtomicRefCountedIntrusive.tla` models `Arci<T>`). The bundle glob only picks up `.hpp`/`.cpp`, so the files are inert for the build. The header comment names the source files and which steps were folded (bounded spins become one step); a model that also covers another bundle (the park/unpark handshake) says so there.
2. One label per atomic operation on the real code path. A `locked(...)` section stays multi-step with the queue lock held; only state that no other thread can observe before the lock drops may be merged into one step.
3. Required checks in the `.cfg`: the primitive's safety invariant (e.g. `Mutex`), a wakeup-safety invariant that names the window in which a waiter may be orphaned, one `~>` liveness property under `fair process`, and `CHECK_DEADLOCK TRUE`. Fairness is weak; an unfair primitive must not claim per-waiter progress.
4. `N = 3` is the floor; raise to 4 when the run stays under a minute. Two reductions before buying more hardware or a TLAPS proof: reset dead process locals (`w := Free` at critical-section entry, `role` at loop end) so stale copies do not multiply identical states (`FiberSharedMutex` N=2: 120k to 893 states); and when the `.cfg` checks invariants only, make the agents model values with `SYMMETRY Permutations(Agents)` (not sound for `~>` properties, so FiberMutex and Signal keep integer agents). Neither changes what the model says about the code.
5. Mutation check, mandatory: remove one line the code's comments call load-bearing (a `fetch_or`, a re-check under the lock) in a scratch copy and confirm TLC reports a violation. A model that passes with the bug is not a model.

TLC resolves modules against the working directory only (a path to the `.tla` does not parse), so run from inside the module's directory; its `states/` output is gitignored under `src/`:
```powershell
cd src\Engine.Async\Primitive
java -cp ..\..\..\verify\tla2tools.jar pcal.trans FiberMutex.tla
java -XX:+UseParallelGC -Xmx16g -cp ..\..\..\verify\tla2tools.jar tlc2.TLC -workers 12 -cleanup FiberMutex.tla
```
From the Git Bash `!` prompt use forward slashes. `-workers auto` takes every core; leave some for the build. Two TLC runs in one directory collide on `states/`; give the second `-metadir <other>`.
Commit the `.tla` with its generated translation block and the `.cfg`. `FiberMutex.tla` is the reference example.

## 2b. Relacy recipe

Relacy is not vendored yet; vendoring it is the first task of this tier and needs the owner's yes (third-party code under `vendor/relacy/include/relacy/`, pinned to a commit of `dvyukov/relacy`, license file copied).

1. Target `Heliogrim-Verify.Relacy` from `verify/relacy/*.cpp`, built with clang-cl like the tests, excluded from `Heliogrim-AllTests`.
2. The engine source is compiled unchanged. A shim include directory `verify/relacy/shim/` goes first on the include path and provides `<atomic>` mapping `std::atomic<T>`, `std::atomic_flag`, and `std::memory_order::*` onto `rl::atomic<T>`, `rl::atomic<bool>` and `rl::mo_*`, each call carrying `$`. `_mm_pause()` maps to `rl::yield(1, $)`.
3. `verify/relacy/Park.cpp` replaces the scheduler's `Park.cpp`: `park` spins on `threadFlag` with `rl::yield`, `unpark` stores and that is all. Fibers are modelled as Relacy threads; the worker handshake is covered by TLA+, not here.
4. One `rl::test_suite<Test, threads>` per primitive and per scenario (lock/unlock, barging, shared+exclusive). `rl::sched_full` for up to 3 threads; `rl::sched_random` with at least 1e6 iterations for 4.
5. Done when every scenario passes under both schedulers and a mutation (change one `acquire` to `relaxed`) fails.

## 3. TSan (when WSL exists)

`wsl --install` is the owner's call. Then a Linux clang build of `Heliogrim-Async.Test` and `Heliogrim-Scheduler.Test` with `-fsanitize=thread`, run with `TSAN_OPTIONS=halt_on_error=1`. clang-cl on Windows has no TSan; do not try.

## Common mistakes

- Modelling the code you wish you had: the model follows the `.cpp` line by line; a difference is a finding against the code or the model, never silently resolved.
- Claiming fairness: FiberMutex allows barging; "every waiter eventually acquires" is false for it and must not be a property. FiberSharedMutex earns its `NoLostWakeup` property only through the queue alternation; a model that drops it must fail.
- Skipping the mutation check: a vacuous model gives false confidence, which is worse than none.
- Treating TLC "no error" as proof of the C++: it proves the protocol under sequential consistency. Memory orders are Relacy's job.
