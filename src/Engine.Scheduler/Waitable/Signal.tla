------------------------------ MODULE Signal ------------------------------
(* Model of src/Engine.Scheduler/Waitable/Signal.cpp: one setter, N waiters,
   plus the park/unpark handshake (Park.cpp, Worker.cpp). Checks when the
   object may be destroyed: a waiter whose wait() returned may destroy it
   once every other waiter has returned too (the std::latch rule in
   Signal.hpp), and every later access to _state, _closed or _waiters by
   anyone is an assert failure.
   Mutation check: weaken MayDestroy to "no waiter is still at WFast" (the
   header's earlier wording, "safe once wait() returned"); a waiter past the
   _state fast check then takes the _waiters lock on the destroyed object.
   reset() is not modelled (documented as unsynchronised).                   *)
EXTENDS Naturals, Sequences, FiniteSets, TLC

CONSTANT N
Waiters == 1..N
Setter == 0
None == 0

(* --algorithm Signal
variables
    state = 0,              \* _state
    closed = FALSE,         \* _closed (under the queue lock)
    qlock = FALSE,          \* ParkQueue::_lck
    queue = << >>,          \* _waiters
    destroyed = FALSE,      \* a waiter destroyed the Signal after wait() returned
    node = [w \in Waiters |-> "Idle"],
    runnable = [w \in Waiters |-> TRUE];

define
    AllDone == \A w \in Waiters : pc[w] = "Done"
    EventuallyAllDone == <>AllDone
    NoLostWakeup ==
        \A w \in Waiters : (node[w] = "Parked" /\ ~runnable[w]) ~> runnable[w]
    MayDestroy == \A o \in Waiters : pc[o] \in {"Finish", "Done"}
end define;

fair process setter = Setter
variables batch = << >>, head = None;
begin
SAcquireQ:
    assert ~destroyed;
    await ~qlock;
    qlock := TRUE;
STakeAll:
    assert ~destroyed;
    closed := TRUE;
    batch := queue;
    queue := << >>;
SReleaseQ:
    assert ~destroyed;
    qlock := FALSE;
SStore:                                     \* last access to *this
    assert ~destroyed;
    state := 1;
SUnpark:
    while batch /= << >> do
        head := Head(batch);
        batch := Tail(batch);               \* next read before unpark
        assert node[head] \in {"Armed", "Parked"};
        if node[head] = "Parked" then
            runnable[head] := TRUE;
        end if;
        node[head] := "Woken";
    end while;
end process;

fair process waiter \in Waiters
variables pushed = FALSE;
begin
WFast:
    assert ~destroyed;
    if state = 1 then
        goto Finish;
    end if;
WArm:
    node[self] := "Armed";
WAcquireQ:
    assert ~destroyed;
    await ~qlock;
    qlock := TRUE;
WPush:
    assert ~destroyed;
    if closed then
        pushed := FALSE;
    else
        queue := Append(queue, self);
        pushed := TRUE;
    end if;
WReleaseQ:
    assert ~destroyed;
    qlock := FALSE;
    if ~pushed then
        goto Spin;
    end if;
ParkSwitch:
    if node[self] = "Woken" then
        node[self] := "Parked";
    else
        node[self] := "Parked";
        runnable[self] := FALSE;
    end if;
Resume:
    await runnable[self];
    node[self] := "Idle";
    goto Finish;
Spin:                                       \* "the setter's store is imminent"
    assert ~destroyed;
    if state = 1 then
        node[self] := "Idle";
        goto Finish;
    else
        goto Spin;
    end if;
Finish:
    either
        await ~destroyed /\ MayDestroy;
        destroyed := TRUE;
    or
        skip;
    end either;
end process;
end algorithm; *)
\* BEGIN TRANSLATION (chksum(pcal) = "78f0fba" /\ chksum(tla) = "4dff271b")
VARIABLES state, closed, qlock, queue, destroyed, node, runnable, pc

(* define statement *)
AllDone == \A w \in Waiters : pc[w] = "Done"
EventuallyAllDone == <>AllDone
NoLostWakeup ==
    \A w \in Waiters : (node[w] = "Parked" /\ ~runnable[w]) ~> runnable[w]
MayDestroy == \A o \in Waiters : pc[o] \in {"Finish", "Done"}

VARIABLES batch, head, pushed

vars == << state, closed, qlock, queue, destroyed, node, runnable, pc, batch, 
           head, pushed >>

ProcSet == {Setter} \cup (Waiters)

Init == (* Global variables *)
        /\ state = 0
        /\ closed = FALSE
        /\ qlock = FALSE
        /\ queue = << >>
        /\ destroyed = FALSE
        /\ node = [w \in Waiters |-> "Idle"]
        /\ runnable = [w \in Waiters |-> TRUE]
        (* Process setter *)
        /\ batch = << >>
        /\ head = None
        (* Process waiter *)
        /\ pushed = [self \in Waiters |-> FALSE]
        /\ pc = [self \in ProcSet |-> CASE self = Setter -> "SAcquireQ"
                                        [] self \in Waiters -> "WFast"]

SAcquireQ == /\ pc[Setter] = "SAcquireQ"
             /\ Assert(~destroyed, 
                       "Failure of assertion at line 41, column 5.")
             /\ ~qlock
             /\ qlock' = TRUE
             /\ pc' = [pc EXCEPT ![Setter] = "STakeAll"]
             /\ UNCHANGED << state, closed, queue, destroyed, node, runnable, 
                             batch, head, pushed >>

STakeAll == /\ pc[Setter] = "STakeAll"
            /\ Assert(~destroyed, 
                      "Failure of assertion at line 45, column 5.")
            /\ closed' = TRUE
            /\ batch' = queue
            /\ queue' = << >>
            /\ pc' = [pc EXCEPT ![Setter] = "SReleaseQ"]
            /\ UNCHANGED << state, qlock, destroyed, node, runnable, head, 
                            pushed >>

SReleaseQ == /\ pc[Setter] = "SReleaseQ"
             /\ Assert(~destroyed, 
                       "Failure of assertion at line 50, column 5.")
             /\ qlock' = FALSE
             /\ pc' = [pc EXCEPT ![Setter] = "SStore"]
             /\ UNCHANGED << state, closed, queue, destroyed, node, runnable, 
                             batch, head, pushed >>

SStore == /\ pc[Setter] = "SStore"
          /\ Assert(~destroyed, "Failure of assertion at line 53, column 5.")
          /\ state' = 1
          /\ pc' = [pc EXCEPT ![Setter] = "SUnpark"]
          /\ UNCHANGED << closed, qlock, queue, destroyed, node, runnable, 
                          batch, head, pushed >>

SUnpark == /\ pc[Setter] = "SUnpark"
           /\ IF batch /= << >>
                 THEN /\ head' = Head(batch)
                      /\ batch' = Tail(batch)
                      /\ Assert(node[head'] \in {"Armed", "Parked"}, 
                                "Failure of assertion at line 59, column 9.")
                      /\ IF node[head'] = "Parked"
                            THEN /\ runnable' = [runnable EXCEPT ![head'] = TRUE]
                            ELSE /\ TRUE
                                 /\ UNCHANGED runnable
                      /\ node' = [node EXCEPT ![head'] = "Woken"]
                      /\ pc' = [pc EXCEPT ![Setter] = "SUnpark"]
                 ELSE /\ pc' = [pc EXCEPT ![Setter] = "Done"]
                      /\ UNCHANGED << node, runnable, batch, head >>
           /\ UNCHANGED << state, closed, qlock, queue, destroyed, pushed >>

