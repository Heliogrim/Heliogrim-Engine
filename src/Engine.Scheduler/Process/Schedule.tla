------------------------------ MODULE Schedule ------------------------------
(* Model of the worker park/wake protocol:
   src/Engine.Scheduler/Process/Schedule.cpp (push, pop, transfer, epoch,
   queued, wakeOne, wakeAll, waitForWork), the loop in
   src/Engine.Scheduler/Worker/Worker.cpp (Worker::handle) and the stop order
   in src/Engine.Scheduler/CompScheduler.cpp (destroy).
   Two work sets (0 and 1); each set's spin lock is folded into an atomic
   insert or pop on that set, except set 1's lock, which transfer() holds
   across its whole move loop (FifoWorkSet::transfer), so it is lock1.
   push: _queued up, insert, epoch bump, wakeOne only for a non-worker
   pusher (the count comes first so a racing pop cannot wrap it).
   pop walks set 0 then set 1 (a full-mask worker), then _queued down.
   WaitOnAddress is an atomic compare-and-enqueue on waitq;
   WakeByAddressSingle dequeues the head, WakeByAddressAll empties waitq;
   spurious returns are the unfair process Spurious.
   Tasks: task 1 is pushed by the external pusher (a thread that is not a
   worker). Task 1's body pushes child 2 (a worker push) and yields once; the
   worker loop re-pushes it (a worker push). This is the worker fan-out the
   cascade exists for. Bounds: N = 2 workers, 2 tasks. N = 3 with 3 tasks
   (a second external push) and N = 3 with 2 tasks both still had a growing
   queue past 2.2M distinct states and were stopped.
   The ager: Schedule::tick() is called at the end of
   CompositePipeline::complete, i.e. by a worker at the end of a task body,
   so the model runs tick (transfer set 1 into set 0, bump when moved) at the
   end of every task completion on the completing worker. A separate ager
   thread is not the code: with the bump after the set lock is released
   (Schedule::transfer), a worker can lose the race, compare the old epoch
   and park before the bump, and a non-worker ager would then lose the task.
   Each task's mask (its set) is fixed per task and chosen nondeterministically
   in the initial state. FifoWorkSet::push never fails, so the push-failed
   undo path is not modelled.
   Stop: with drain, destroy() runs after every task completed; without it,
   destroy() races the workers, the pusher and the cascade at any point.
   EveryTaskRuns is required only under drain (a stop drops queued work by
   design); StopTerminates is required in both.
   CascadeWakes' TwoRunning disjunct is weak for N >= 3 (it should require
   that w itself runs). The deadlock check cannot catch a lost wake, because
   Spurious can always step; the liveness properties catch it.
   Dead locals are reset (seen after its use, task at loop end).
   Mutations (N = 2, 2 tasks):
   (a) load the epoch after the pop: fails NoLostWakeup (18 states) and,
       with it off, EveryTaskRuns (19 states);
   (b) drop WakeByAddressAll in wakeAll: fails StopTerminates (55 states,
       also with the racing stop);
   (c) drop the cascade wake: fails CascadeWakes only (61 states);
   (d) drop the epoch bump in transfer: passes. tick runs on a worker that
       goes straight back to pop, so it takes the moved task itself, and a
       parked worker with the old epoch is then not needed for progress. *)
EXTENDS Naturals, Sequences, FiniteSets, TLC

CONSTANT N
Workers == 1..N
Pusher == N + 1
Stopper == N + 2
Spurious == N + 3
Tasks == 1..2
Forker == 1
Child == 2
None == 0

InQ(w, q) == \E i \in 1..Len(q) : q[i] = w
Drop(q, i) == SubSeq(q, 1, i - 1) \o SubSeq(q, i + 1, Len(q))

