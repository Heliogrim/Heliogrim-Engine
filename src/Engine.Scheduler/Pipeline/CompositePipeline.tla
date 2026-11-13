------------------------- MODULE CompositePipeline -------------------------
(* Model of the task-graph runner's counters in
   src/Engine.Scheduler/Pipeline/CompositePipeline.cpp (startFrame, dispatch,
   complete, the frame end in complete, stop) and
   src/Engine.Scheduler/Pipeline/Composite/CompositeSlot.cpp (onComplete).
   Graph: A -> B, A -> C, B -> D, C -> D, D -> E; E has no tasks (the dummy
   task path is one step like any other).
   Folded: a node's tasks are one task, so CompositeSlot::_signals is not
   modelled and "the slot's last signal fired" is one step with the task
   body. dispatch() plus submit() is one step (adding the node to ready).
   startFrame() is one step: no node is in flight when it runs.
   schedule->tick() is not modelled (it touches no counter and reads only
   the local copy of _schedule).
   Only the unbounded mode (_maxInFlight == 0) is modelled; the serial mode
   (_maxInFlight == 1, a cursor over graph.order) is not.
   Differences to the code:
   - Bound for TLC: frame MaxFrames does not restart; its frame end waits
     for _stopping instead, so the only way to the stop bit (and to the
     release) is through stop(). The code restarts frames until _stopping.
   - _stopping.test(), _stopping.clear() and _activeCalls.fetch_or are one
     step (FrameEnd). Sound because the frame-ending call is still counted:
     another call's fetch_sub between them cannot see (stoppedBit | 1), so
     no release can happen in the gap. clear() is not modelled (nothing reads
     it after the stop bit is set).
   - Workers leave the loop once the stop bit is set. The code's workers
     keep running other tasks; after the frame end no runner call starts
     (no node is ready and the frame was not restarted), so the model loses
     no runner interleaving by stopping them.
   - stop()'s tail (_activeCalls.store(0), _stopped.reset()) and a restart
     through start() after stop() are not modelled: one stop cycle only.
   - Sequential consistency. The relaxed fetch_add, the relaxed fetch_or and
     the relaxed store(0) were argued by hand (the fetch_add is sequenced
     before the acq_rel _remaining fetch_sub; the fetch_or and the leaving
     acq_rel fetch_sub are RMWs on one word; set()/wait() publish to stop()),
     not model-checked. Relacy (2b) is pending.
   _activeCalls is split into `active` (the count) and `stopBit`
   (stoppedBit); a fetch_sub reads both in one step, as the one word does.
   The releaser's _stopped.set() is one step with its fetch_sub: set() is
   its last access, and the Signal destroy rule lets stop() return while
   set() unwinds. stop()'s _stopped.wait() is Signal.tla's; here an await.
   Checks: a node is dispatched at most once per frame, _remaining reaches 0
   only when every node is done and never drops below the not-done count,
   a started frame ends, stop() returns, and once it returns no worker is
   inside a runner call (the pipeline may be destroyed). The interleaving
   where the frame-ending call is not the last to leave is covered here
   (NoCallAfterRelease), not by a gtest: no stage body can hold a runner
   call inside complete().
   Mutations: (a) split the successor fetch_sub into a decrement and a later
   read of pending (fails NoDoubleDispatch); (b) run the frame end on
   remaining = 1 before the _remaining fetch_sub (fails RemainingCountsDone);
   (c) release at the frame end, the old protocol (fails
   NoCallAfterRelease); (d) read the stop bit and the count in a load after
   the fetch_sub (fails NoCallAfterRelease); (e) the frame end under stop
   does not set the stop bit (deadlocks: stop() never returns). *)
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
    active = 0,                                 \* _activeCalls count
    stopBit = FALSE,                            \* _activeCalls & stoppedBit
    released = FALSE,                           \* _stopped
    inCall = [w \in Workers |-> FALSE];         \* worker between enter and leave

