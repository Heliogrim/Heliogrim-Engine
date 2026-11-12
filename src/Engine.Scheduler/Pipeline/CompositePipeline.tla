------------------------- MODULE CompositePipeline -------------------------
(* Model of the task-graph runner's counters in
   src/Engine.Scheduler/Pipeline/CompositePipeline.cpp (startFrame, dispatch,
   complete, the frame end in complete) and
   src/Engine.Scheduler/Pipeline/Composite/CompositeSlot.cpp (onComplete).
   Graph: A -> B, A -> C, B -> D, C -> D, D -> E; E has no tasks (the dummy
   task path is one step like any other).
   Folded: a node's tasks are one task, so CompositeSlot::_signals is not
   modelled and "the slot's last signal fired" is one step with the task
   body. dispatch() plus submit() is one step (adding the node to ready).
   startFrame() is one step: no node is in flight when it runs.
   schedule->tick() is not modelled (it touches no counter).
   Only the unbounded mode (_maxInFlight == 0) is modelled; the serial mode
   (_maxInFlight == 1, a cursor over graph.order) is not.
   Difference to the code: the frame end also stops after MaxFrames frames
   (a bound for TLC; the code only stops on _stopping). _stopping.clear() is
   not modelled (nothing reads it after _stopped is set).
   There is no park or wait in this protocol, so there is no wakeup-safety
   invariant; stop()'s _stopped.wait() is Signal.tla's.
   Assumption: the pipeline outlives every runner call; stop() waits for
   in-flight calls (_activeCalls), so steps after the last counter operation
   are not modelled.
   Checks: a node is dispatched at most once per frame, _remaining reaches 0
   only when every node is done and never drops below the not-done count,
   and a started frame ends.
   Mutations: (a) split the successor fetch_sub into a decrement and a later
   read of pending (fails NoDoubleDispatch); (b) run the frame end on
   remaining = 1 before the _remaining fetch_sub (fails RemainingCountsDone). *)
EXTENDS Naturals, FiniteSets, TLC

CONSTANTS Workers, MaxFrames
Stopper == 0
NoNode == "none"
Root == "A"
Nodes == {"A", "B", "C", "D", "E"}
Edges == {<<"A", "B">>, <<"A", "C">>, <<"B", "D">>, <<"C", "D">>, <<"D", "E">>}
Succ(n) == {e[2] : e \in {x \in Edges : x[1] = n}}
Preds(n) == Cardinality({e \in Edges : e[2] = n})

(* --algorithm CompositePipeline
variables
    pending = [n \in Nodes |-> Preds(n)],       \* _pending[]
    remaining = Cardinality(Nodes),             \* _remaining
    dispatched = [n \in Nodes |-> IF n = Root THEN 1 ELSE 0], \* per frame
    done = [n \in Nodes |-> FALSE],
    ready = {Root},                             \* dispatched, not completed
    frame = 1,
    stopping = FALSE,                           \* _stopping
    stopped = FALSE;                            \* _stopped

define
    NoDoubleDispatch == \A n \in Nodes : dispatched[n] <= 1
    RemainingCountsDone ==
        /\ (remaining = 0 => \A n \in Nodes : done[n])
        /\ remaining >= Cardinality({n \in Nodes : ~done[n]})
    FrameMakesProgress == remaining > 0 ~> (remaining = 0 \/ stopped)
end define;

fair process worker \in Workers
variables cur = NoNode, succs = {};
begin
Loop:
    while ~stopped do
Take:                                       \* task body + onComplete fetch_sub == 1
        await ready /= {} \/ stopped;
        if ready /= {} then
            with n \in ready do
                ready := ready \ {n};
                done[n] := TRUE;
                cur := n;
                succs := Succ(n);
            end with;
Succs:                                      \* complete(): one fetch_sub per successor
            while succs /= {} do
                with s = CHOOSE x \in succs : TRUE, prev = pending[s] do
                    pending[s] := prev - 1;
                    if prev = 1 then        \* dispatch(successor)
                        ready := ready \cup {s};
                        dispatched[s] := dispatched[s] + 1;
                    end if;
                    succs := succs \ {s};
                end with;
            end while;
Remaining:                                  \* _remaining.fetch_sub(1)
            cur := NoNode;
            remaining := remaining - 1;
            if remaining = 0 then
FrameEnd:                                   \* prevRemaining == 1
                if stopping \/ frame = MaxFrames then
                    stopped := TRUE;
                else                        \* startFrame()
                    pending := [n \in Nodes |-> Preds(n)];
                    remaining := Cardinality(Nodes);
                    dispatched := [n \in Nodes |-> IF n = Root THEN 1 ELSE 0];
                    done := [n \in Nodes |-> FALSE];
                    ready := {Root};
                    frame := frame + 1;
                end if;
            end if;
        end if;
    end while;
end process;

fair process stopper = Stopper              \* stop(): _stopping.test_and_set()
begin
Stop:
    stopping := TRUE;
end process;
end algorithm; *)
\* BEGIN TRANSLATION (chksum(pcal) = "82130c46" /\ chksum(tla) = "1ce50225")
VARIABLES pending, remaining, dispatched, done, ready, frame, stopping, 
          stopped, pc

(* define statement *)
NoDoubleDispatch == \A n \in Nodes : dispatched[n] <= 1
RemainingCountsDone ==
    /\ (remaining = 0 => \A n \in Nodes : done[n])
    /\ remaining >= Cardinality({n \in Nodes : ~done[n]})