(* --algorithm Schedule
variables
    prio \in [Tasks -> {0, 1}],     \* maskToPriority(task->mask())
    sets = [s \in {0, 1} |-> << >>],
    lock1 = FALSE,                  \* set 1's _lck, held across transfer
    queued = 0,                     \* _queued
    epoch = 0,                      \* _pushEpoch
    waitq = << >>,                  \* threads blocked in WaitOnAddress
    interrupt = [w \in Workers |-> FALSE],
    done = [t \in Tasks |-> FALSE],
    yielded = FALSE,                \* task 1 ran its first half
    drain \in BOOLEAN;              \* stop waits for all work, or races it

define
    AllDone == \A t \in Tasks : done[t]
    AllExited == \A w \in Workers : pc[w] = "Done"
end define;

procedure push(pt, pext)
begin
PInc:
    queued := queued + 1;
PInsert:
    await prio[pt] = 0 \/ ~lock1;
    sets[prio[pt]] := Append(sets[prio[pt]], pt);
PBump:
    epoch := epoch + 1;
    if ~pext then
        return;
    end if;
PWake:
    if waitq # << >> then
        waitq := Tail(waitq);
    end if;
    return;
end procedure;

fair process worker \in Workers
variables seen = 0, task = None, moved = 0;
begin
WCheck:
    while ~interrupt[self] do
WEpoch:
        seen := epoch;
WPop0:
        if sets[0] # << >> then
            task := Head(sets[0]);
            sets[0] := Tail(sets[0]);
            goto WDec;
        end if;
WPop1:
        await ~lock1;
        if sets[1] # << >> then
            task := Head(sets[1]);
            sets[1] := Tail(sets[1]);
            goto WDec;
        end if;
WFail:
        if interrupt[self] then
            goto WExit;
        end if;
WPark:
        if epoch = seen then
            waitq := Append(waitq, self);
            seen := 0;
        else
            seen := 0;
            goto WCheck;
        end if;
WSleep:
        await ~InQ(self, waitq);
        goto WCheck;
WDec:
        queued := queued - 1;
        seen := 0;
WQueued:
        if queued > 0 then
WWake:
            if waitq # << >> then
                waitq := Tail(waitq);
            end if;
        end if;
WRun:
        if task = Forker /\ ~yielded then
            yielded := TRUE;
            call push(Child, FALSE);
            goto WYield;
        else
            done[task] := TRUE;
        end if;
WTLock:
        await ~lock1;
        lock1 := TRUE;
WTMove:
        while sets[1] # << >> do
            sets := [sets EXCEPT ![0] = Append(@, Head(sets[1])), ![1] = Tail(@)];
            moved := moved + 1;
        end while;
WTRelease:
        lock1 := FALSE;
WTBump:
        if moved > 0 then
            epoch := epoch + 1;
        end if;
        moved := 0;
        task := None;
        goto WCheck;
WYield:
        call push(Forker, FALSE);
        goto WEnd;
WEnd:
        task := None;
    end while;
WExit:
    skip;
end process;

fair process pusher = Pusher
begin
P1:
    call push(Forker, TRUE);
end process;

fair process stopper = Stopper
variables i = 1;
begin
SAwait:
    await ~drain \/ AllDone;
SReq:
    while i <= N do
        interrupt[i] := TRUE;
        i := i + 1;
    end while;
SBump:
    epoch := epoch + 1;
SWakeAll:
    waitq := << >>;
SJoin:
    await AllExited;
end process;

process spurious = Spurious
begin
SpLoop:
    either
        await waitq # << >>;
        with k \in 1..Len(waitq) do
            waitq := Drop(waitq, k);
        end with;
        goto SpLoop;
    or
        await AllExited;
    end either;
end process;

end algorithm; *)
\* BEGIN TRANSLATION
CONSTANT defaultInitValue
VARIABLES prio, sets, lock1, queued, epoch, waitq, interrupt, done, yielded, 
          drain, pc, stack

(* define statement *)
AllDone == \A t \in Tasks : done[t]
AllExited == \A w \in Workers : pc[w] = "Done"

VARIABLES pt, pext, seen, task, moved, i