define
    NoDoubleDispatch == \A n \in Nodes : dispatched[n] <= 1
    RemainingCountsDone ==
        /\ (remaining = 0 => \A n \in Nodes : done[n])
        /\ remaining >= Cardinality({n \in Nodes : ~done[n]})
    NoCallAfterRelease == released => \A w \in Workers : ~inCall[w]
    FrameMakesProgress == remaining > 0 ~> (remaining = 0 \/ stopBit)
    StopReturns == <>(pc[Stopper] = "Done")
end define;

fair process worker \in Workers
variables cur = NoNode, succs = {};
begin
Loop:
    while ~stopBit do
Take:                                       \* task body + onComplete fetch_sub == 1
        await ready /= {} \/ stopBit;
        if ready /= {} then
            with n \in ready do
                ready := ready \ {n};
                done[n] := TRUE;
                cur := n;
                succs := Succ(n);
            end with;
Enter:                                      \* _activeCalls.fetch_add(1)
            active := active + 1;
            inCall[self] := TRUE;
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
                await stopping \/ frame < MaxFrames; \* bound: the last frame only ends under stop
                if stopping then
                    stopBit := TRUE;        \* _activeCalls.fetch_or(stoppedBit)
                else                        \* startFrame()
                    pending := [n \in Nodes |-> Preds(n)];
                    remaining := Cardinality(Nodes);
                    dispatched := [n \in Nodes |-> IF n = Root THEN 1 ELSE 0];
                    done := [n \in Nodes |-> FALSE];
                    ready := {Root};
                    frame := frame + 1;
                end if;
            end if;
Leave:                                      \* prev = _activeCalls.fetch_sub(1)
            if stopBit /\ active = 1 then   \* prev == (stoppedBit | 1)
                released := TRUE;           \* _stopped.set()
            end if;
            active := active - 1;
            inCall[self] := FALSE;
        end if;
    end while;
end process;

fair process stopper = Stopper
begin
Stop:                                       \* _stopping.test_and_set()
    stopping := TRUE;
Wait:                                       \* _stopped.wait()
    await released;
end process;
end algorithm; *)
\* BEGIN TRANSLATION (chksum(pcal) = "cedc58c0" /\ chksum(tla) = "c0555015")
VARIABLES pending, remaining, dispatched, done, ready, frame, stopping, 
          active, stopBit, released, inCall, pc

(* define statement *)
NoDoubleDispatch == \A n \in Nodes : dispatched[n] <= 1
RemainingCountsDone ==
    /\ (remaining = 0 => \A n \in Nodes : done[n])
    /\ remaining >= Cardinality({n \in Nodes : ~done[n]})
NoCallAfterRelease == released => \A w \in Workers : ~inCall[w]
FrameMakesProgress == remaining > 0 ~> (remaining = 0 \/ stopBit)
StopReturns == <>(pc[Stopper] = "Done")

VARIABLES cur, succs

vars == << pending, remaining, dispatched, done, ready, frame, stopping, 
           active, stopBit, released, inCall, pc, cur, succs >>

ProcSet == (Workers) \cup {Stopper}

Init == (* Global variables *)
        /\ pending = [n \in Nodes |-> Preds(n)]
        /\ remaining = Cardinality(Nodes)
        /\ dispatched = [n \in Nodes |-> IF n = Root THEN 1 ELSE 0]
        /\ done = [n \in Nodes |-> FALSE]
        /\ ready = {Root}
        /\ frame = 1
        /\ stopping = FALSE
        /\ active = 0
        /\ stopBit = FALSE
        /\ released = FALSE
        /\ inCall = [w \in Workers |-> FALSE]
        (* Process worker *)
        /\ cur = [self \in Workers |-> NoNode]
        /\ succs = [self \in Workers |-> {}]
        /\ pc = [self \in ProcSet |-> CASE self \in Workers -> "Loop"
                                        [] self = Stopper -> "Stop"]

Loop(self) == /\ pc[self] = "Loop"
              /\ IF ~stopBit
                    THEN /\ pc' = [pc EXCEPT ![self] = "Take"]
                    ELSE /\ pc' = [pc EXCEPT ![self] = "Done"]
              /\ UNCHANGED << pending, remaining, dispatched, done, ready, 
                              frame, stopping, active, stopBit, released, 
                              inCall, cur, succs >>

