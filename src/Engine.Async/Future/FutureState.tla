---------------------------- MODULE FutureState ----------------------------
(* Model of the then-chain handoff in src/Engine.Async/Future/Schedule.hpp
   (complete, store, scheduleContinuation), driven by Promise.hpp (resolve,
   reject, drop all end in complete) and Future.hpp then(). A chain of K
   states: state 1 belongs to the promise, then() on state s creates s+1 and
   stores the continuation on s. The continuation of s either settles s+1
   directly, or (fn returned a Future) stores a forwarding continuation on an
   inner state K+s that an independent completer settles later.
   contState is the atomic: complete() exchanges eClaimed after done.set(),
   store() exchanges eStored after writing cont; the side that sees the other
   side's value schedules the task. Scheduled tasks run as their own
   processes. Folded: slot emplace + done.set() are one step (nothing reads
   the slot before done); scheduler exec is the pending set; Arci lifetime
   is covered by Arci.tla; the done Signal by Signal.tla.                  *)
EXTENDS Naturals, FiniteSets, TLC

CONSTANT K                                  \* chain length
Chain == 1..K
Links == 1..(K - 1)
Inner(s) == K + s                           \* inner state used by link s
States == 1..(2 * K - 1)
InnerIds == {100 + l : l \in Links}         \* innerCompleter ids, disjoint from States
Succ(x) == IF x <= K THEN x + 1 ELSE x - K + 1

(* --algorithm FutureState
variables
    done = [x \in States |-> FALSE],        \* slot filled and done.set()
    contSet = [x \in States |-> FALSE],     \* cont written
    contState = [x \in States |-> "None"],  \* eNone / eStored / eClaimed
    pending = {},                           \* continuations handed to the scheduler
    ran = [x \in States |-> 0],             \* how often the continuation of x ran
    useInner = [s \in Links |-> FALSE];     \* link s went through an inner future

define
    RunOnce == \A x \in States : ran[x] <= 1
    RunsAfterBoth == \A x \in States : ran[x] = 1 => (done[x] /\ contSet[x])
    PendingIsReady == \A x \in pending : done[x] /\ contSet[x]
    EventuallyDone == <>done[K]
end define;

\* detail::complete(): done.set() then exchange(eClaimed); schedule if the cont was already stored.
macro complete(x) begin
    done[x] := TRUE;
end macro;

\* detail::store(): write cont then exchange(eStored); schedule if the slot was already complete.
macro store(x) begin
    contSet[x] := TRUE;
end macro;

\* Promise::resolve / reject / ~Promise drop: all reach complete(state 1).
fair process promise = 0
begin
PFill:
    complete(1);
PClaim:
    if contState[1] = "Stored" then
        pending := pending \cup {1};
    end if;
    contState[1] := "Claimed";
end process;

\* Future::then() applied K-1 times, each consuming the previous future.
fair process builder = 99
variable link = 1;
begin
BLoop:
    while link < K do
    BStore:
        store(link);
    BExchange:
        if contState[link] = "Claimed" then
            pending := pending \cup {link};
        end if;
        contState[link] := "Stored";
        link := link + 1;
    end while;
end process;

\* Settles the inner future of link l once its continuation created it.
fair process innerCompleter \in InnerIds
variable l = 0;
begin
ICWait:
    l := self - 100;
    await useInner[self - 100] \/ done[K];
    if ~useInner[self - 100] then
        goto ICDone;
    end if;
ICFill:
    complete(Inner(l));
ICClaim:
    if contState[Inner(l)] = "Stored" then
        pending := pending \cup {Inner(l)};
    end if;
    contState[Inner(l)] := "Claimed";
ICDone:
    skip;
end process;

\* One scheduler task per state: runs the stored continuation once it was scheduled.
fair process task \in States
begin
TWait:
    await self \in pending \/ done[K];
    if self \in pending then
        pending := pending \ {self};
        assert done[self] /\ contSet[self];
        ran[self] := ran[self] + 1;
        if self <= K - 1 then
            either
                skip;                       \* fn returned a value or a Result
            or
                useInner[self] := TRUE;     \* fn returned a Future: forward through Inner(self)
                goto TStoreInner;
            end either;
        end if;
    else
        goto TDone;
    end if;
TSettle:                                    \* settle the successor and complete() it
    complete(Succ(self));
TClaim:
    if contState[Succ(self)] = "Stored" then
        pending := pending \cup {Succ(self)};
    end if;
    contState[Succ(self)] := "Claimed";
    goto TDone;
TStoreInner:                                \* store() the forwarding continuation on the inner state
    store(Inner(self));
TExchangeInner:
    if contState[Inner(self)] = "Claimed" then
        pending := pending \cup {Inner(self)};
    end if;
    contState[Inner(self)] := "Stored";
TDone:
    skip;
end process;
end algorithm; *)
\* BEGIN TRANSLATION (chksum(pcal) = "eec8d2a0" /\ chksum(tla) = "74e272b6")
VARIABLES done, contSet, contState, pending, ran, useInner, pc