vars == << prio, sets, lock1, queued, epoch, waitq, interrupt, done, yielded, 
           drain, pc, stack, pt, pext, seen, task, moved, i >>

ProcSet == (Workers) \cup {Pusher} \cup {Stopper} \cup {Spurious}

Init == (* Global variables *)
        /\ prio \in [Tasks -> {0, 1}]
        /\ sets = [s \in {0, 1} |-> << >>]
        /\ lock1 = FALSE
        /\ queued = 0
        /\ epoch = 0
        /\ waitq = << >>
        /\ interrupt = [w \in Workers |-> FALSE]
        /\ done = [t \in Tasks |-> FALSE]
        /\ yielded = FALSE
        /\ drain \in BOOLEAN
        (* Procedure push *)
        /\ pt = [ self \in ProcSet |-> defaultInitValue]
        /\ pext = [ self \in ProcSet |-> defaultInitValue]
        (* Process worker *)
        /\ seen = [self \in Workers |-> 0]
        /\ task = [self \in Workers |-> None]
        /\ moved = [self \in Workers |-> 0]
        (* Process stopper *)
        /\ i = 1
        /\ stack = [self \in ProcSet |-> << >>]
        /\ pc = [self \in ProcSet |-> CASE self \in Workers -> "WCheck"
                                        [] self = Pusher -> "P1"
                                        [] self = Stopper -> "SAwait"
                                        [] self = Spurious -> "SpLoop"]

PInc(self) == /\ pc[self] = "PInc"
              /\ queued' = queued + 1
              /\ pc' = [pc EXCEPT ![self] = "PInsert"]
              /\ UNCHANGED << prio, sets, lock1, epoch, waitq, interrupt, done, 
                              yielded, drain, stack, pt, pext, seen, task, 
                              moved, i >>

PInsert(self) == /\ pc[self] = "PInsert"
                 /\ prio[pt[self]] = 0 \/ ~lock1
                 /\ sets' = [sets EXCEPT ![prio[pt[self]]] = Append(sets[prio[pt[self]]], pt[self])]
                 /\ pc' = [pc EXCEPT ![self] = "PBump"]
                 /\ UNCHANGED << prio, lock1, queued, epoch, waitq, interrupt, 
                                 done, yielded, drain, stack, pt, pext, seen, 
                                 task, moved, i >>

PBump(self) == /\ pc[self] = "PBump"
               /\ epoch' = epoch + 1
               /\ IF ~pext[self]
                     THEN /\ pc' = [pc EXCEPT ![self] = Head(stack[self]).pc]
                          /\ pt' = [pt EXCEPT ![self] = Head(stack[self]).pt]
                          /\ pext' = [pext EXCEPT ![self] = Head(stack[self]).pext]
                          /\ stack' = [stack EXCEPT ![self] = Tail(stack[self])]
                     ELSE /\ pc' = [pc EXCEPT ![self] = "PWake"]
                          /\ UNCHANGED << stack, pt, pext >>
               /\ UNCHANGED << prio, sets, lock1, queued, waitq, interrupt, 
                               done, yielded, drain, seen, task, moved, i >>

PWake(self) == /\ pc[self] = "PWake"
               /\ IF waitq # << >>
                     THEN /\ waitq' = Tail(waitq)
                     ELSE /\ TRUE
                          /\ waitq' = waitq
               /\ pc' = [pc EXCEPT ![self] = Head(stack[self]).pc]
               /\ pt' = [pt EXCEPT ![self] = Head(stack[self]).pt]
               /\ pext' = [pext EXCEPT ![self] = Head(stack[self]).pext]
               /\ stack' = [stack EXCEPT ![self] = Tail(stack[self])]
               /\ UNCHANGED << prio, sets, lock1, queued, epoch, interrupt, 
                               done, yielded, drain, seen, task, moved, i >>

push(self) == PInc(self) \/ PInsert(self) \/ PBump(self) \/ PWake(self)

