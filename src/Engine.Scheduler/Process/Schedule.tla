------------------------------ MODULE Schedule ------------------------------
(* Model of the three-lane worker park/wake protocol:
   src/Engine.Scheduler/Process/Schedule.cpp (push, pop, leave, rollFrame,
   parkTimeoutMillis, waitForWork, takeable, cascade, wakeOne, wakeAll) and
   src/Engine.Scheduler/Worker/Worker.cpp (Worker::handle).

   Wake rule (tree rule, W02.15): a push with wake_ wakes one parked worker
   when the pusher is not a worker or its depth fetch_add made the lane
   non-empty (depth after the increment is 1); a successful pop wakes two
   when takeable(now) holds after it (cascade); the yield and park
   re-pushes in Worker::handle pass wake_ = false (quiet: that worker pops
   next). wakeAfterSubmit (batch submit) is not modelled.

   Modelled, one label per atomic operation of the code:
   - push(pt, pext, pquiet): depth[lane]++ before the insert, which also
     decides the wake (~pquiet /\ (pext \/ depth = 1), from the fetch_add
     result), the insert, the epoch bump, and WakeByAddressSingle when it
     decided to wake. pext is "not a worker", pquiet is wake_ = false.
   - pop: the floor turn (depth load, CAS on the due flag, pop or turn spent,
     lowerRunning++, depth--), then the Frame set, then the lower path
     (depth load, reserve load, CAS claim of lowerRunning against
     N - reserve, pop, depth--, lastStart stamp, or undo of the claim).
   - Worker::handle: epoch load before the pop, cascade after a successful
     pop (WCasc: takeable, WWake: two wakeOne), run, leave(lane) when the
     run ends, quiet re-push of a yielded task after leave, and the park:
     parkTimeoutMillis reads the Frame depth (non-zero: timeout 0, the
     worker loops back to the epoch load and pop instead of parking) and
     the lower depth, WaitOnAddress compares the epoch and enqueues.
   - rollFrame: the reserve exchange, then epoch bump and wakeOne when the
     reserve dropped and lower work is queued. resetReserve (pipeline stop)
     does the same; a reserve not rolled for 250 ms counts as the minimum
     in pop (stale fallback, not modelled).
   - Scheduler stop: per-worker interrupt, then wakeAll (bump + wake all).

   Abstractions:
   - Set 0 is Frame, set 1 is one lower lane. One lower lane is enough: the
     code treats Interactive and Background alike in every step that touches
     parking, the reserve and lowerRunning (one shared counter, one summed
     depth check, one cap); the floor is per lane but each lane's turn is
     the same CAS on its own lastStart, so a second lane adds a copy of the
     same protocol and no new interleaving with the Frame or park paths.
   - The clock is a flag: floorDue stands for now - lastStart >= floor. The
     Clock process raises it while lower work is queued; a floor claim (CAS)
     or a normal lower start (raiseTo lastStart) lowers it.
   - The bounded park (parkTimeoutMillis while lower work is queued) is the
     weakly fair FloorWake step: it takes a worker that parked with a
     timeout out of waitq once floorDue holds and lower work is queued.
   - The push epoch is abstracted ABA-free: fresh[w] is "the epoch has not
     moved since w loaded it"; every bump clears all fresh flags.
   - The reserve starts at RHi and takes the values 0 and RHi, moved by
     the Roll process at most Rolls times. The pick is unfair (a roll may
     never come); the steps after a pick are weakly fair. With Rolls = 2
     the reserve can drop and rise again (RHi -> 0 -> RHi), so a rise can
     land after a worker read the reserve; the rise stands for
     noteFrameWait's raise and rollFrame's raiseTo to framePeak, which need
     no wake. The code clamps the reserve to 1..workers-1; 0 is below the
     clamp and RHi = N is above it (all-parked scenario only).
   - WCasc evaluates takeable in one step (Takeable: Frame depth, or lower
     depth with floorDue or lowerRunning < N - reserve); the code does
     separate relaxed loads of the depths, lastStart, lowerRunning and the
     reserve. A stale load only skips or adds the cascade's wakes; a
     skipped wake is the case the next push or pop covers, an added one is
     a spurious return. effectiveReserve's staleness fallback reads as
     reserve here, as in pop.
   - WWake does both wakeOne calls in one step (removes up to two
     waiters); in the code a worker may enqueue between the two calls and
     take the second wake, which the model shows as that worker's park
     failing on the epoch or a spurious return.
   - WTime reads depth[0] and depth[1] in one step; the code does one
     relaxed load of the Frame depth, then (when zero) the lower depth.
     depth[0] # 0 takes the timeout-0 path: waitForWork(epoch, 0) yields
     and returns without parking, which the model shows as going back to
     WCheck without enqueueing. A stale zero is the case the epoch
     compare in WPark covers (the push bumps after its insert).
   - Scheduler stop sets every worker's interrupt in one step.
   - frameRunning, framePeak, queuedAt and slices are metrics only.
   - Tasks: F (Frame) is pushed once from outside (pext). Its first run
     pushes L (lower) from the worker with wake_ = true, an exec from a
     worker: L's lane was empty, so that push wakes one. It then pushes
     two Frame children C1 and C2 (WBurst1, WBurst2): C1 finds the Frame
     lane empty (depth = 1, wakes), C2 is queued behind it (depth = 2, no
     wake; the cascade of C1's pop covers it). Then F yields, or with
     Hold = TRUE waits for done[L], done[C1] and done[C2] on its worker
     without leaving (WHold; a fiber blocking its worker, the case the
     worker-push wake exists for). With Hold and N >= 3 (KidHold) C1 also
     blocks its worker until C2 ran (WKid; a fiber awaiting a sibling). F
     re-pushes itself quietly after leave until it has run FRuns times, or
     forever when Forever = TRUE. The stopper drains (waits for all tasks)
     when Drain.

   Bounds, one file per configuration, all with Drain = TRUE,
   Forever = FALSE, RHi = 1:
   - Schedule.cfg: N = 2, Rolls = 1, FRuns = 2, Spurs = 1, Hold = TRUE.
   - ScheduleNoHold.cfg: the same with Hold = FALSE.
   - ScheduleCascade.cfg: N = 3, Rolls = 0, FRuns = 1, Spurs = 0,
     Hold = TRUE (KidHold: the cascade is the only wake for C2).
   Run from this directory, one at a time:
     java -cp tla2tools.jar pcal.trans Schedule.tla
     java -Xmx3g -cp tla2tools.jar tlc2.TLC -workers 4 \
       -config <cfg> Schedule.tla
   Other configurations in Results change only these constants.

   Properties:
   - TypeOK, DepthExact (depth counts queued plus in-flight inserts/pops),
     RunningExact (lowerRunning counts floor and claim holders).
   - NoLostWakeup: never all workers parked while takeable work is queued,
     unless a push or a reserve drop is between its bump and its wake.
   - FrameFirst: a worker takes lower work only after seeing Frame empty or
     while holding the floor claim.
   - OneFloorTurn: at most one floor claim per raising of floorDue.
   - ReserveHeld: a claim never makes the claim holders exceed
     N - reserve, judged against the reserve the claiming worker read.
     Floor turns are outside the cap. A rise that lands after the read can
     leave more holders than the new cap allows: that soft over-admission
     is accepted by design (Task 4 review) and not flagged.
   - EveryTaskRuns: Drain => <>[]AllDone.
   - StopTerminates: ~Forever => <>AllExited.
     (Verified under Drain only; plain-stop liveness incomplete.)
   - CascadeWakes: a parked head while work is takeable ends, before all
     tasks are done, in two running workers, its wake, or the work taken
     (~Takeable). The last case is a quiet re-push: the re-pushing worker
     pops its own task and the head rightly stays parked.
   - LowerProgress: Forever => <>done[L] (F re-pushes forever; Clock fair).

   TLC "no error" shows the protocol holds under sequential consistency
   only; the memory orders of the code are not checked by this model.

   Floor and roll wake. The model shows the floor is needed for lower
   progress: without it a single self-re-pushing Frame task starves L at a
   reachable reserve (mutation d). It does not show the rollFrame wake
   (Schedule.cpp rollFrame, wake when next < prev) is needed: with the
   reserve clamped to 1..workers-1 the cap is at least 1 and the last lower
   runner pops again after leave, so "all parked behind the reserve" is
   unreachable in the code. Removing the wake costs latency (a parked
   worker stays parked until the next push or cascade), not liveness.

   Results (TLC 2.19, -Xmx3g -workers 4; distinct states, search depth,
   time). Rows marked "earlier model" predate the tree wake rule and the
   park guard and were not re-run:
   - Schedule.cfg (N=2, Hold=TRUE): no error; 1,495,154 distinct,
     depth 110, 7m39s.
   - ScheduleNoHold.cfg (Hold=FALSE): stopped at 15 min with no error so
     far (3,008,986 distinct); not complete.
   - ScheduleCascade.cfg (N=3, Rolls=0, FRuns=1, Spurs=0): stopped at
     15 min with no error so far (1,950,915 distinct); not complete. The
     same bounds without PROPERTIES (safety and deadlock only): no error,
     6,575,746 distinct, depth 104, 56s (complete).
   - Forever, Reserve rise (Rolls=2), Plain stop, All parked (RHi=2):
     earlier model only, see the git history of this file.
   Lost wakeup (found by W02.15, fixed in 688bcb183): a worker push at
   Frame depth 2 skips the wake (the first push already woke someone);
   without a guard the parker parks while that push is in flight and no
   wake follows. The fix makes the parker retry (timeout 0: a yield, no
   park) while Frame depth > 0 (WTime: depth[0] # 0 -> WCheck).
   Mutation (j) removes the guard and TLC finds the deadlock.
   Mutations (each in a scratch copy; Schedule.cfg unless noted):
   (j) park ignores an in-flight Frame push (no depth[0] guard in WTime)
       -> TLC deadlock check; 759,603, depth 71, 3m07s
   (k) wake on every non-quiet push (pwake := ~pquiet) -> passes with a
       changed count, 1,436,980 vs 1,495,154: the extra wakes are
       harmless, the depth-1 rule only saves wakes
   (i) no wake on a worker push (pwake := ~pquiet /\ pext) -> TLC
       deadlock check; 369,119, depth 48, 52s
   (c) no cascade wakes (WWake skip) -> passes under Schedule.cfg with
       the same count (1,495,154: at N=2 no cascade finds a waiter);
       ScheduleCascade.cfg liveness stopped at 15 min (2,128,851); the
       Cascade bounds without PROPERTIES: TLC deadlock check, 3,157,421,
       depth 65. Trace: C2 is queued, worker 1 holds at WHold for it,
       worker 2 is parked at the head of the wait queue, worker 3 runs C1;
       the depth-2 push is F's C2 from worker 1 (WBurst2), which does not
       wake, and no cascade follows.
       CascadeWakes itself is not shown violated (its liveness pass did
       not finish); the deadlock shows the cascade is needed.
   (e) floor by plain store, not CAS -> ScheduleNoHold.cfg: OneFloorTurn,
       346,197, depth 42, 1m21s (Hold=TRUE keeps two workers off the
       floor race)
   (h) lower claim as check, then a separate unconditional increment (no
       CAS) -> ScheduleNoHold.cfg: ReserveHeld, 3,479,111, depth 81,
       13m34s
   Earlier model, not re-run: (a) epoch loaded after the pop ->
   NoLostWakeup; (b) no WakeByAddressAll in wakeAll -> deadlock (the
   stopper never joins); (d) no floor, Forever, Rolls=0 -> LowerProgress;
   (f) lower set popped before Frame -> FrameFirst; (g) no wake on a
   reserve drop -> passes at reachable reserves (latency, not liveness).
   Schedule.cfg is needed for (i), (j) and (k); ScheduleNoHold.cfg for
   (e) and (h); ScheduleCascade.cfg for (c).
*)
EXTENDS Naturals, Sequences, FiniteSets, TLC

CONSTANTS N, RHi, Rolls, FRuns, Spurs, Drain, Forever, Hold
ASSUME Forever => ~Drain
ASSUME Hold \in BOOLEAN

Workers == 1..N
Pusher == N + 1
Stopper == N + 2
Spurious == N + 3
Clock == N + 4
FloorWake == N + 5
Roll == N + 6
None == 0
F == 1
L == 2
C1 == 3
C2 == 4
Tasks == {F, L, C1, C2}
LaneOf(t) == IF t = L THEN 1 ELSE 0
\* C1 blocks its worker until C2 ran (a fiber awaiting a sibling); only
\* with a third worker, so F and C1 holding leave one worker free
KidHold == Hold /\ N >= 3

InQ(w, q) == \E i \in 1..Len(q) : q[i] = w
Drop(q, i) == SubSeq(q, 1, i - 1) \o SubSeq(q, i + 1, Len(q))

(* --algorithm Schedule
variables
    sets = [s \in 0..1 |-> << >>],
    depth = [s \in 0..1 |-> 0],
    lowerRunning = 0,
    reserve = RHi,
    floorDue = FALSE,
    fresh = [w \in Workers |-> FALSE],
    waitq = << >>,
    interrupt = [w \in Workers |-> FALSE],
    done = [t \in Tasks |-> FALSE],
    held = [w \in Workers |-> "no"],
    timed = [w \in Workers |-> FALSE],
    fRuns = 0,
    rolls = 0,
    turnClaims = 0,
    overCap = FALSE,
    spurs = 0;

define
    AllDone == \A t \in Tasks : done[t]
    AllExited == \A w \in Workers : pc[w] = "Done"
    Takeable == depth[0] > 0 \/ (depth[1] > 0 /\ (floorDue \/ lowerRunning < N - reserve))
end define;

procedure push(pt = None, pext = FALSE, pquiet = FALSE)
variable pwake = FALSE;
begin
PInc:
    depth[LaneOf(pt)] := depth[LaneOf(pt)] + 1;
    pwake := ~pquiet /\ (pext \/ depth[LaneOf(pt)] = 1);
PInsert:
    sets[LaneOf(pt)] := Append(sets[LaneOf(pt)], pt);
PBump:
    fresh := [w \in Workers |-> FALSE];
    if ~pwake then
        return;
    end if;
PWake:
    if waitq # << >> then
        waitq := Tail(waitq);
    end if;
    return;
end procedure;

fair process worker \in Workers
variables task = None, sawEmpty = FALSE, readRes = 0, yield = FALSE;
begin
WCheck:
    while ~interrupt[self] do
WEpoch:
        fresh[self] := TRUE;
WFDepth:
        if depth[1] # 0 then
WFCas:
            if floorDue then
                floorDue := FALSE;
                turnClaims := turnClaims + 1;
                held[self] := "floor";
WFPop:
                if sets[1] # << >> then
                    task := Head(sets[1]);
                    sets[1] := Tail(sets[1]);
WFRun:
                    lowerRunning := lowerRunning + 1;
WFDec:
                    depth[1] := depth[1] - 1;
                    goto WCasc;
                else
                    held[self] := "no";
                end if;
            end if;
        end if;
WPop0:
        if sets[0] # << >> then
            task := Head(sets[0]);
            sets[0] := Tail(sets[0]);
WDec0:
            depth[0] := depth[0] - 1;
            goto WCasc;
        else
            sawEmpty := TRUE;
        end if;
WLDepth:
        if depth[1] # 0 then
WLRes:
            readRes := reserve;
WLClaim:
            if lowerRunning < N - readRes then
                overCap := overCap \/
                    (Cardinality({v \in Workers : held[v] = "claim"}) + 1 > N - readRes);
                lowerRunning := lowerRunning + 1;
                held[self] := "claim";
                readRes := 0;
WLPop:
                if sets[1] # << >> then
                    task := Head(sets[1]);
                    sets[1] := Tail(sets[1]);
WLDec:
                    depth[1] := depth[1] - 1;
                    sawEmpty := FALSE;
WLStamp:
                    floorDue := FALSE;
                    goto WCasc;
                else
WLUndo:
                    lowerRunning := lowerRunning - 1;
                    held[self] := "no";
                end if;
            end if;
        end if;
WFail:
        sawEmpty := FALSE;
        readRes := 0;
        if interrupt[self] then
            goto WExit;
        end if;
WTime:
        \* parkTimeoutMillis: 0 while Frame depth counts a push in flight
        if depth[0] # 0 then
            goto WCheck;
        else
            timed[self] := depth[1] # 0;
        end if;
WPark:
        if fresh[self] then
            waitq := Append(waitq, self);
        else
            timed[self] := FALSE;
            goto WCheck;
        end if;
WSleep:
        await ~InQ(self, waitq);
        timed[self] := FALSE;
        goto WCheck;
WCasc:
        if Takeable then
WWake:
            waitq := IF Len(waitq) <= 2 THEN << >> ELSE SubSeq(waitq, 3, Len(waitq));
        end if;
WRun:
        if task = F /\ fRuns = 0 then
            fRuns := 1;
            yield := Forever \/ 1 < FRuns;
            call push(L, FALSE, FALSE);
            goto WBurst1;
        elsif task = F then
            yield := Forever \/ fRuns + 1 < FRuns;
            fRuns := IF Forever THEN fRuns ELSE fRuns + 1;
        elsif task = C1 /\ KidHold then
            goto WKid;
        else
            done[task] := TRUE;
        end if;
WLeave:
        if LaneOf(task) = 1 then
            lowerRunning := lowerRunning - 1;
        end if;
        held[self] := "no";
WYield:
        if yield then
            task := None;
            yield := FALSE;
            call push(F, FALSE, TRUE);
            goto WCheck;
        elsif task = F then
            done[F] := TRUE;
            task := None;
        else
            task := None;
        end if;
    end while;
WExit:
    skip;
    goto Done;
WBurst1:
    \* two Frame children from the same run: C1 finds its lane empty
    \* (depth = 1, wakes), C2 queued behind it does not wake (cascade)
    call push(C1, FALSE, FALSE);
    goto WBurst2;
WBurst2:
    call push(C2, FALSE, FALSE);
    goto WHold;
WHold:
    \* F's first run waits for its children on its worker (Hold), or yields
    if Hold then
        await done[L] /\ done[C1] /\ done[C2];
    end if;
    goto WLeave;
WKid:
    await done[C2];
    done[C1] := TRUE;
    goto WLeave;
end process;

fair process pusher = Pusher
begin
PStart:
    call push(F, TRUE, FALSE);
end process;

fair process stopper = Stopper
begin
SAwait:
    await (~Drain /\ ~Forever) \/ AllDone;
SReq:
    interrupt := [w \in Workers |-> TRUE];
SBump:
    fresh := [w \in Workers |-> FALSE];
SWakeAll:
    waitq := << >>;
SJoin:
    await AllExited;
end process;

fair process clock = Clock
begin
CTick:
    while ~AllExited do
        await (depth[1] # 0 /\ ~floorDue) \/ AllExited;
        if ~AllExited then
            floorDue := TRUE;
            turnClaims := 0;
        end if;
    end while;
end process;

fair process floorWake = FloorWake
begin
FW:
    while ~AllExited do
        await (floorDue /\ depth[1] # 0 /\ \E j \in 1..Len(waitq) : timed[waitq[j]])
              \/ AllExited;
        if ~AllExited then
            with j \in {k \in 1..Len(waitq) : timed[waitq[k]]} do
                waitq := Drop(waitq, j);
            end with;
        end if;
    end while;
end process;

fair process roll = Roll
variables drop = FALSE;
begin
RPick:-
    while rolls < Rolls do
        rolls := rolls + 1;
        with r \in {0, RHi} do
            drop := r < reserve;
            reserve := r;
        end with;
RCheck:
        if drop /\ depth[1] # 0 then
RBump:
            fresh := [w \in Workers |-> FALSE];
RWake:
            if waitq # << >> then
                waitq := Tail(waitq);
            end if;
        end if;
RReset:
        drop := FALSE;
    end while;
end process;

process spurious = Spurious
begin
SpLoop:
    while TRUE do
        either
            await waitq # << >> /\ spurs < Spurs;
            spurs := spurs + 1;
            with j \in 1..Len(waitq) do
                waitq := Drop(waitq, j);
            end with;
        or
            await AllExited;
            goto Done;
        end either;
    end while;
end process;

end algorithm; *)
\* BEGIN TRANSLATION
VARIABLES sets, depth, lowerRunning, reserve, floorDue, fresh, waitq, 
          interrupt, done, held, timed, fRuns, rolls, turnClaims, overCap, 
          spurs, pc, stack

(* define statement *)
AllDone == \A t \in Tasks : done[t]
AllExited == \A w \in Workers : pc[w] = "Done"
Takeable == depth[0] > 0 \/ (depth[1] > 0 /\ (floorDue \/ lowerRunning < N - reserve))

VARIABLES pt, pext, pquiet, pwake, task, sawEmpty, readRes, yield, drop

vars == << sets, depth, lowerRunning, reserve, floorDue, fresh, waitq, 
           interrupt, done, held, timed, fRuns, rolls, turnClaims, overCap, 
           spurs, pc, stack, pt, pext, pquiet, pwake, task, sawEmpty, readRes, 
           yield, drop >>

ProcSet == (Workers) \cup {Pusher} \cup {Stopper} \cup {Clock} \cup {FloorWake} \cup {Roll} \cup {Spurious}

Init == (* Global variables *)
        /\ sets = [s \in 0..1 |-> << >>]
        /\ depth = [s \in 0..1 |-> 0]
        /\ lowerRunning = 0
        /\ reserve = RHi
        /\ floorDue = FALSE
        /\ fresh = [w \in Workers |-> FALSE]
        /\ waitq = << >>
        /\ interrupt = [w \in Workers |-> FALSE]
        /\ done = [t \in Tasks |-> FALSE]
        /\ held = [w \in Workers |-> "no"]
        /\ timed = [w \in Workers |-> FALSE]
        /\ fRuns = 0
        /\ rolls = 0
        /\ turnClaims = 0
        /\ overCap = FALSE
        /\ spurs = 0
        (* Procedure push *)
        /\ pt = [ self \in ProcSet |-> None]
        /\ pext = [ self \in ProcSet |-> FALSE]
        /\ pquiet = [ self \in ProcSet |-> FALSE]
        /\ pwake = [ self \in ProcSet |-> FALSE]
        (* Process worker *)
        /\ task = [self \in Workers |-> None]
        /\ sawEmpty = [self \in Workers |-> FALSE]
        /\ readRes = [self \in Workers |-> 0]
        /\ yield = [self \in Workers |-> FALSE]
        (* Process roll *)
        /\ drop = FALSE
        /\ stack = [self \in ProcSet |-> << >>]
        /\ pc = [self \in ProcSet |-> CASE self \in Workers -> "WCheck"
                                        [] self = Pusher -> "PStart"
                                        [] self = Stopper -> "SAwait"
                                        [] self = Clock -> "CTick"
                                        [] self = FloorWake -> "FW"
                                        [] self = Roll -> "RPick"
                                        [] self = Spurious -> "SpLoop"]

PInc(self) == /\ pc[self] = "PInc"
              /\ depth' = [depth EXCEPT ![LaneOf(pt[self])] = depth[LaneOf(pt[self])] + 1]
              /\ pwake' = [pwake EXCEPT ![self] = ~pquiet[self] /\ (pext[self] \/ depth'[LaneOf(pt[self])] = 1)]
              /\ pc' = [pc EXCEPT ![self] = "PInsert"]
              /\ UNCHANGED << sets, lowerRunning, reserve, floorDue, fresh, 
                              waitq, interrupt, done, held, timed, fRuns, 
                              rolls, turnClaims, overCap, spurs, stack, pt, 
                              pext, pquiet, task, sawEmpty, readRes, yield, 
                              drop >>

PInsert(self) == /\ pc[self] = "PInsert"
                 /\ sets' = [sets EXCEPT ![LaneOf(pt[self])] = Append(sets[LaneOf(pt[self])], pt[self])]
                 /\ pc' = [pc EXCEPT ![self] = "PBump"]
                 /\ UNCHANGED << depth, lowerRunning, reserve, floorDue, fresh, 
                                 waitq, interrupt, done, held, timed, fRuns, 
                                 rolls, turnClaims, overCap, spurs, stack, pt, 
                                 pext, pquiet, pwake, task, sawEmpty, readRes, 
                                 yield, drop >>

PBump(self) == /\ pc[self] = "PBump"
               /\ fresh' = [w \in Workers |-> FALSE]
               /\ IF ~pwake[self]
                     THEN /\ pc' = [pc EXCEPT ![self] = Head(stack[self]).pc]
                          /\ pwake' = [pwake EXCEPT ![self] = Head(stack[self]).pwake]
                          /\ pt' = [pt EXCEPT ![self] = Head(stack[self]).pt]
                          /\ pext' = [pext EXCEPT ![self] = Head(stack[self]).pext]
                          /\ pquiet' = [pquiet EXCEPT ![self] = Head(stack[self]).pquiet]
                          /\ stack' = [stack EXCEPT ![self] = Tail(stack[self])]
                     ELSE /\ pc' = [pc EXCEPT ![self] = "PWake"]
                          /\ UNCHANGED << stack, pt, pext, pquiet, pwake >>
               /\ UNCHANGED << sets, depth, lowerRunning, reserve, floorDue, 
                               waitq, interrupt, done, held, timed, fRuns, 
                               rolls, turnClaims, overCap, spurs, task, 
                               sawEmpty, readRes, yield, drop >>

PWake(self) == /\ pc[self] = "PWake"
               /\ IF waitq # << >>
                     THEN /\ waitq' = Tail(waitq)
                     ELSE /\ TRUE
                          /\ waitq' = waitq
               /\ pc' = [pc EXCEPT ![self] = Head(stack[self]).pc]
               /\ pwake' = [pwake EXCEPT ![self] = Head(stack[self]).pwake]
               /\ pt' = [pt EXCEPT ![self] = Head(stack[self]).pt]
               /\ pext' = [pext EXCEPT ![self] = Head(stack[self]).pext]
               /\ pquiet' = [pquiet EXCEPT ![self] = Head(stack[self]).pquiet]
               /\ stack' = [stack EXCEPT ![self] = Tail(stack[self])]
               /\ UNCHANGED << sets, depth, lowerRunning, reserve, floorDue, 
                               fresh, interrupt, done, held, timed, fRuns, 
                               rolls, turnClaims, overCap, spurs, task, 
                               sawEmpty, readRes, yield, drop >>

push(self) == PInc(self) \/ PInsert(self) \/ PBump(self) \/ PWake(self)

WCheck(self) == /\ pc[self] = "WCheck"
                /\ IF ~interrupt[self]
                      THEN /\ pc' = [pc EXCEPT ![self] = "WEpoch"]
                      ELSE /\ pc' = [pc EXCEPT ![self] = "WExit"]
                /\ UNCHANGED << sets, depth, lowerRunning, reserve, floorDue, 
                                fresh, waitq, interrupt, done, held, timed, 
                                fRuns, rolls, turnClaims, overCap, spurs, 
                                stack, pt, pext, pquiet, pwake, task, sawEmpty, 
                                readRes, yield, drop >>

WEpoch(self) == /\ pc[self] = "WEpoch"
                /\ fresh' = [fresh EXCEPT ![self] = TRUE]
                /\ pc' = [pc EXCEPT ![self] = "WFDepth"]
                /\ UNCHANGED << sets, depth, lowerRunning, reserve, floorDue, 
                                waitq, interrupt, done, held, timed, fRuns, 
                                rolls, turnClaims, overCap, spurs, stack, pt, 
                                pext, pquiet, pwake, task, sawEmpty, readRes, 
                                yield, drop >>

WFDepth(self) == /\ pc[self] = "WFDepth"
                 /\ IF depth[1] # 0
                       THEN /\ pc' = [pc EXCEPT ![self] = "WFCas"]
                       ELSE /\ pc' = [pc EXCEPT ![self] = "WPop0"]
                 /\ UNCHANGED << sets, depth, lowerRunning, reserve, floorDue, 
                                 fresh, waitq, interrupt, done, held, timed, 
                                 fRuns, rolls, turnClaims, overCap, spurs, 
                                 stack, pt, pext, pquiet, pwake, task, 
                                 sawEmpty, readRes, yield, drop >>

WFCas(self) == /\ pc[self] = "WFCas"
               /\ IF floorDue
                     THEN /\ floorDue' = FALSE
                          /\ turnClaims' = turnClaims + 1
                          /\ held' = [held EXCEPT ![self] = "floor"]
                          /\ pc' = [pc EXCEPT ![self] = "WFPop"]
                     ELSE /\ pc' = [pc EXCEPT ![self] = "WPop0"]
                          /\ UNCHANGED << floorDue, held, turnClaims >>
               /\ UNCHANGED << sets, depth, lowerRunning, reserve, fresh, 
                               waitq, interrupt, done, timed, fRuns, rolls, 
                               overCap, spurs, stack, pt, pext, pquiet, pwake, 
                               task, sawEmpty, readRes, yield, drop >>

WFPop(self) == /\ pc[self] = "WFPop"
               /\ IF sets[1] # << >>
                     THEN /\ task' = [task EXCEPT ![self] = Head(sets[1])]
                          /\ sets' = [sets EXCEPT ![1] = Tail(sets[1])]
                          /\ pc' = [pc EXCEPT ![self] = "WFRun"]
                          /\ held' = held
                     ELSE /\ held' = [held EXCEPT ![self] = "no"]
                          /\ pc' = [pc EXCEPT ![self] = "WPop0"]
                          /\ UNCHANGED << sets, task >>
               /\ UNCHANGED << depth, lowerRunning, reserve, floorDue, fresh, 
                               waitq, interrupt, done, timed, fRuns, rolls, 
                               turnClaims, overCap, spurs, stack, pt, pext, 
                               pquiet, pwake, sawEmpty, readRes, yield, drop >>

WFRun(self) == /\ pc[self] = "WFRun"
               /\ lowerRunning' = lowerRunning + 1
               /\ pc' = [pc EXCEPT ![self] = "WFDec"]
               /\ UNCHANGED << sets, depth, reserve, floorDue, fresh, waitq, 
                               interrupt, done, held, timed, fRuns, rolls, 
                               turnClaims, overCap, spurs, stack, pt, pext, 
                               pquiet, pwake, task, sawEmpty, readRes, yield, 
                               drop >>

WFDec(self) == /\ pc[self] = "WFDec"
               /\ depth' = [depth EXCEPT ![1] = depth[1] - 1]
               /\ pc' = [pc EXCEPT ![self] = "WCasc"]
               /\ UNCHANGED << sets, lowerRunning, reserve, floorDue, fresh, 
                               waitq, interrupt, done, held, timed, fRuns, 
                               rolls, turnClaims, overCap, spurs, stack, pt, 
                               pext, pquiet, pwake, task, sawEmpty, readRes, 
                               yield, drop >>

WPop0(self) == /\ pc[self] = "WPop0"
               /\ IF sets[0] # << >>
                     THEN /\ task' = [task EXCEPT ![self] = Head(sets[0])]
                          /\ sets' = [sets EXCEPT ![0] = Tail(sets[0])]
                          /\ pc' = [pc EXCEPT ![self] = "WDec0"]
                          /\ UNCHANGED sawEmpty
                     ELSE /\ sawEmpty' = [sawEmpty EXCEPT ![self] = TRUE]
                          /\ pc' = [pc EXCEPT ![self] = "WLDepth"]
                          /\ UNCHANGED << sets, task >>
               /\ UNCHANGED << depth, lowerRunning, reserve, floorDue, fresh, 
                               waitq, interrupt, done, held, timed, fRuns, 
                               rolls, turnClaims, overCap, spurs, stack, pt, 
                               pext, pquiet, pwake, readRes, yield, drop >>

WDec0(self) == /\ pc[self] = "WDec0"
               /\ depth' = [depth EXCEPT ![0] = depth[0] - 1]
               /\ pc' = [pc EXCEPT ![self] = "WCasc"]
               /\ UNCHANGED << sets, lowerRunning, reserve, floorDue, fresh, 
                               waitq, interrupt, done, held, timed, fRuns, 
                               rolls, turnClaims, overCap, spurs, stack, pt, 
                               pext, pquiet, pwake, task, sawEmpty, readRes, 
                               yield, drop >>

WLDepth(self) == /\ pc[self] = "WLDepth"
                 /\ IF depth[1] # 0
                       THEN /\ pc' = [pc EXCEPT ![self] = "WLRes"]
                       ELSE /\ pc' = [pc EXCEPT ![self] = "WFail"]
                 /\ UNCHANGED << sets, depth, lowerRunning, reserve, floorDue, 
                                 fresh, waitq, interrupt, done, held, timed, 
                                 fRuns, rolls, turnClaims, overCap, spurs, 
                                 stack, pt, pext, pquiet, pwake, task, 
                                 sawEmpty, readRes, yield, drop >>

WLRes(self) == /\ pc[self] = "WLRes"
               /\ readRes' = [readRes EXCEPT ![self] = reserve]
               /\ pc' = [pc EXCEPT ![self] = "WLClaim"]
               /\ UNCHANGED << sets, depth, lowerRunning, reserve, floorDue, 
                               fresh, waitq, interrupt, done, held, timed, 
                               fRuns, rolls, turnClaims, overCap, spurs, stack, 
                               pt, pext, pquiet, pwake, task, sawEmpty, yield, 
                               drop >>

WLClaim(self) == /\ pc[self] = "WLClaim"
                 /\ IF lowerRunning < N - readRes[self]
                       THEN /\ overCap' = (       overCap \/
                                           (Cardinality({v \in Workers : held[v] = "claim"}) + 1 > N - readRes[self]))
                            /\ lowerRunning' = lowerRunning + 1
                            /\ held' = [held EXCEPT ![self] = "claim"]
                            /\ readRes' = [readRes EXCEPT ![self] = 0]
                            /\ pc' = [pc EXCEPT ![self] = "WLPop"]
                       ELSE /\ pc' = [pc EXCEPT ![self] = "WFail"]
                            /\ UNCHANGED << lowerRunning, held, overCap, 
                                            readRes >>
                 /\ UNCHANGED << sets, depth, reserve, floorDue, fresh, waitq, 
                                 interrupt, done, timed, fRuns, rolls, 
                                 turnClaims, spurs, stack, pt, pext, pquiet, 
                                 pwake, task, sawEmpty, yield, drop >>

WLPop(self) == /\ pc[self] = "WLPop"
               /\ IF sets[1] # << >>
                     THEN /\ task' = [task EXCEPT ![self] = Head(sets[1])]
                          /\ sets' = [sets EXCEPT ![1] = Tail(sets[1])]
                          /\ pc' = [pc EXCEPT ![self] = "WLDec"]
                     ELSE /\ pc' = [pc EXCEPT ![self] = "WLUndo"]
                          /\ UNCHANGED << sets, task >>
               /\ UNCHANGED << depth, lowerRunning, reserve, floorDue, fresh, 
                               waitq, interrupt, done, held, timed, fRuns, 
                               rolls, turnClaims, overCap, spurs, stack, pt, 
                               pext, pquiet, pwake, sawEmpty, readRes, yield, 
                               drop >>

WLDec(self) == /\ pc[self] = "WLDec"
               /\ depth' = [depth EXCEPT ![1] = depth[1] - 1]
               /\ sawEmpty' = [sawEmpty EXCEPT ![self] = FALSE]
               /\ pc' = [pc EXCEPT ![self] = "WLStamp"]
               /\ UNCHANGED << sets, lowerRunning, reserve, floorDue, fresh, 
                               waitq, interrupt, done, held, timed, fRuns, 
                               rolls, turnClaims, overCap, spurs, stack, pt, 
                               pext, pquiet, pwake, task, readRes, yield, drop >>

WLStamp(self) == /\ pc[self] = "WLStamp"
                 /\ floorDue' = FALSE
                 /\ pc' = [pc EXCEPT ![self] = "WCasc"]
                 /\ UNCHANGED << sets, depth, lowerRunning, reserve, fresh, 
                                 waitq, interrupt, done, held, timed, fRuns, 
                                 rolls, turnClaims, overCap, spurs, stack, pt, 
                                 pext, pquiet, pwake, task, sawEmpty, readRes, 
                                 yield, drop >>

WLUndo(self) == /\ pc[self] = "WLUndo"
                /\ lowerRunning' = lowerRunning - 1
                /\ held' = [held EXCEPT ![self] = "no"]
                /\ pc' = [pc EXCEPT ![self] = "WFail"]
                /\ UNCHANGED << sets, depth, reserve, floorDue, fresh, waitq, 
                                interrupt, done, timed, fRuns, rolls, 
                                turnClaims, overCap, spurs, stack, pt, pext, 
                                pquiet, pwake, task, sawEmpty, readRes, yield, 
                                drop >>

WFail(self) == /\ pc[self] = "WFail"
               /\ sawEmpty' = [sawEmpty EXCEPT ![self] = FALSE]
               /\ readRes' = [readRes EXCEPT ![self] = 0]
               /\ IF interrupt[self]
                     THEN /\ pc' = [pc EXCEPT ![self] = "WExit"]
                     ELSE /\ pc' = [pc EXCEPT ![self] = "WTime"]
               /\ UNCHANGED << sets, depth, lowerRunning, reserve, floorDue, 
                               fresh, waitq, interrupt, done, held, timed, 
                               fRuns, rolls, turnClaims, overCap, spurs, stack, 
                               pt, pext, pquiet, pwake, task, yield, drop >>

WTime(self) == /\ pc[self] = "WTime"
               /\ IF depth[0] # 0
                     THEN /\ pc' = [pc EXCEPT ![self] = "WCheck"]
                          /\ timed' = timed
                     ELSE /\ timed' = [timed EXCEPT ![self] = depth[1] # 0]
                          /\ pc' = [pc EXCEPT ![self] = "WPark"]
               /\ UNCHANGED << sets, depth, lowerRunning, reserve, floorDue, 
                               fresh, waitq, interrupt, done, held, fRuns, 
                               rolls, turnClaims, overCap, spurs, stack, pt, 
                               pext, pquiet, pwake, task, sawEmpty, readRes, 
                               yield, drop >>

WPark(self) == /\ pc[self] = "WPark"
               /\ IF fresh[self]
                     THEN /\ waitq' = Append(waitq, self)
                          /\ pc' = [pc EXCEPT ![self] = "WSleep"]
                          /\ timed' = timed
                     ELSE /\ timed' = [timed EXCEPT ![self] = FALSE]
                          /\ pc' = [pc EXCEPT ![self] = "WCheck"]
                          /\ waitq' = waitq
               /\ UNCHANGED << sets, depth, lowerRunning, reserve, floorDue, 
                               fresh, interrupt, done, held, fRuns, rolls, 
                               turnClaims, overCap, spurs, stack, pt, pext, 
                               pquiet, pwake, task, sawEmpty, readRes, yield, 
                               drop >>

WSleep(self) == /\ pc[self] = "WSleep"
                /\ ~InQ(self, waitq)
                /\ timed' = [timed EXCEPT ![self] = FALSE]
                /\ pc' = [pc EXCEPT ![self] = "WCheck"]
                /\ UNCHANGED << sets, depth, lowerRunning, reserve, floorDue, 
                                fresh, waitq, interrupt, done, held, fRuns, 
                                rolls, turnClaims, overCap, spurs, stack, pt, 
                                pext, pquiet, pwake, task, sawEmpty, readRes, 
                                yield, drop >>

WCasc(self) == /\ pc[self] = "WCasc"
               /\ IF Takeable
                     THEN /\ pc' = [pc EXCEPT ![self] = "WWake"]
                     ELSE /\ pc' = [pc EXCEPT ![self] = "WRun"]
               /\ UNCHANGED << sets, depth, lowerRunning, reserve, floorDue, 
                               fresh, waitq, interrupt, done, held, timed, 
                               fRuns, rolls, turnClaims, overCap, spurs, stack, 
                               pt, pext, pquiet, pwake, task, sawEmpty, 
                               readRes, yield, drop >>

WWake(self) == /\ pc[self] = "WWake"
               /\ waitq' = (IF Len(waitq) <= 2 THEN << >> ELSE SubSeq(waitq, 3, Len(waitq)))
               /\ pc' = [pc EXCEPT ![self] = "WRun"]
               /\ UNCHANGED << sets, depth, lowerRunning, reserve, floorDue, 
                               fresh, interrupt, done, held, timed, fRuns, 
                               rolls, turnClaims, overCap, spurs, stack, pt, 
                               pext, pquiet, pwake, task, sawEmpty, readRes, 
                               yield, drop >>

WRun(self) == /\ pc[self] = "WRun"
              /\ IF task[self] = F /\ fRuns = 0
                    THEN /\ fRuns' = 1
                         /\ yield' = [yield EXCEPT ![self] = Forever \/ 1 < FRuns]
                         /\ /\ pext' = [pext EXCEPT ![self] = FALSE]
                            /\ pquiet' = [pquiet EXCEPT ![self] = FALSE]
                            /\ pt' = [pt EXCEPT ![self] = L]
                            /\ stack' = [stack EXCEPT ![self] = << [ procedure |->  "push",
                                                                     pc        |->  "WBurst1",
                                                                     pwake     |->  pwake[self],
                                                                     pt        |->  pt[self],
                                                                     pext      |->  pext[self],
                                                                     pquiet    |->  pquiet[self] ] >>
                                                                 \o stack[self]]
                         /\ pwake' = [pwake EXCEPT ![self] = FALSE]
                         /\ pc' = [pc EXCEPT ![self] = "PInc"]
                         /\ done' = done
                    ELSE /\ IF task[self] = F
                               THEN /\ yield' = [yield EXCEPT ![self] = Forever \/ fRuns + 1 < FRuns]
                                    /\ fRuns' = IF Forever THEN fRuns ELSE fRuns + 1
                                    /\ pc' = [pc EXCEPT ![self] = "WLeave"]
                                    /\ done' = done
                               ELSE /\ IF task[self] = C1 /\ KidHold
                                          THEN /\ pc' = [pc EXCEPT ![self] = "WKid"]
                                               /\ done' = done
                                          ELSE /\ done' = [done EXCEPT ![task[self]] = TRUE]
                                               /\ pc' = [pc EXCEPT ![self] = "WLeave"]
                                    /\ UNCHANGED << fRuns, yield >>
                         /\ UNCHANGED << stack, pt, pext, pquiet, pwake >>
              /\ UNCHANGED << sets, depth, lowerRunning, reserve, floorDue, 
                              fresh, waitq, interrupt, held, timed, rolls, 
                              turnClaims, overCap, spurs, task, sawEmpty, 
                              readRes, drop >>