(* define statement *)
RunOnce == \A x \in States : ran[x] <= 1
RunsAfterBoth == \A x \in States : ran[x] = 1 => (done[x] /\ contSet[x])
PendingIsReady == \A x \in pending : done[x] /\ contSet[x]
EventuallyDone == <>done[K]

VARIABLES link, l

vars == << done, contSet, contState, pending, ran, useInner, pc, link, l >>

ProcSet == {0} \cup {99} \cup (InnerIds) \cup (States)

Init == (* Global variables *)
        /\ done = [x \in States |-> FALSE]
        /\ contSet = [x \in States |-> FALSE]
        /\ contState = [x \in States |-> "None"]
        /\ pending = {}
        /\ ran = [x \in States |-> 0]
        /\ useInner = [s \in Links |-> FALSE]
        (* Process builder *)
        /\ link = 1
        (* Process innerCompleter *)
        /\ l = [self \in InnerIds |-> 0]
        /\ pc = [self \in ProcSet |-> CASE self = 0 -> "PFill"
                                        [] self = 99 -> "BLoop"
                                        [] self \in InnerIds -> "ICWait"
                                        [] self \in States -> "TWait"]

PFill == /\ pc[0] = "PFill"
         /\ done' = [done EXCEPT ![1] = TRUE]
         /\ pc' = [pc EXCEPT ![0] = "PClaim"]
         /\ UNCHANGED << contSet, contState, pending, ran, useInner, link, l >>

PClaim == /\ pc[0] = "PClaim"
          /\ IF contState[1] = "Stored"
                THEN /\ pending' = (pending \cup {1})
                ELSE /\ TRUE
                     /\ UNCHANGED pending
          /\ contState' = [contState EXCEPT ![1] = "Claimed"]
          /\ pc' = [pc EXCEPT ![0] = "Done"]
          /\ UNCHANGED << done, contSet, ran, useInner, link, l >>

promise == PFill \/ PClaim

BLoop == /\ pc[99] = "BLoop"
         /\ IF link < K
               THEN /\ pc' = [pc EXCEPT ![99] = "BStore"]
               ELSE /\ pc' = [pc EXCEPT ![99] = "Done"]
         /\ UNCHANGED << done, contSet, contState, pending, ran, useInner, 
                         link, l >>

BStore == /\ pc[99] = "BStore"
          /\ contSet' = [contSet EXCEPT ![link] = TRUE]
          /\ pc' = [pc EXCEPT ![99] = "BExchange"]
          /\ UNCHANGED << done, contState, pending, ran, useInner, link, l >>

BExchange == /\ pc[99] = "BExchange"
             /\ IF contState[link] = "Claimed"
                   THEN /\ pending' = (pending \cup {link})
                   ELSE /\ TRUE
                        /\ UNCHANGED pending
             /\ contState' = [contState EXCEPT ![link] = "Stored"]
             /\ link' = link + 1
             /\ pc' = [pc EXCEPT ![99] = "BLoop"]
             /\ UNCHANGED << done, contSet, ran, useInner, l >>

builder == BLoop \/ BStore \/ BExchange

ICWait(self) == /\ pc[self] = "ICWait"
                /\ l' = [l EXCEPT ![self] = self - 100]
                /\ useInner[self - 100] \/ done[K]
                /\ IF ~useInner[self - 100]
                      THEN /\ pc' = [pc EXCEPT ![self] = "ICDone"]
                      ELSE /\ pc' = [pc EXCEPT ![self] = "ICFill"]
                /\ UNCHANGED << done, contSet, contState, pending, ran, 
                                useInner, link >>

ICFill(self) == /\ pc[self] = "ICFill"
                /\ done' = [done EXCEPT ![(Inner(l[self]))] = TRUE]
                /\ pc' = [pc EXCEPT ![self] = "ICClaim"]
                /\ UNCHANGED << contSet, contState, pending, ran, useInner, 
                                link, l >>

ICClaim(self) == /\ pc[self] = "ICClaim"
                 /\ IF contState[Inner(l[self])] = "Stored"
                       THEN /\ pending' = (pending \cup {Inner(l[self])})
                       ELSE /\ TRUE
                            /\ UNCHANGED pending
                 /\ contState' = [contState EXCEPT ![Inner(l[self])] = "Claimed"]
                 /\ pc' = [pc EXCEPT ![self] = "ICDone"]
                 /\ UNCHANGED << done, contSet, ran, useInner, link, l >>