WCheck(self) == /\ pc[self] = "WCheck"
                /\ IF ~interrupt[self]
                      THEN /\ pc' = [pc EXCEPT ![self] = "WEpoch"]
                      ELSE /\ pc' = [pc EXCEPT ![self] = "WExit"]
                /\ UNCHANGED << prio, sets, lock1, queued, epoch, waitq, 
                                interrupt, done, yielded, drain, stack, pt, 
                                pext, seen, task, moved, i >>

WEpoch(self) == /\ pc[self] = "WEpoch"
                /\ seen' = [seen EXCEPT ![self] = epoch]
                /\ pc' = [pc EXCEPT ![self] = "WPop0"]
                /\ UNCHANGED << prio, sets, lock1, queued, epoch, waitq, 
                                interrupt, done, yielded, drain, stack, pt, 
                                pext, task, moved, i >>

WPop0(self) == /\ pc[self] = "WPop0"
               /\ IF sets[0] # << >>
                     THEN /\ task' = [task EXCEPT ![self] = Head(sets[0])]
                          /\ sets' = [sets EXCEPT ![0] = Tail(sets[0])]
                          /\ pc' = [pc EXCEPT ![self] = "WDec"]
                     ELSE /\ pc' = [pc EXCEPT ![self] = "WPop1"]
                          /\ UNCHANGED << sets, task >>
               /\ UNCHANGED << prio, lock1, queued, epoch, waitq, interrupt, 
                               done, yielded, drain, stack, pt, pext, seen, 
                               moved, i >>

WPop1(self) == /\ pc[self] = "WPop1"
               /\ ~lock1
               /\ IF sets[1] # << >>
                     THEN /\ task' = [task EXCEPT ![self] = Head(sets[1])]
                          /\ sets' = [sets EXCEPT ![1] = Tail(sets[1])]
                          /\ pc' = [pc EXCEPT ![self] = "WDec"]
                     ELSE /\ pc' = [pc EXCEPT ![self] = "WFail"]
                          /\ UNCHANGED << sets, task >>
               /\ UNCHANGED << prio, lock1, queued, epoch, waitq, interrupt, 
                               done, yielded, drain, stack, pt, pext, seen, 
                               moved, i >>

WFail(self) == /\ pc[self] = "WFail"
               /\ IF interrupt[self]
                     THEN /\ pc' = [pc EXCEPT ![self] = "WExit"]
                     ELSE /\ pc' = [pc EXCEPT ![self] = "WPark"]
               /\ UNCHANGED << prio, sets, lock1, queued, epoch, waitq, 
                               interrupt, done, yielded, drain, stack, pt, 
                               pext, seen, task, moved, i >>

WPark(self) == /\ pc[self] = "WPark"
               /\ IF epoch = seen[self]
                     THEN /\ waitq' = Append(waitq, self)
                          /\ seen' = [seen EXCEPT ![self] = 0]
                          /\ pc' = [pc EXCEPT ![self] = "WSleep"]
                     ELSE /\ seen' = [seen EXCEPT ![self] = 0]
                          /\ pc' = [pc EXCEPT ![self] = "WCheck"]
                          /\ waitq' = waitq
               /\ UNCHANGED << prio, sets, lock1, queued, epoch, interrupt, 
                               done, yielded, drain, stack, pt, pext, task, 
                               moved, i >>

WSleep(self) == /\ pc[self] = "WSleep"
                /\ ~InQ(self, waitq)
                /\ pc' = [pc EXCEPT ![self] = "WCheck"]
                /\ UNCHANGED << prio, sets, lock1, queued, epoch, waitq, 
                                interrupt, done, yielded, drain, stack, pt, 
                                pext, seen, task, moved, i >>

WDec(self) == /\ pc[self] = "WDec"
              /\ queued' = queued - 1
              /\ seen' = [seen EXCEPT ![self] = 0]
              /\ pc' = [pc EXCEPT ![self] = "WQueued"]
              /\ UNCHANGED << prio, sets, lock1, epoch, waitq, interrupt, done, 
                              yielded, drain, stack, pt, pext, task, moved, i >>