WLeave(self) == /\ pc[self] = "WLeave"
                /\ IF LaneOf(task[self]) = 1
                      THEN /\ lowerRunning' = lowerRunning - 1
                      ELSE /\ TRUE
                           /\ UNCHANGED lowerRunning
                /\ held' = [held EXCEPT ![self] = "no"]
                /\ pc' = [pc EXCEPT ![self] = "WYield"]
                /\ UNCHANGED << sets, depth, reserve, floorDue, fresh, waitq, 
                                interrupt, done, timed, fRuns, rolls, 
                                turnClaims, overCap, spurs, stack, pt, pext, 
                                pquiet, pwake, task, sawEmpty, readRes, yield, 
                                drop >>

WYield(self) == /\ pc[self] = "WYield"
                /\ IF yield[self]
                      THEN /\ task' = [task EXCEPT ![self] = None]
                           /\ yield' = [yield EXCEPT ![self] = FALSE]
                           /\ /\ pext' = [pext EXCEPT ![self] = FALSE]
                              /\ pquiet' = [pquiet EXCEPT ![self] = TRUE]
                              /\ pt' = [pt EXCEPT ![self] = F]
                              /\ stack' = [stack EXCEPT ![self] = << [ procedure |->  "push",
                                                                       pc        |->  "WCheck",
                                                                       pwake     |->  pwake[self],
                                                                       pt        |->  pt[self],
                                                                       pext      |->  pext[self],
                                                                       pquiet    |->  pquiet[self] ] >>
                                                                   \o stack[self]]
                           /\ pwake' = [pwake EXCEPT ![self] = FALSE]
                           /\ pc' = [pc EXCEPT ![self] = "PInc"]
                           /\ done' = done
                      ELSE /\ IF task[self] = F
                                 THEN /\ done' = [done EXCEPT ![F] = TRUE]
                                      /\ task' = [task EXCEPT ![self] = None]
                                 ELSE /\ task' = [task EXCEPT ![self] = None]
                                      /\ done' = done
                           /\ pc' = [pc EXCEPT ![self] = "WCheck"]
                           /\ UNCHANGED << stack, pt, pext, pquiet, pwake, 
                                           yield >>
                /\ UNCHANGED << sets, depth, lowerRunning, reserve, floorDue, 
                                fresh, waitq, interrupt, held, timed, fRuns, 
                                rolls, turnClaims, overCap, spurs, sawEmpty, 
                                readRes, drop >>

