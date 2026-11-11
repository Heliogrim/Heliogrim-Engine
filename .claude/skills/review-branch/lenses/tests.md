LENS: tests

Scope: do the tests on the range prove the change, and do they stay cheap. Test files live in `src/<Bundle>.Test/`. For T1 and T2 use LSP `findReferences` on the changed public symbol and keep only references inside `*.Test/` directories.

Checklist (cite the id):
- T1 Untested public symbol: a new or behavior-changed public function, operator, or type with zero references from a test file.
- T2 Untested branch: a new `if`/`else`, `switch` case, early return, or error path whose condition no test on the range drives both ways. Name the condition.
- T3 Wrong level: behavior tested only through the editor or engine entry point when the bundle's own test target could test it directly.
- T4 Timing-based waiting: `sleep`, fixed timeouts, spin-until-time, or retry loops in a test instead of waiting on a condition, signal, or join; such tests are flaky and slow.
- T5 Heavy per-test setup: a fixture whose `SetUp` builds an engine, scheduler, device, or file tree that `SetUpTestSuite` could build once; a test suite that starts the scheduler when the unit does not need it.
- T6 Assertion strength: `EXPECT_TRUE(x)` where the value could be compared; a test with no assertion on the result of the function it names; a death or exception test that would also pass on an unrelated failure.
- T7 Duplicate test: a new test that checks the same path as an existing one in the same file with different data only.
- T8 Concurrency test without a model: a new test for a type in `Engine.Async`, `Engine.Scheduler`, or any file with `std::atomic` that relies on thread timing to reproduce the race (see verifying-concurrency skill; report, do not try to fix).

Severity: T1 and T8 major; T2 to T6 minor; T7 minor.