Take(self) == /\ pc[self] = "Take"
              /\ ready /= {} \/ stopBit
              /\ IF ready /= {}
                    THEN /\ \E n \in ready:
                              /\ ready' = ready \ {n}
                              /\ done' = [done EXCEPT ![n] = TRUE]
                              /\ cur' = [cur EXCEPT ![self] = n]
                              /\ succs' = [succs EXCEPT ![self] = Succ(n)]
                         /\ pc' = [pc EXCEPT ![self] = "Enter"]
                    ELSE /\ pc' = [pc EXCEPT ![self] = "Loop"]
                         /\ UNCHANGED << done, ready, cur, succs >>
              /\ UNCHANGED << pending, remaining, dispatched, frame, stopping, 
                              active, stopBit, released, inCall >>

Enter(self) == /\ pc[self] = "Enter"
               /\ active' = active + 1
               /\ inCall' = [inCall EXCEPT ![self] = TRUE]
               /\ pc' = [pc EXCEPT ![self] = "Succs"]
               /\ UNCHANGED << pending, remaining, dispatched, done, ready, 
                               frame, stopping, stopBit, released, cur, succs >>

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
               /\ UNCHANGED << remaining, done, frame, stopping, active, 
                               stopBit, released, inCall, cur >>

Remaining(self) == /\ pc[self] = "Remaining"
                   /\ cur' = [cur EXCEPT ![self] = NoNode]
                   /\ remaining' = remaining - 1
                   /\ IF remaining' = 0
                         THEN /\ pc' = [pc EXCEPT ![self] = "FrameEnd"]
                         ELSE /\ pc' = [pc EXCEPT ![self] = "Leave"]
                   /\ UNCHANGED << pending, dispatched, done, ready, frame, 
                                   stopping, active, stopBit, released, inCall, 
                                   succs >>

FrameEnd(self) == /\ pc[self] = "FrameEnd"
                  /\ stopping \/ frame < MaxFrames
                  /\ IF stopping
                        THEN /\ stopBit' = TRUE
                             /\ UNCHANGED << pending, remaining, dispatched, 
                                             done, ready, frame >>
                        ELSE /\ pending' = [n \in Nodes |-> Preds(n)]
                             /\ remaining' = Cardinality(Nodes)
                             /\ dispatched' = [n \in Nodes |-> IF n = Root THEN 1 ELSE 0]
                             /\ done' = [n \in Nodes |-> FALSE]
                             /\ ready' = {Root}
                             /\ frame' = frame + 1
                             /\ UNCHANGED stopBit
                  /\ pc' = [pc EXCEPT ![self] = "Leave"]
                  /\ UNCHANGED << stopping, active, released, inCall, cur, 
                                  succs >>

Leave(self) == /\ pc[self] = "Leave"
               /\ IF stopBit /\ active = 1
                     THEN /\ released' = TRUE
                     ELSE /\ TRUE
                          /\ UNCHANGED released
               /\ active' = active - 1
               /\ inCall' = [inCall EXCEPT ![self] = FALSE]
               /\ pc' = [pc EXCEPT ![self] = "Loop"]
               /\ UNCHANGED << pending, remaining, dispatched, done, ready, 
                               frame, stopping, stopBit, cur, succs >>

worker(self) == Loop(self) \/ Take(self) \/ Enter(self) \/ Succs(self)
                   \/ Remaining(self) \/ FrameEnd(self) \/ Leave(self)

Stop == /\ pc[Stopper] = "Stop"
        /\ stopping' = TRUE
        /\ pc' = [pc EXCEPT ![Stopper] = "Wait"]
        /\ UNCHANGED << pending, remaining, dispatched, done, ready, frame, 
                        active, stopBit, released, inCall, cur, succs >>

Wait == /\ pc[Stopper] = "Wait"
        /\ released
        /\ pc' = [pc EXCEPT ![Stopper] = "Done"]
        /\ UNCHANGED << pending, remaining, dispatched, done, ready, frame, 
                        stopping, active, stopBit, released, inCall, cur, 
                        succs >>

stopper == Stop \/ Wait

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