WExit(self) == /\ pc[self] = "WExit"
               /\ TRUE
               /\ pc' = [pc EXCEPT ![self] = "Done"]
               /\ UNCHANGED << sets, depth, lowerRunning, reserve, floorDue, 
                               fresh, waitq, interrupt, done, held, timed, 
                               fRuns, rolls, turnClaims, overCap, spurs, stack, 
                               pt, pext, pquiet, pwake, task, sawEmpty, 
                               readRes, yield, drop >>

WBurst1(self) == /\ pc[self] = "WBurst1"
                 /\ /\ pext' = [pext EXCEPT ![self] = FALSE]
                    /\ pquiet' = [pquiet EXCEPT ![self] = FALSE]
                    /\ pt' = [pt EXCEPT ![self] = C1]
                    /\ stack' = [stack EXCEPT ![self] = << [ procedure |->  "push",
                                                             pc        |->  "WBurst2",
                                                             pwake     |->  pwake[self],
                                                             pt        |->  pt[self],
                                                             pext      |->  pext[self],
                                                             pquiet    |->  pquiet[self] ] >>
                                                         \o stack[self]]
                 /\ pwake' = [pwake EXCEPT ![self] = FALSE]
                 /\ pc' = [pc EXCEPT ![self] = "PInc"]
                 /\ UNCHANGED << sets, depth, lowerRunning, reserve, floorDue, 
                                 fresh, waitq, interrupt, done, held, timed, 
                                 fRuns, rolls, turnClaims, overCap, spurs, 
                                 task, sawEmpty, readRes, yield, drop >>