setter == SAcquireQ \/ STakeAll \/ SReleaseQ \/ SStore \/ SUnpark

WFast(self) == /\ pc[self] = "WFast"
               /\ Assert(~destroyed, 
                         "Failure of assertion at line 71, column 5.")
               /\ IF state = 1
                     THEN /\ pc' = [pc EXCEPT ![self] = "Finish"]
                     ELSE /\ pc' = [pc EXCEPT ![self] = "WArm"]
               /\ UNCHANGED << state, closed, qlock, queue, destroyed, node, 
                               runnable, batch, head, pushed >>

WArm(self) == /\ pc[self] = "WArm"
              /\ node' = [node EXCEPT ![self] = "Armed"]
              /\ pc' = [pc EXCEPT ![self] = "WAcquireQ"]
              /\ UNCHANGED << state, closed, qlock, queue, destroyed, runnable, 
                              batch, head, pushed >>

WAcquireQ(self) == /\ pc[self] = "WAcquireQ"
                   /\ Assert(~destroyed, 
                             "Failure of assertion at line 78, column 5.")
                   /\ ~qlock
                   /\ qlock' = TRUE
                   /\ pc' = [pc EXCEPT ![self] = "WPush"]
                   /\ UNCHANGED << state, closed, queue, destroyed, node, 
                                   runnable, batch, head, pushed >>