WQueued(self) == /\ pc[self] = "WQueued"
                 /\ IF queued > 0
                       THEN /\ pc' = [pc EXCEPT ![self] = "WWake"]
                       ELSE /\ pc' = [pc EXCEPT ![self] = "WRun"]
                 /\ UNCHANGED << prio, sets, lock1, queued, epoch, waitq, 
                                 interrupt, done, yielded, drain, stack, pt, 
                                 pext, seen, task, moved, i >>

WWake(self) == /\ pc[self] = "WWake"
               /\ IF waitq # << >>
                     THEN /\ waitq' = Tail(waitq)
                     ELSE /\ TRUE
                          /\ waitq' = waitq
               /\ pc' = [pc EXCEPT ![self] = "WRun"]
               /\ UNCHANGED << prio, sets, lock1, queued, epoch, interrupt, 
                               done, yielded, drain, stack, pt, pext, seen, 
                               task, moved, i >>

WRun(self) == /\ pc[self] = "WRun"
              /\ IF task[self] = Forker /\ ~yielded
                    THEN /\ yielded' = TRUE
                         /\ /\ pext' = [pext EXCEPT ![self] = FALSE]
                            /\ pt' = [pt EXCEPT ![self] = Child]
                            /\ stack' = [stack EXCEPT ![self] = << [ procedure |->  "push",
                                                                     pc        |->  "WYield",
                                                                     pt        |->  pt[self],
                                                                     pext      |->  pext[self] ] >>
                                                                 \o stack[self]]
                         /\ pc' = [pc EXCEPT ![self] = "PInc"]
                         /\ done' = done
                    ELSE /\ done' = [done EXCEPT ![task[self]] = TRUE]
                         /\ pc' = [pc EXCEPT ![self] = "WTLock"]
                         /\ UNCHANGED << yielded, stack, pt, pext >>
              /\ UNCHANGED << prio, sets, lock1, queued, epoch, waitq, 
                              interrupt, drain, seen, task, moved, i >>

WTLock(self) == /\ pc[self] = "WTLock"
                /\ ~lock1
                /\ lock1' = TRUE
                /\ pc' = [pc EXCEPT ![self] = "WTMove"]
                /\ UNCHANGED << prio, sets, queued, epoch, waitq, interrupt, 
                                done, yielded, drain, stack, pt, pext, seen, 
                                task, moved, i >>

WTMove(self) == /\ pc[self] = "WTMove"
                /\ IF sets[1] # << >>
                      THEN /\ sets' = [sets EXCEPT ![0] = Append(@, Head(sets[1])), ![1] = Tail(@)]
                           /\ moved' = [moved EXCEPT ![self] = moved[self] + 1]
                           /\ pc' = [pc EXCEPT ![self] = "WTMove"]
                      ELSE /\ pc' = [pc EXCEPT ![self] = "WTRelease"]
                           /\ UNCHANGED << sets, moved >>
                /\ UNCHANGED << prio, lock1, queued, epoch, waitq, interrupt, 
                                done, yielded, drain, stack, pt, pext, seen, 
                                task, i >>

WTRelease(self) == /\ pc[self] = "WTRelease"
                   /\ lock1' = FALSE
                   /\ pc' = [pc EXCEPT ![self] = "WTBump"]
                   /\ UNCHANGED << prio, sets, queued, epoch, waitq, interrupt, 
                                   done, yielded, drain, stack, pt, pext, seen, 
                                   task, moved, i >>

WTBump(self) == /\ pc[self] = "WTBump"
                /\ IF moved[self] > 0
                      THEN /\ epoch' = epoch + 1
                      ELSE /\ TRUE
                           /\ epoch' = epoch
                /\ moved' = [moved EXCEPT ![self] = 0]
                /\ task' = [task EXCEPT ![self] = None]
                /\ pc' = [pc EXCEPT ![self] = "WCheck"]
                /\ UNCHANGED << prio, sets, lock1, queued, waitq, interrupt, 
                                done, yielded, drain, stack, pt, pext, seen, i >>