WBurst2(self) == /\ pc[self] = "WBurst2"
                 /\ /\ pext' = [pext EXCEPT ![self] = FALSE]
                    /\ pquiet' = [pquiet EXCEPT ![self] = FALSE]
                    /\ pt' = [pt EXCEPT ![self] = C2]
                    /\ stack' = [stack EXCEPT ![self] = << [ procedure |->  "push",
                                                             pc        |->  "WHold",
                                                             pwake     |->  pwake[self],
                                                             pt        |->  pt[self],
                                                             pext      |->  pext[self],
                                                             pquiet    |->  pquiet[self] ] >>
                                                         \o stack[self]]
                 /\ pwake' = [pwake EXCEPT ![self] = FALSE]
                 /\ pc' = [pc EXCEPT ![self] = "PInc"]
                 /\ UNCHANGED << sets, depth, lowerRunning, reserve, floorDue, 
                                 fresh, waitq, interrupt, done, held, timed, 
                                 fRuns, rolls, turnClaims, overCap, spurs, 
                                 task, sawEmpty, readRes, yield, drop >>

WHold(self) == /\ pc[self] = "WHold"
               /\ IF Hold
                     THEN /\ done[L] /\ done[C1] /\ done[C2]
                     ELSE /\ TRUE
               /\ pc' = [pc EXCEPT ![self] = "WLeave"]
               /\ UNCHANGED << sets, depth, lowerRunning, reserve, floorDue, 
                               fresh, waitq, interrupt, done, held, timed, 
                               fRuns, rolls, turnClaims, overCap, spurs, stack, 
                               pt, pext, pquiet, pwake, task, sawEmpty, 
                               readRes, yield, drop >>