WPush(self) == /\ pc[self] = "WPush"
               /\ Assert(~destroyed, 
                         "Failure of assertion at line 82, column 5.")
               /\ IF closed
                     THEN /\ pushed' = [pushed EXCEPT ![self] = FALSE]
                          /\ queue' = queue
                     ELSE /\ queue' = Append(queue, self)
                          /\ pushed' = [pushed EXCEPT ![self] = TRUE]
               /\ pc' = [pc EXCEPT ![self] = "WReleaseQ"]
               /\ UNCHANGED << state, closed, qlock, destroyed, node, runnable, 
                               batch, head >>

WReleaseQ(self) == /\ pc[self] = "WReleaseQ"
                   /\ Assert(~destroyed, 
                             "Failure of assertion at line 90, column 5.")
                   /\ qlock' = FALSE
                   /\ IF ~pushed[self]
                         THEN /\ pc' = [pc EXCEPT ![self] = "Spin"]
                         ELSE /\ pc' = [pc EXCEPT ![self] = "ParkSwitch"]
                   /\ UNCHANGED << state, closed, queue, destroyed, node, 
                                   runnable, batch, head, pushed >>

ParkSwitch(self) == /\ pc[self] = "ParkSwitch"
                    /\ IF node[self] = "Woken"
                          THEN /\ node' = [node EXCEPT ![self] = "Parked"]
                               /\ UNCHANGED runnable
                          ELSE /\ node' = [node EXCEPT ![self] = "Parked"]
                               /\ runnable' = [runnable EXCEPT ![self] = FALSE]
                    /\ pc' = [pc EXCEPT ![self] = "Resume"]
                    /\ UNCHANGED << state, closed, qlock, queue, destroyed, 
                                    batch, head, pushed >>

Resume(self) == /\ pc[self] = "Resume"
                /\ runnable[self]
                /\ node' = [node EXCEPT ![self] = "Idle"]
                /\ pc' = [pc EXCEPT ![self] = "Finish"]
                /\ UNCHANGED << state, closed, qlock, queue, destroyed, 
                                runnable, batch, head, pushed >>

Spin(self) == /\ pc[self] = "Spin"
              /\ Assert(~destroyed, 
                        "Failure of assertion at line 107, column 5.")
              /\ IF state = 1
                    THEN /\ node' = [node EXCEPT ![self] = "Idle"]
                         /\ pc' = [pc EXCEPT ![self] = "Finish"]
                    ELSE /\ pc' = [pc EXCEPT ![self] = "Spin"]
                         /\ node' = node
              /\ UNCHANGED << state, closed, qlock, queue, destroyed, runnable, 
                              batch, head, pushed >>

Finish(self) == /\ pc[self] = "Finish"
                /\ \/ /\ ~destroyed /\ MayDestroy
                      /\ destroyed' = TRUE
                   \/ /\ TRUE
                      /\ UNCHANGED destroyed
                /\ pc' = [pc EXCEPT ![self] = "Done"]
                /\ UNCHANGED << state, closed, qlock, queue, node, runnable, 
                                batch, head, pushed >>

waiter(self) == WFast(self) \/ WArm(self) \/ WAcquireQ(self) \/ WPush(self)
                   \/ WReleaseQ(self) \/ ParkSwitch(self) \/ Resume(self)
                   \/ Spin(self) \/ Finish(self)

(* Allow infinite stuttering to prevent deadlock on termination. *)
Terminating == /\ \A self \in ProcSet: pc[self] = "Done"
               /\ UNCHANGED vars

Next == setter
           \/ (\E self \in Waiters: waiter(self))
           \/ Terminating

Spec == /\ Init /\ [][Next]_vars
        /\ WF_vars(setter)
        /\ \A self \in Waiters : WF_vars(waiter(self))

Termination == <>(\A self \in ProcSet: pc[self] = "Done")

\* END TRANSLATION 
=============================================================================