WYield(self) == /\ pc[self] = "WYield"
                /\ /\ pext' = [pext EXCEPT ![self] = FALSE]
                   /\ pt' = [pt EXCEPT ![self] = Forker]
                   /\ stack' = [stack EXCEPT ![self] = << [ procedure |->  "push",
                                                            pc        |->  "WEnd",
                                                            pt        |->  pt[self],
                                                            pext      |->  pext[self] ] >>
                                                        \o stack[self]]
                /\ pc' = [pc EXCEPT ![self] = "PInc"]
                /\ UNCHANGED << prio, sets, lock1, queued, epoch, waitq, 
                                interrupt, done, yielded, drain, seen, task, 
                                moved, i >>

WEnd(self) == /\ pc[self] = "WEnd"
              /\ task' = [task EXCEPT ![self] = None]
              /\ pc' = [pc EXCEPT ![self] = "WCheck"]
              /\ UNCHANGED << prio, sets, lock1, queued, epoch, waitq, 
                              interrupt, done, yielded, drain, stack, pt, pext, 
                              seen, moved, i >>

WExit(self) == /\ pc[self] = "WExit"
               /\ TRUE
               /\ pc' = [pc EXCEPT ![self] = "Done"]
               /\ UNCHANGED << prio, sets, lock1, queued, epoch, waitq, 
                               interrupt, done, yielded, drain, stack, pt, 
                               pext, seen, task, moved, i >>

worker(self) == WCheck(self) \/ WEpoch(self) \/ WPop0(self) \/ WPop1(self)
                   \/ WFail(self) \/ WPark(self) \/ WSleep(self)
                   \/ WDec(self) \/ WQueued(self) \/ WWake(self)
                   \/ WRun(self) \/ WTLock(self) \/ WTMove(self)
                   \/ WTRelease(self) \/ WTBump(self) \/ WYield(self)
                   \/ WEnd(self) \/ WExit(self)

P1 == /\ pc[Pusher] = "P1"
      /\ /\ pext' = [pext EXCEPT ![Pusher] = TRUE]
         /\ pt' = [pt EXCEPT ![Pusher] = Forker]
         /\ stack' = [stack EXCEPT ![Pusher] = << [ procedure |->  "push",
                                                    pc        |->  "Done",
                                                    pt        |->  pt[Pusher],
                                                    pext      |->  pext[Pusher] ] >>
                                                \o stack[Pusher]]
      /\ pc' = [pc EXCEPT ![Pusher] = "PInc"]
      /\ UNCHANGED << prio, sets, lock1, queued, epoch, waitq, interrupt, done, 
                      yielded, drain, seen, task, moved, i >>

pusher == P1

SAwait == /\ pc[Stopper] = "SAwait"
          /\ ~drain \/ AllDone
          /\ pc' = [pc EXCEPT ![Stopper] = "SReq"]
          /\ UNCHANGED << prio, sets, lock1, queued, epoch, waitq, interrupt, 
                          done, yielded, drain, stack, pt, pext, seen, task, 
                          moved, i >>

SReq == /\ pc[Stopper] = "SReq"
        /\ IF i <= N
              THEN /\ interrupt' = [interrupt EXCEPT ![i] = TRUE]
                   /\ i' = i + 1
                   /\ pc' = [pc EXCEPT ![Stopper] = "SReq"]
              ELSE /\ pc' = [pc EXCEPT ![Stopper] = "SBump"]
                   /\ UNCHANGED << interrupt, i >>
        /\ UNCHANGED << prio, sets, lock1, queued, epoch, waitq, done, yielded, 
                        drain, stack, pt, pext, seen, task, moved >>