WKid(self) == /\ pc[self] = "WKid"
              /\ done[C2]
              /\ done' = [done EXCEPT ![C1] = TRUE]
              /\ pc' = [pc EXCEPT ![self] = "WLeave"]
              /\ UNCHANGED << sets, depth, lowerRunning, reserve, floorDue, 
                              fresh, waitq, interrupt, held, timed, fRuns, 
                              rolls, turnClaims, overCap, spurs, stack, pt, 
                              pext, pquiet, pwake, task, sawEmpty, readRes, 
                              yield, drop >>

worker(self) == WCheck(self) \/ WEpoch(self) \/ WFDepth(self)
                   \/ WFCas(self) \/ WFPop(self) \/ WFRun(self)
                   \/ WFDec(self) \/ WPop0(self) \/ WDec0(self)
                   \/ WLDepth(self) \/ WLRes(self) \/ WLClaim(self)
                   \/ WLPop(self) \/ WLDec(self) \/ WLStamp(self)
                   \/ WLUndo(self) \/ WFail(self) \/ WTime(self)
                   \/ WPark(self) \/ WSleep(self) \/ WCasc(self)
                   \/ WWake(self) \/ WRun(self) \/ WLeave(self)
                   \/ WYield(self) \/ WExit(self) \/ WBurst1(self)
                   \/ WBurst2(self) \/ WHold(self) \/ WKid(self)