ICDone(self) == /\ pc[self] = "ICDone"
                /\ TRUE
                /\ pc' = [pc EXCEPT ![self] = "Done"]
                /\ UNCHANGED << done, contSet, contState, pending, ran, 
                                useInner, link, l >>

innerCompleter(self) == ICWait(self) \/ ICFill(self) \/ ICClaim(self)
                           \/ ICDone(self)

TWait(self) == /\ pc[self] = "TWait"
               /\ self \in pending \/ done[K]
               /\ IF self \in pending
                     THEN /\ pending' = pending \ {self}
                          /\ Assert(done[self] /\ contSet[self], 
                                    "Failure of assertion at line 108, column 9.")
                          /\ ran' = [ran EXCEPT ![self] = ran[self] + 1]
                          /\ IF self <= K - 1
                                THEN /\ \/ /\ TRUE
                                           /\ pc' = [pc EXCEPT ![self] = "TSettle"]
                                           /\ UNCHANGED useInner
                                        \/ /\ useInner' = [useInner EXCEPT ![self] = TRUE]
                                           /\ pc' = [pc EXCEPT ![self] = "TStoreInner"]
                                ELSE /\ pc' = [pc EXCEPT ![self] = "TSettle"]
                                     /\ UNCHANGED useInner
                     ELSE /\ pc' = [pc EXCEPT ![self] = "TDone"]
                          /\ UNCHANGED << pending, ran, useInner >>
               /\ UNCHANGED << done, contSet, contState, link, l >>

TSettle(self) == /\ pc[self] = "TSettle"
                 /\ done' = [done EXCEPT ![(Succ(self))] = TRUE]
                 /\ pc' = [pc EXCEPT ![self] = "TClaim"]
                 /\ UNCHANGED << contSet, contState, pending, ran, useInner, 
                                 link, l >>

TClaim(self) == /\ pc[self] = "TClaim"
                /\ IF contState[Succ(self)] = "Stored"
                      THEN /\ pending' = (pending \cup {Succ(self)})
                      ELSE /\ TRUE
                           /\ UNCHANGED pending
                /\ contState' = [contState EXCEPT ![Succ(self)] = "Claimed"]
                /\ pc' = [pc EXCEPT ![self] = "TDone"]
                /\ UNCHANGED << done, contSet, ran, useInner, link, l >>

TStoreInner(self) == /\ pc[self] = "TStoreInner"
                     /\ contSet' = [contSet EXCEPT ![(Inner(self))] = TRUE]
                     /\ pc' = [pc EXCEPT ![self] = "TExchangeInner"]
                     /\ UNCHANGED << done, contState, pending, ran, useInner, 
                                     link, l >>

TExchangeInner(self) == /\ pc[self] = "TExchangeInner"
                        /\ IF contState[Inner(self)] = "Claimed"
                              THEN /\ pending' = (pending \cup {Inner(self)})
                              ELSE /\ TRUE
                                   /\ UNCHANGED pending
                        /\ contState' = [contState EXCEPT ![Inner(self)] = "Stored"]
                        /\ pc' = [pc EXCEPT ![self] = "TDone"]
                        /\ UNCHANGED << done, contSet, ran, useInner, link, l >>

TDone(self) == /\ pc[self] = "TDone"
               /\ TRUE
               /\ pc' = [pc EXCEPT ![self] = "Done"]
               /\ UNCHANGED << done, contSet, contState, pending, ran, 
                               useInner, link, l >>

task(self) == TWait(self) \/ TSettle(self) \/ TClaim(self)
                 \/ TStoreInner(self) \/ TExchangeInner(self)
                 \/ TDone(self)

(* Allow infinite stuttering to prevent deadlock on termination. *)
Terminating == /\ \A self \in ProcSet: pc[self] = "Done"
               /\ UNCHANGED vars

Next == promise \/ builder
           \/ (\E self \in InnerIds: innerCompleter(self))
           \/ (\E self \in States: task(self))
           \/ Terminating

Spec == /\ Init /\ [][Next]_vars
        /\ WF_vars(promise)
        /\ WF_vars(builder)
        /\ \A self \in InnerIds : WF_vars(innerCompleter(self))
        /\ \A self \in States : WF_vars(task(self))

Termination == <>(\A self \in ProcSet: pc[self] = "Done")

\* END TRANSLATION 
=============================================================================