FrameMakesProgress == remaining > 0 ~> (remaining = 0 \/ stopped)

VARIABLES cur, succs

vars == << pending, remaining, dispatched, done, ready, frame, stopping, 
           stopped, pc, cur, succs >>

ProcSet == (Workers) \cup {Stopper}

Init == (* Global variables *)
        /\ pending = [n \in Nodes |-> Preds(n)]
        /\ remaining = Cardinality(Nodes)
        /\ dispatched = [n \in Nodes |-> IF n = Root THEN 1 ELSE 0]
        /\ done = [n \in Nodes |-> FALSE]
        /\ ready = {Root}
        /\ frame = 1
        /\ stopping = FALSE
        /\ stopped = FALSE
        (* Process worker *)
        /\ cur = [self \in Workers |-> NoNode]
        /\ succs = [self \in Workers |-> {}]
        /\ pc = [self \in ProcSet |-> CASE self \in Workers -> "Loop"
                                        [] self = Stopper -> "Stop"]

Loop(self) == /\ pc[self] = "Loop"
              /\ IF ~stopped
                    THEN /\ pc' = [pc EXCEPT ![self] = "Take"]
                    ELSE /\ pc' = [pc EXCEPT ![self] = "Done"]
              /\ UNCHANGED << pending, remaining, dispatched, done, ready, 
                              frame, stopping, stopped, cur, succs >>

Take(self) == /\ pc[self] = "Take"
              /\ ready /= {} \/ stopped
              /\ IF ready /= {}
                    THEN /\ \E n \in ready:
                              /\ ready' = ready \ {n}
                              /\ done' = [done EXCEPT ![n] = TRUE]
                              /\ cur' = [cur EXCEPT ![self] = n]
                              /\ succs' = [succs EXCEPT ![self] = Succ(n)]
                         /\ pc' = [pc EXCEPT ![self] = "Succs"]
                    ELSE /\ pc' = [pc EXCEPT ![self] = "Loop"]
                         /\ UNCHANGED << done, ready, cur, succs >>
              /\ UNCHANGED << pending, remaining, dispatched, frame, stopping, 
                              stopped >>

Succs(self) == /\ pc[self] = "Succs"
               /\ IF succs[self] /= {}
                     THEN /\ LET s == CHOOSE x \in succs[self] : TRUE IN
                               LET prev == pending[s] IN
                                 /\ pending' = [pending EXCEPT ![s] = prev - 1]
                                 /\ IF prev = 1
                                       THEN /\ ready' = (ready \cup {s})
                                            /\ dispatched' = [dispatched EXCEPT ![s] = dispatched[s] + 1]
                                       ELSE /\ TRUE
                                            /\ UNCHANGED << dispatched, ready >>
                                 /\ succs' = [succs EXCEPT ![self] = succs[self] \ {s}]
                          /\ pc' = [pc EXCEPT ![self] = "Succs"]
                     ELSE /\ pc' = [pc EXCEPT ![self] = "Remaining"]
                          /\ UNCHANGED << pending, dispatched, ready, succs >>
               /\ UNCHANGED << remaining, done, frame, stopping, stopped, cur >>

Remaining(self) == /\ pc[self] = "Remaining"
                   /\ cur' = [cur EXCEPT ![self] = NoNode]
                   /\ remaining' = remaining - 1
                   /\ IF remaining' = 0
                         THEN /\ pc' = [pc EXCEPT ![self] = "FrameEnd"]
                         ELSE /\ pc' = [pc EXCEPT ![self] = "Loop"]
                   /\ UNCHANGED << pending, dispatched, done, ready, frame, 
                                   stopping, stopped, succs >>

FrameEnd(self) == /\ pc[self] = "FrameEnd"
                  /\ IF stopping \/ frame = MaxFrames
                        THEN /\ stopped' = TRUE
                             /\ UNCHANGED << pending, remaining, dispatched, 
                                             done, ready, frame >>
                        ELSE /\ pending' = [n \in Nodes |-> Preds(n)]
                             /\ remaining' = Cardinality(Nodes)
                             /\ dispatched' = [n \in Nodes |-> IF n = Root THEN 1 ELSE 0]
                             /\ done' = [n \in Nodes |-> FALSE]
                             /\ ready' = {Root}
                             /\ frame' = frame + 1
                             /\ UNCHANGED stopped
                  /\ pc' = [pc EXCEPT ![self] = "Loop"]
                  /\ UNCHANGED << stopping, cur, succs >>

worker(self) == Loop(self) \/ Take(self) \/ Succs(self) \/ Remaining(self)
                   \/ FrameEnd(self)

Stop == /\ pc[Stopper] = "Stop"
        /\ stopping' = TRUE
        /\ pc' = [pc EXCEPT ![Stopper] = "Done"]
        /\ UNCHANGED << pending, remaining, dispatched, done, ready, frame, 
                        stopped, cur, succs >>

stopper == Stop

(* Allow infinite stuttering to prevent deadlock on termination. *)
Terminating == /\ \A self \in ProcSet: pc[self] = "Done"
               /\ UNCHANGED vars

Next == stopper
           \/ (\E self \in Workers: worker(self))
           \/ Terminating

Spec == /\ Init /\ [][Next]_vars
        /\ \A self \in Workers : WF_vars(worker(self))
        /\ WF_vars(stopper)

Termination == <>(\A self \in ProcSet: pc[self] = "Done")

\* END TRANSLATION 
=============================================================================