SBump == /\ pc[Stopper] = "SBump"
         /\ epoch' = epoch + 1
         /\ pc' = [pc EXCEPT ![Stopper] = "SWakeAll"]
         /\ UNCHANGED << prio, sets, lock1, queued, waitq, interrupt, done, 
                         yielded, drain, stack, pt, pext, seen, task, moved, i >>

SWakeAll == /\ pc[Stopper] = "SWakeAll"
            /\ waitq' = << >>
            /\ pc' = [pc EXCEPT ![Stopper] = "SJoin"]
            /\ UNCHANGED << prio, sets, lock1, queued, epoch, interrupt, done, 
                            yielded, drain, stack, pt, pext, seen, task, moved, 
                            i >>

SJoin == /\ pc[Stopper] = "SJoin"
         /\ AllExited
         /\ pc' = [pc EXCEPT ![Stopper] = "Done"]
         /\ UNCHANGED << prio, sets, lock1, queued, epoch, waitq, interrupt, 
                         done, yielded, drain, stack, pt, pext, seen, task, 
                         moved, i >>

stopper == SAwait \/ SReq \/ SBump \/ SWakeAll \/ SJoin

SpLoop == /\ pc[Spurious] = "SpLoop"
          /\ \/ /\ waitq # << >>
                /\ \E k \in 1..Len(waitq):
                     waitq' = Drop(waitq, k)
                /\ pc' = [pc EXCEPT ![Spurious] = "SpLoop"]
             \/ /\ AllExited
                /\ pc' = [pc EXCEPT ![Spurious] = "Done"]
                /\ waitq' = waitq
          /\ UNCHANGED << prio, sets, lock1, queued, epoch, interrupt, done, 
                          yielded, drain, stack, pt, pext, seen, task, moved, 
                          i >>

spurious == SpLoop

(* Allow infinite stuttering to prevent deadlock on termination. *)
Terminating == /\ \A self \in ProcSet: pc[self] = "Done"
               /\ UNCHANGED vars

Next == pusher \/ stopper \/ spurious
           \/ (\E self \in ProcSet: push(self))
           \/ (\E self \in Workers: worker(self))
           \/ Terminating

Spec == /\ Init /\ [][Next]_vars
        /\ \A self \in Workers : WF_vars(worker(self)) /\ WF_vars(push(self))
        /\ WF_vars(pusher) /\ WF_vars(push(Pusher))
        /\ WF_vars(stopper)

Termination == <>(\A self \in ProcSet: pc[self] = "Done")

\* END TRANSLATION

-----------------------------------------------------------------------------
TypeOK ==
    /\ queued \in Nat
    /\ epoch \in Nat
    /\ \A w \in Workers : task[w] \in {None} \cup Tasks

\* _queued counts the tasks in the sets, those counted but not yet inserted,
\* and those popped but not yet counted down; it never wraps below zero.
QueuedExact ==
    queued = Len(sets[0]) + Len(sets[1])
             + Cardinality({p \in Workers \cup {Pusher} : pc[p] = "PInsert"})
             + Cardinality({w \in Workers : pc[w] = "WDec"})

\* Wakeup safety: when every worker is blocked in WaitOnAddress and a task sits
\* in a set, the external pusher is between its insert and its wake.
NoLostWakeup ==
    ((\A w \in Workers : InQ(w, waitq)) /\ (sets[0] # << >> \/ sets[1] # << >>))
        => pc[Pusher] \in {"PBump", "PWake"}

EveryTaskRuns == drain => <>[]AllDone
StopTerminates == <>AllExited

Running(w) == task[w] # None
TwoRunning == Cardinality({w \in Workers : Running(w)}) >= 2

\* The cascade: while two tasks are queued and w heads the wait queue, either
\* two workers run at once or w is woken while work remains.
CascadeWakes ==
    \A w \in Workers :
        (queued >= 2 /\ waitq # << >> /\ Head(waitq) = w)
            ~> (TwoRunning \/ (~InQ(w, waitq) /\ ~AllDone))
=============================================================================