PStart == /\ pc[Pusher] = "PStart"
          /\ /\ pext' = [pext EXCEPT ![Pusher] = TRUE]
             /\ pquiet' = [pquiet EXCEPT ![Pusher] = FALSE]
             /\ pt' = [pt EXCEPT ![Pusher] = F]
             /\ stack' = [stack EXCEPT ![Pusher] = << [ procedure |->  "push",
                                                        pc        |->  "Done",
                                                        pwake     |->  pwake[Pusher],
                                                        pt        |->  pt[Pusher],
                                                        pext      |->  pext[Pusher],
                                                        pquiet    |->  pquiet[Pusher] ] >>
                                                    \o stack[Pusher]]
          /\ pwake' = [pwake EXCEPT ![Pusher] = FALSE]
          /\ pc' = [pc EXCEPT ![Pusher] = "PInc"]
          /\ UNCHANGED << sets, depth, lowerRunning, reserve, floorDue, fresh, 
                          waitq, interrupt, done, held, timed, fRuns, rolls, 
                          turnClaims, overCap, spurs, task, sawEmpty, readRes, 
                          yield, drop >>

pusher == PStart

SAwait == /\ pc[Stopper] = "SAwait"
          /\ (~Drain /\ ~Forever) \/ AllDone
          /\ pc' = [pc EXCEPT ![Stopper] = "SReq"]
          /\ UNCHANGED << sets, depth, lowerRunning, reserve, floorDue, fresh, 
                          waitq, interrupt, done, held, timed, fRuns, rolls, 
                          turnClaims, overCap, spurs, stack, pt, pext, pquiet, 
                          pwake, task, sawEmpty, readRes, yield, drop >>

