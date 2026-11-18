------------------------------ MODULE BoostCell ------------------------------
(* Model of a wait boost racing the producer it boosts:
   src/Engine.Scheduler/Process/BoostCell.cpp (BoostCell::tryRaise, boost),
   src/Engine.Scheduler/Process/Schedule.cpp (Schedule::promote,
   Schedule::pushBoostable), src/Engine.Scheduler/Process/WorkSet/
   FifoWorkSet.cpp (push, pop, erase under the work set lock) and the two
   callers of pushBoostable: Worker::handle's re-push of a yielded task
   (src/Engine.Scheduler/Worker/Worker.cpp) and unpark
   (src/Engine.Scheduler/Fiber/Park.cpp).

   Modelled, one label per atomic operation of the code:
   - Waiter (boost): the CAS raise of cell.lane, the seq_cst load of
     cell.task, the erase from the lower work set (promote's search), the
     push into the raised lane.
   - Producer, run by either of two workers: pop, run, then finish or
     yield. The yield is pushBoostable: read cell.lane and write the task
     lane, push, re-read cell.lane, erase from the lower set when it rose,
     push into the raised lane.
   - Recycler: after the producer finished, its task address is reused by
     an unrelated task queued in the producer's lane under another (or no)
     cell. Every erase matches (task, cell): the entry's boostCell, written
     by pushBoostable, must be the boosting cell (FifoWorkSet::erase).

   Folded steps:
   - promote's lane write and its push are one step (the erase made the
     promoter the only holder of the entry, nobody else reads the lane).
   - push's depth counting, epoch bump and wake are left out: they do not
     decide where the entry is queued. Schedule.tla checks them.
   - The worker's re-push and unpark are the same pushBoostable; after the
     park handshake exactly one side calls it, so one Yield path stands for
     both.
   - The cell lane starts at the lowest lane (Background) and records waiter urgency only.
   - Finishing clears cell.task and ends the producer in one step (the
     clear is done by the future, Task 8).
   - The running producer's checkpoint yield is the free choice between
     finish and yield in WRun.

   Abstractions:
   - Two lanes: 0 is the waiter's lane (Frame), 1 the producer's
     (Background). With two lanes the re-push inside promote cannot rise
     again, so its pushBoostable is a plain push.
   - The work sets hold one task, so where in {None, 0, 1} is the whole
     queue state; erase is "where = 1 /\ entryCell = Mine -> None". The
     reused address is the same slot: entryCell tells the producer (Mine)
     from the unrelated task (Foreign). A worker popping the foreign task
     runs it to completion (it is not the producer).
   - One waiter: a second boost of the same cell to the same lane returns
     at its raise (tryRaise fails); to a higher lane it is the same race
     again one lane up.
   - Yields bounds how often the producer yields, so the state space is
     finite; Yields = 2 lets a yield follow a boosted re-push.

   Property BoostHolds: once boost has returned and the producer has not
   finished, at every quiescent point (no worker inside pushBoostable, no
   promote in flight) the producer is not queued below the waiter's lane.

   Property NoForeignPromote: no erase of promote (boost's or
   pushBoostable's re-read) ever removes the unrelated task that reuses the
   producer's address.

   The cell lifetime is not modelled; the code keeps the cell alive across the push by a counted reference.
   That counted reference is also why the cell match is sound: no other live
   task can be bound to a cell address the booster still holds.

   Mutations (each must fail the named property):
   (a) pushBoostable without the re-read after the push (WReread raises
       nothing): FAILS BoostHolds.
   (b) boost raising the cell after the search (WaRaise after WaPush):
       FAILS BoostHolds.
   (c) erase matching the task address only (drop "/\ entryCell = Mine"
       in WaSearch and WErase): FAILS NoForeignPromote.

   TLC, Yields = 2, -workers 6: no error, 24848 states generated, 9788 distinct, depth 30, 1s.
   Mutation (c): Invariant NoForeignPromote is violated, 264 states generated, 159 distinct, 0s.
*)
EXTENDS Naturals, TLC

CONSTANTS Yields

Workers == {1, 2}
Waiter == 0
None == 2
Recycler == 3
Mine == "mine"
Foreign == "foreign"

(* --fair algorithm BoostCell {
variables
    cellLane = 1,
    cellTask = TRUE,
    taskLane = 1,
    where = 1,
    finished = FALSE,
    yields = 0,
    entryCell = Mine,
    foreignMoved = FALSE;

define {
    TypeOK == /\ cellLane \in {0, 1}
              /\ taskLane \in {0, 1}
              /\ where \in {0, 1, None}
              /\ finished \in BOOLEAN
              /\ yields \in 0..Yields
              /\ entryCell \in {Mine, Foreign}
              /\ foreignMoved \in BOOLEAN

    \* The producer counts as the waiter's lane from the raise on: never lowered.
    LaneMonotone == [][cellLane' <= cellLane]_cellLane

    Quiescent == \A w \in Workers : pc[w] \in {"WLoop", "WPop", "WRun", "Done"}

    BoostHolds == (pc[Waiter] = "Done" /\ ~finished /\ Quiescent) => where /= 1

    Terminates == <>(\A p \in Workers \cup {Waiter} : pc[p] = "Done")

    BoostSticks == (pc[Waiter] = "Done" /\ where = 1) ~> (where /= 1 \/ finished)

    NoForeignPromote == ~foreignMoved
}

fair process (Worker \in Workers)
variables mine = 1, raised = 1, popped = FALSE, moved = FALSE;
{
WLoop:
    while (~finished) {
WPop:
        await where /= None \/ finished;
        if (where /= None) {
            where := None;
            popped := entryCell = Mine;
        };
WRun:
        if (popped) {
            popped := FALSE;
            either {
                cellTask := FALSE;
                finished := TRUE;
            } or {
                await yields < Yields;
                yields := yields + 1;
WRead:
                mine := IF cellLane < taskLane THEN cellLane ELSE taskLane;
                taskLane := mine;
WPush:
                where := mine;
                entryCell := Mine;
WReread:
                raised := cellLane;
WErase:
                if (raised < mine /\ where = 1 /\ entryCell = Mine) {
                    foreignMoved := foreignMoved \/ entryCell /= Mine;
                    where := None;
                    moved := TRUE;
                };
WMoved:
                if (moved) {
                    taskLane := raised;
                    where := raised;
                    entryCell := Mine;
                    moved := FALSE;
                };
            };
        };
    };
}

fair process (Boost = Waiter)
variables t = FALSE, wmoved = FALSE;
{
WaRaise:
    if (cellLane > 0) {
        cellLane := 0;
    } else {
        goto Done;
    };
WaLoad:
    t := cellTask;
WaSearch:
    if (t /\ where = 1 /\ entryCell = Mine) {
        foreignMoved := foreignMoved \/ entryCell /= Mine;
        where := None;
        wmoved := TRUE;
    };
WaPush:
    if (wmoved) {
        taskLane := 0;
        where := 0;
        entryCell := Mine;
    };
}

fair process (Reuse = Recycler)
{
WrReuse:
    await finished;
    assert where = None;
    where := 1;
    entryCell := Foreign;
}
} *)
\* BEGIN TRANSLATION
VARIABLES cellLane, cellTask, taskLane, where, finished, yields, entryCell, 
          foreignMoved, pc

(* define statement *)
TypeOK == /\ cellLane \in {0, 1}
          /\ taskLane \in {0, 1}
          /\ where \in {0, 1, None}
          /\ finished \in BOOLEAN
          /\ yields \in 0..Yields
          /\ entryCell \in {Mine, Foreign}
          /\ foreignMoved \in BOOLEAN


LaneMonotone == [][cellLane' <= cellLane]_cellLane

Quiescent == \A w \in Workers : pc[w] \in {"WLoop", "WPop", "WRun", "Done"}

BoostHolds == (pc[Waiter] = "Done" /\ ~finished /\ Quiescent) => where /= 1

Terminates == <>(\A p \in Workers \cup {Waiter} : pc[p] = "Done")

BoostSticks == (pc[Waiter] = "Done" /\ where = 1) ~> (where /= 1 \/ finished)

NoForeignPromote == ~foreignMoved

VARIABLES mine, raised, popped, moved, t, wmoved

vars == << cellLane, cellTask, taskLane, where, finished, yields, entryCell, 
           foreignMoved, pc, mine, raised, popped, moved, t, wmoved >>

ProcSet == (Workers) \cup {Waiter} \cup {Recycler}

Init == (* Global variables *)
        /\ cellLane = 1
        /\ cellTask = TRUE
        /\ taskLane = 1
        /\ where = 1
        /\ finished = FALSE
        /\ yields = 0
        /\ entryCell = Mine
        /\ foreignMoved = FALSE
        (* Process Worker *)
        /\ mine = [self \in Workers |-> 1]
        /\ raised = [self \in Workers |-> 1]
        /\ popped = [self \in Workers |-> FALSE]
        /\ moved = [self \in Workers |-> FALSE]
        (* Process Boost *)
        /\ t = FALSE
        /\ wmoved = FALSE
        /\ pc = [self \in ProcSet |-> CASE self \in Workers -> "WLoop"
                                        [] self = Waiter -> "WaRaise"
                                        [] self = Recycler -> "WrReuse"]

WLoop(self) == /\ pc[self] = "WLoop"
               /\ IF ~finished
                     THEN /\ pc' = [pc EXCEPT ![self] = "WPop"]
                     ELSE /\ pc' = [pc EXCEPT ![self] = "Done"]
               /\ UNCHANGED << cellLane, cellTask, taskLane, where, finished, 
                               yields, entryCell, foreignMoved, mine, raised, 
                               popped, moved, t, wmoved >>

WPop(self) == /\ pc[self] = "WPop"
              /\ where /= None \/ finished
              /\ IF where /= None
                    THEN /\ where' = None
                         /\ popped' = [popped EXCEPT ![self] = entryCell = Mine]
                    ELSE /\ TRUE
                         /\ UNCHANGED << where, popped >>
              /\ pc' = [pc EXCEPT ![self] = "WRun"]
              /\ UNCHANGED << cellLane, cellTask, taskLane, finished, yields, 
                              entryCell, foreignMoved, mine, raised, moved, t, 
                              wmoved >>

WRun(self) == /\ pc[self] = "WRun"
              /\ IF popped[self]
                    THEN /\ popped' = [popped EXCEPT ![self] = FALSE]
                         /\ \/ /\ cellTask' = FALSE
                               /\ finished' = TRUE
                               /\ pc' = [pc EXCEPT ![self] = "WLoop"]
                               /\ UNCHANGED yields
                            \/ /\ yields < Yields
                               /\ yields' = yields + 1
                               /\ pc' = [pc EXCEPT ![self] = "WRead"]
                               /\ UNCHANGED <<cellTask, finished>>
                    ELSE /\ pc' = [pc EXCEPT ![self] = "WLoop"]
                         /\ UNCHANGED << cellTask, finished, yields, popped >>
              /\ UNCHANGED << cellLane, taskLane, where, entryCell, 
                              foreignMoved, mine, raised, moved, t, wmoved >>

WRead(self) == /\ pc[self] = "WRead"
               /\ mine' = [mine EXCEPT ![self] = IF cellLane < taskLane THEN cellLane ELSE taskLane]
               /\ taskLane' = mine'[self]
               /\ pc' = [pc EXCEPT ![self] = "WPush"]
               /\ UNCHANGED << cellLane, cellTask, where, finished, yields, 
                               entryCell, foreignMoved, raised, popped, moved, 
                               t, wmoved >>

WPush(self) == /\ pc[self] = "WPush"
               /\ where' = mine[self]
               /\ entryCell' = Mine
               /\ pc' = [pc EXCEPT ![self] = "WReread"]
               /\ UNCHANGED << cellLane, cellTask, taskLane, finished, yields, 
                               foreignMoved, mine, raised, popped, moved, t, 
                               wmoved >>

WReread(self) == /\ pc[self] = "WReread"
                 /\ raised' = [raised EXCEPT ![self] = cellLane]
                 /\ pc' = [pc EXCEPT ![self] = "WErase"]
                 /\ UNCHANGED << cellLane, cellTask, taskLane, where, finished, 
                                 yields, entryCell, foreignMoved, mine, popped, 
                                 moved, t, wmoved >>

WErase(self) == /\ pc[self] = "WErase"
                /\ IF raised[self] < mine[self] /\ where = 1 /\ entryCell = Mine
                      THEN /\ foreignMoved' = (foreignMoved \/ entryCell /= Mine)
                           /\ where' = None
                           /\ moved' = [moved EXCEPT ![self] = TRUE]
                      ELSE /\ TRUE
                           /\ UNCHANGED << where, foreignMoved, moved >>
                /\ pc' = [pc EXCEPT ![self] = "WMoved"]
                /\ UNCHANGED << cellLane, cellTask, taskLane, finished, yields, 
                                entryCell, mine, raised, popped, t, wmoved >>

WMoved(self) == /\ pc[self] = "WMoved"
                /\ IF moved[self]
                      THEN /\ taskLane' = raised[self]
                           /\ where' = raised[self]
                           /\ entryCell' = Mine
                           /\ moved' = [moved EXCEPT ![self] = FALSE]
                      ELSE /\ TRUE
                           /\ UNCHANGED << taskLane, where, entryCell, moved >>
                /\ pc' = [pc EXCEPT ![self] = "WLoop"]
                /\ UNCHANGED << cellLane, cellTask, finished, yields, 
                                foreignMoved, mine, raised, popped, t, wmoved >>

Worker(self) == WLoop(self) \/ WPop(self) \/ WRun(self) \/ WRead(self)
                   \/ WPush(self) \/ WReread(self) \/ WErase(self)
                   \/ WMoved(self)

WaRaise == /\ pc[Waiter] = "WaRaise"
           /\ IF cellLane > 0
                 THEN /\ cellLane' = 0
                      /\ pc' = [pc EXCEPT ![Waiter] = "WaLoad"]
                 ELSE /\ pc' = [pc EXCEPT ![Waiter] = "Done"]
                      /\ UNCHANGED cellLane
           /\ UNCHANGED << cellTask, taskLane, where, finished, yields, 
                           entryCell, foreignMoved, mine, raised, popped, 
                           moved, t, wmoved >>

WaLoad == /\ pc[Waiter] = "WaLoad"
          /\ t' = cellTask
          /\ pc' = [pc EXCEPT ![Waiter] = "WaSearch"]
          /\ UNCHANGED << cellLane, cellTask, taskLane, where, finished, 
                          yields, entryCell, foreignMoved, mine, raised, 
                          popped, moved, wmoved >>

WaSearch == /\ pc[Waiter] = "WaSearch"
            /\ IF t /\ where = 1 /\ entryCell = Mine
                  THEN /\ foreignMoved' = (foreignMoved \/ entryCell /= Mine)
                       /\ where' = None
                       /\ wmoved' = TRUE
                  ELSE /\ TRUE
                       /\ UNCHANGED << where, foreignMoved, wmoved >>
            /\ pc' = [pc EXCEPT ![Waiter] = "WaPush"]
            /\ UNCHANGED << cellLane, cellTask, taskLane, finished, yields, 
                            entryCell, mine, raised, popped, moved, t >>

WaPush == /\ pc[Waiter] = "WaPush"
          /\ IF wmoved
                THEN /\ taskLane' = 0
                     /\ where' = 0
                     /\ entryCell' = Mine
                ELSE /\ TRUE
                     /\ UNCHANGED << taskLane, where, entryCell >>
          /\ pc' = [pc EXCEPT ![Waiter] = "Done"]
          /\ UNCHANGED << cellLane, cellTask, finished, yields, foreignMoved, 
                          mine, raised, popped, moved, t, wmoved >>

Boost == WaRaise \/ WaLoad \/ WaSearch \/ WaPush

WrReuse == /\ pc[Recycler] = "WrReuse"
           /\ finished
           /\ Assert(where = None, 
                     "Failure of assertion at line 196, column 5.")
           /\ where' = 1
           /\ entryCell' = Foreign
           /\ pc' = [pc EXCEPT ![Recycler] = "Done"]
           /\ UNCHANGED << cellLane, cellTask, taskLane, finished, yields, 
                           foreignMoved, mine, raised, popped, moved, t, 
                           wmoved >>

Reuse == WrReuse

(* Allow infinite stuttering to prevent deadlock on termination. *)
Terminating == /\ \A self \in ProcSet: pc[self] = "Done"
               /\ UNCHANGED vars

Next == Boost \/ Reuse
           \/ (\E self \in Workers: Worker(self))
           \/ Terminating

Spec == /\ Init /\ [][Next]_vars
        /\ WF_vars(Next)
        /\ \A self \in Workers : WF_vars(Worker(self))
        /\ WF_vars(Boost)
        /\ WF_vars(Reuse)

Termination == <>(\A self \in ProcSet: pc[self] = "Done")

\* END TRANSLATION
=============================================================================