SReq == /\ pc[Stopper] = "SReq"
        /\ interrupt' = [w \in Workers |-> TRUE]
        /\ pc' = [pc EXCEPT ![Stopper] = "SBump"]
        /\ UNCHANGED << sets, depth, lowerRunning, reserve, floorDue, fresh, 
                        waitq, done, held, timed, fRuns, rolls, turnClaims, 
                        overCap, spurs, stack, pt, pext, pquiet, pwake, task, 
                        sawEmpty, readRes, yield, drop >>

SBump == /\ pc[Stopper] = "SBump"
         /\ fresh' = [w \in Workers |-> FALSE]
         /\ pc' = [pc EXCEPT ![Stopper] = "SWakeAll"]
         /\ UNCHANGED << sets, depth, lowerRunning, reserve, floorDue, waitq, 
                         interrupt, done, held, timed, fRuns, rolls, 
                         turnClaims, overCap, spurs, stack, pt, pext, pquiet, 
                         pwake, task, sawEmpty, readRes, yield, drop >>

SWakeAll == /\ pc[Stopper] = "SWakeAll"
            /\ waitq' = << >>
            /\ pc' = [pc EXCEPT ![Stopper] = "SJoin"]
            /\ UNCHANGED << sets, depth, lowerRunning, reserve, floorDue, 
                            fresh, interrupt, done, held, timed, fRuns, rolls, 
                            turnClaims, overCap, spurs, stack, pt, pext, 
                            pquiet, pwake, task, sawEmpty, readRes, yield, 
                            drop >>

SJoin == /\ pc[Stopper] = "SJoin"
         /\ AllExited
         /\ pc' = [pc EXCEPT ![Stopper] = "Done"]
         /\ UNCHANGED << sets, depth, lowerRunning, reserve, floorDue, fresh, 
                         waitq, interrupt, done, held, timed, fRuns, rolls, 
                         turnClaims, overCap, spurs, stack, pt, pext, pquiet, 
                         pwake, task, sawEmpty, readRes, yield, drop >>

stopper == SAwait \/ SReq \/ SBump \/ SWakeAll \/ SJoin

CTick == /\ pc[Clock] = "CTick"
         /\ IF ~AllExited
               THEN /\ (depth[1] # 0 /\ ~floorDue) \/ AllExited
                    /\ IF ~AllExited
                          THEN /\ floorDue' = TRUE
                               /\ turnClaims' = 0
                          ELSE /\ TRUE
                               /\ UNCHANGED << floorDue, turnClaims >>
                    /\ pc' = [pc EXCEPT ![Clock] = "CTick"]
               ELSE /\ pc' = [pc EXCEPT ![Clock] = "Done"]
                    /\ UNCHANGED << floorDue, turnClaims >>
         /\ UNCHANGED << sets, depth, lowerRunning, reserve, fresh, waitq, 
                         interrupt, done, held, timed, fRuns, rolls, overCap, 
                         spurs, stack, pt, pext, pquiet, pwake, task, sawEmpty, 
                         readRes, yield, drop >>

clock == CTick

FW == /\ pc[FloorWake] = "FW"
      /\ IF ~AllExited
            THEN /\ (floorDue /\ depth[1] # 0 /\ \E j \in 1..Len(waitq) : timed[waitq[j]])
                    \/ AllExited
                 /\ IF ~AllExited
                       THEN /\ \E j \in {k \in 1..Len(waitq) : timed[waitq[k]]}:
                                 waitq' = Drop(waitq, j)
                       ELSE /\ TRUE
                            /\ waitq' = waitq
                 /\ pc' = [pc EXCEPT ![FloorWake] = "FW"]
            ELSE /\ pc' = [pc EXCEPT ![FloorWake] = "Done"]
                 /\ waitq' = waitq
      /\ UNCHANGED << sets, depth, lowerRunning, reserve, floorDue, fresh, 
                      interrupt, done, held, timed, fRuns, rolls, turnClaims, 
                      overCap, spurs, stack, pt, pext, pquiet, pwake, task, 
                      sawEmpty, readRes, yield, drop >>

floorWake == FW

RPick == /\ pc[Roll] = "RPick"
         /\ IF rolls < Rolls
               THEN /\ rolls' = rolls + 1
                    /\ \E r \in {0, RHi}:
                         /\ drop' = (r < reserve)
                         /\ reserve' = r
                    /\ pc' = [pc EXCEPT ![Roll] = "RCheck"]
               ELSE /\ pc' = [pc EXCEPT ![Roll] = "Done"]
                    /\ UNCHANGED << reserve, rolls, drop >>
         /\ UNCHANGED << sets, depth, lowerRunning, floorDue, fresh, waitq, 
                         interrupt, done, held, timed, fRuns, turnClaims, 
                         overCap, spurs, stack, pt, pext, pquiet, pwake, task, 
                         sawEmpty, readRes, yield >>

RCheck == /\ pc[Roll] = "RCheck"
          /\ IF drop /\ depth[1] # 0
                THEN /\ pc' = [pc EXCEPT ![Roll] = "RBump"]
                ELSE /\ pc' = [pc EXCEPT ![Roll] = "RReset"]
          /\ UNCHANGED << sets, depth, lowerRunning, reserve, floorDue, fresh, 
                          waitq, interrupt, done, held, timed, fRuns, rolls, 
                          turnClaims, overCap, spurs, stack, pt, pext, pquiet, 
                          pwake, task, sawEmpty, readRes, yield, drop >>

RBump == /\ pc[Roll] = "RBump"
         /\ fresh' = [w \in Workers |-> FALSE]
         /\ pc' = [pc EXCEPT ![Roll] = "RWake"]
         /\ UNCHANGED << sets, depth, lowerRunning, reserve, floorDue, waitq, 
                         interrupt, done, held, timed, fRuns, rolls, 
                         turnClaims, overCap, spurs, stack, pt, pext, pquiet, 
                         pwake, task, sawEmpty, readRes, yield, drop >>

RWake == /\ pc[Roll] = "RWake"
         /\ IF waitq # << >>
               THEN /\ waitq' = Tail(waitq)
               ELSE /\ TRUE
                    /\ waitq' = waitq
         /\ pc' = [pc EXCEPT ![Roll] = "RReset"]
         /\ UNCHANGED << sets, depth, lowerRunning, reserve, floorDue, fresh, 
                         interrupt, done, held, timed, fRuns, rolls, 
                         turnClaims, overCap, spurs, stack, pt, pext, pquiet, 
                         pwake, task, sawEmpty, readRes, yield, drop >>

RReset == /\ pc[Roll] = "RReset"
          /\ drop' = FALSE
          /\ pc' = [pc EXCEPT ![Roll] = "RPick"]
          /\ UNCHANGED << sets, depth, lowerRunning, reserve, floorDue, fresh, 
                          waitq, interrupt, done, held, timed, fRuns, rolls, 
                          turnClaims, overCap, spurs, stack, pt, pext, pquiet, 
                          pwake, task, sawEmpty, readRes, yield >>

roll == RPick \/ RCheck \/ RBump \/ RWake \/ RReset

SpLoop == /\ pc[Spurious] = "SpLoop"
          /\ \/ /\ waitq # << >> /\ spurs < Spurs
                /\ spurs' = spurs + 1
                /\ \E j \in 1..Len(waitq):
                     waitq' = Drop(waitq, j)
                /\ pc' = [pc EXCEPT ![Spurious] = "SpLoop"]
             \/ /\ AllExited
                /\ pc' = [pc EXCEPT ![Spurious] = "Done"]
                /\ UNCHANGED <<waitq, spurs>>
          /\ UNCHANGED << sets, depth, lowerRunning, reserve, floorDue, fresh, 
                          interrupt, done, held, timed, fRuns, rolls, 
                          turnClaims, overCap, stack, pt, pext, pquiet, pwake, 
                          task, sawEmpty, readRes, yield, drop >>

spurious == SpLoop

(* Allow infinite stuttering to prevent deadlock on termination. *)
Terminating == /\ \A self \in ProcSet: pc[self] = "Done"
               /\ UNCHANGED vars

Next == pusher \/ stopper \/ clock \/ floorWake \/ roll \/ spurious
           \/ (\E self \in ProcSet: push(self))
           \/ (\E self \in Workers: worker(self))
           \/ Terminating

Spec == /\ Init /\ [][Next]_vars
        /\ \A self \in Workers : WF_vars(worker(self)) /\ WF_vars(push(self))
        /\ WF_vars(pusher) /\ WF_vars(push(Pusher))
        /\ WF_vars(stopper)
        /\ WF_vars(clock)
        /\ WF_vars(floorWake)
        /\ WF_vars((pc[Roll] # "RPick") /\ roll)

Termination == <>(\A self \in ProcSet: pc[self] = "Done")

\* END TRANSLATION

TypeOK ==
    /\ \A s \in 0..1 : depth[s] \in 0..3
    /\ lowerRunning \in 0..N
    /\ reserve \in {0, RHi}
    /\ \A w \in Workers : task[w] \in {None} \cup Tasks
    /\ \A w \in Workers : held[w] \in {"no", "floor", "claim"}

DepthExact ==
    \A s \in 0..1 :
        depth[s] = Len(sets[s])
            + Cardinality({p \in Workers \cup {Pusher} :
                              pc[p] = "PInsert" /\ LaneOf(pt[p]) = s})
            + Cardinality({w \in Workers :
                              IF s = 0 THEN pc[w] = "WDec0"
                              ELSE pc[w] \in {"WFRun", "WFDec", "WLDec"}})

RunningExact ==
    lowerRunning =
        Cardinality({w \in Workers : held[w] = "claim"})
        + Cardinality({w \in Workers : held[w] = "floor" /\ pc[w] \notin {"WFPop", "WFRun"}})

LowerOpen == lowerRunning < N - reserve

NoLostWakeup ==
    ((\A w \in Workers : InQ(w, waitq))
        /\ (sets[0] # << >> \/ (sets[1] # << >> /\ LowerOpen)))
    => (pc[Pusher] \in {"PBump", "PWake"} \/ pc[Roll] \in {"RCheck", "RBump", "RWake"})

FrameFirst ==
    \A w \in Workers :
        (pc[w] = "WLPop" /\ sets[1] # << >>) => sawEmpty[w]

OneFloorTurn == turnClaims <= 1

ReserveHeld == ~overCap

EveryTaskRuns == Drain => <>[]AllDone

StopTerminates == ~Forever => <>AllExited

Running(w) ==
    /\ task[w] # None
    /\ pc[w] \in {"WCasc", "WWake", "WRun", "WBurst1", "WBurst2", "WHold", "WKid",
                  "WLeave", "WYield"}
TwoRunning == Cardinality({w \in Workers : Running(w)}) >= 2
CascadeWakes ==
    \A w \in Workers :
        (Takeable /\ waitq # << >> /\ Head(waitq) = w)
            ~> (~AllDone /\ (TwoRunning \/ ~InQ(w, waitq) \/ ~Takeable))

LowerProgress == Forever => <>done[L]

=============================================================================
