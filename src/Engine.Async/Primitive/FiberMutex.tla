---------------------------- MODULE FiberMutex ----------------------------
(* Model of src/Engine.Async/Primitive/FiberMutex.cpp (lock/unlock slow path)
   plus the park/unpark handshake in src/Engine.Scheduler/Fiber/Park.cpp and
   Worker.cpp. The bounded spin in lock() is folded into one TryLock step.
   Plain-thread parking (threadFlag) is not modelled; it has one fewer state.
   Run: java -cp tla2tools.jar pcal.trans FiberMutex.tla
        java -cp tla2tools.jar tlc2.TLC -workers auto FiberMutex.tla      *)
EXTENDS Naturals, Sequences, FiniteSets, TLC

CONSTANT N
Fibers == 1..N
None == 0

(* --algorithm FiberMutex
variables
    locked = FALSE,         \* _word & eLocked
    contended = FALSE,      \* _word & eContended
    qlock = FALSE,          \* ParkQueue::_lck
    queue = << >>,          \* ParkQueue list, head first
    node = [f \in Fibers |-> "Idle"],       \* ParkNode::state
    runnable = [f \in Fibers |-> TRUE];     \* fiber is scheduled (not parked)

define
    Mutex == Cardinality({f \in Fibers : pc[f] = "CS"}) <= 1
    \* A waiter with eContended clear is only ever orphaned while an unlocker
    \* is between its exchange(0) and its pop; that unlocker will wake it.
    WakeupSafety ==
        (queue /= << >> /\ ~contended)
            => \E f \in Fibers : pc[f] \in {"UAcquireQ", "UPop"}
    NoLostWakeup ==
        \A f \in Fibers : (node[f] = "Parked" /\ ~runnable[f]) ~> runnable[f]
end define;

fair process fiber \in Fibers
variables w = [l |-> FALSE, c |-> FALSE], acquired = FALSE, pushed = FALSE, head = None;
begin
Loop:
    while TRUE do
    TryLock:                                    \* try_lock(): CAS on a free word
        if ~locked then
            locked := TRUE;
            goto CS;
        end if;
    AcquireQ:                                   \* _waiters.locked(...)
        await ~qlock;
        qlock := TRUE;
    Check:                                      \* w = _word.load(relaxed)
        w := [l |-> locked, c |-> contended];
    Cas:
        if ~w.l then                            \* CAS(w, w | eLocked)
            if locked = w.l /\ contended = w.c then
                locked := TRUE;
                acquired := TRUE;
            else
                acquired := FALSE;
            end if;
            pushed := FALSE;
        else                                    \* CAS(w, w | eContended), then arm + push
            if locked = w.l /\ contended = w.c then
                contended := TRUE;
                node[self] := "Armed";
                queue := Append(queue, self);
                pushed := TRUE;
            else
                pushed := FALSE;
            end if;
            acquired := FALSE;
        end if;
    ReleaseQ:
        qlock := FALSE;
        if acquired then
            goto CS;
        elsif ~pushed then
            goto TryLock;
        end if;
    ParkSwitch:                                 \* park(): yield; worker does state.exchange(eParked)
        if node[self] = "Woken" then
            node[self] := "Parked";             \* worker re-pushes the task; stays runnable
        else
            node[self] := "Parked";
            runnable[self] := FALSE;
        end if;
    Resume:
        await runnable[self];
        node[self] := "Idle";
        goto TryLock;
    CS:
        skip;
    Unlock:                                     \* prev = _word.exchange(0)
        w := [l |-> locked, c |-> contended];
        locked := FALSE;
        contended := FALSE;
        assert w.l;
        if ~w.c then
            goto Loop;
        end if;
    UAcquireQ:
        await ~qlock;
        qlock := TRUE;
    UPop:                                       \* popOne(); fetch_or(eContended) if more waiters
        if queue /= << >> then
            head := Head(queue);
            queue := Tail(queue);
        else
            head := None;
        end if;
        if queue /= << >> then
            contended := TRUE;
        end if;
    UReleaseQ:
        qlock := FALSE;
    Unpark:                                     \* unpark(node): state.exchange(eWoken)
        if head /= None then
            assert node[head] \in {"Armed", "Parked"};
            if node[head] = "Parked" then
                runnable[head] := TRUE;
            end if;
            node[head] := "Woken";
            head := None;
        end if;
    end while;
end process;
end algorithm; *)
\* BEGIN TRANSLATION (chksum(pcal) = "11c0e63f" /\ chksum(tla) = "c5c9d0d")
VARIABLES locked, contended, qlock, queue, node, runnable, pc

(* define statement *)
Mutex == Cardinality({f \in Fibers : pc[f] = "CS"}) <= 1


WakeupSafety ==
    (queue /= << >> /\ ~contended)
        => \E f \in Fibers : pc[f] \in {"UAcquireQ", "UPop"}
NoLostWakeup ==
    \A f \in Fibers : (node[f] = "Parked" /\ ~runnable[f]) ~> runnable[f]

VARIABLES w, acquired, pushed, head

vars == << locked, contended, qlock, queue, node, runnable, pc, w, acquired, 
           pushed, head >>

ProcSet == (Fibers)

Init == (* Global variables *)
        /\ locked = FALSE
        /\ contended = FALSE
        /\ qlock = FALSE
        /\ queue = << >>
        /\ node = [f \in Fibers |-> "Idle"]
        /\ runnable = [f \in Fibers |-> TRUE]
        (* Process fiber *)
        /\ w = [self \in Fibers |-> [l |-> FALSE, c |-> FALSE]]
        /\ acquired = [self \in Fibers |-> FALSE]
        /\ pushed = [self \in Fibers |-> FALSE]
        /\ head = [self \in Fibers |-> None]
        /\ pc = [self \in ProcSet |-> "Loop"]

Loop(self) == /\ pc[self] = "Loop"
              /\ pc' = [pc EXCEPT ![self] = "TryLock"]
              /\ UNCHANGED << locked, contended, qlock, queue, node, runnable, 
                              w, acquired, pushed, head >>

TryLock(self) == /\ pc[self] = "TryLock"
                 /\ IF ~locked
                       THEN /\ locked' = TRUE
                            /\ pc' = [pc EXCEPT ![self] = "CS"]
                       ELSE /\ pc' = [pc EXCEPT ![self] = "AcquireQ"]
                            /\ UNCHANGED locked
                 /\ UNCHANGED << contended, qlock, queue, node, runnable, w, 
                                 acquired, pushed, head >>

AcquireQ(self) == /\ pc[self] = "AcquireQ"
                  /\ ~qlock
                  /\ qlock' = TRUE
                  /\ pc' = [pc EXCEPT ![self] = "Check"]
                  /\ UNCHANGED << locked, contended, queue, node, runnable, w, 
                                  acquired, pushed, head >>

Check(self) == /\ pc[self] = "Check"
               /\ w' = [w EXCEPT ![self] = [l |-> locked, c |-> contended]]
               /\ pc' = [pc EXCEPT ![self] = "Cas"]
               /\ UNCHANGED << locked, contended, qlock, queue, node, runnable, 
                               acquired, pushed, head >>

Cas(self) == /\ pc[self] = "Cas"
             /\ IF ~w[self].l
                   THEN /\ IF locked = w[self].l /\ contended = w[self].c
                              THEN /\ locked' = TRUE
                                   /\ acquired' = [acquired EXCEPT ![self] = TRUE]
                              ELSE /\ acquired' = [acquired EXCEPT ![self] = FALSE]
                                   /\ UNCHANGED locked
                        /\ pushed' = [pushed EXCEPT ![self] = FALSE]
                        /\ UNCHANGED << contended, queue, node >>
                   ELSE /\ IF locked = w[self].l /\ contended = w[self].c
                              THEN /\ contended' = TRUE
                                   /\ node' = [node EXCEPT ![self] = "Armed"]
                                   /\ queue' = Append(queue, self)
                                   /\ pushed' = [pushed EXCEPT ![self] = TRUE]
                              ELSE /\ pushed' = [pushed EXCEPT ![self] = FALSE]
                                   /\ UNCHANGED << contended, queue, node >>
                        /\ acquired' = [acquired EXCEPT ![self] = FALSE]
                        /\ UNCHANGED locked
             /\ pc' = [pc EXCEPT ![self] = "ReleaseQ"]
             /\ UNCHANGED << qlock, runnable, w, head >>

ReleaseQ(self) == /\ pc[self] = "ReleaseQ"
                  /\ qlock' = FALSE
                  /\ IF acquired[self]
                        THEN /\ pc' = [pc EXCEPT ![self] = "CS"]
                        ELSE /\ IF ~pushed[self]
                                   THEN /\ pc' = [pc EXCEPT ![self] = "TryLock"]
                                   ELSE /\ pc' = [pc EXCEPT ![self] = "ParkSwitch"]
                  /\ UNCHANGED << locked, contended, queue, node, runnable, w, 
                                  acquired, pushed, head >>

ParkSwitch(self) == /\ pc[self] = "ParkSwitch"
                    /\ IF node[self] = "Woken"
                          THEN /\ node' = [node EXCEPT ![self] = "Parked"]
                               /\ UNCHANGED runnable
                          ELSE /\ node' = [node EXCEPT ![self] = "Parked"]
                               /\ runnable' = [runnable EXCEPT ![self] = FALSE]
                    /\ pc' = [pc EXCEPT ![self] = "Resume"]
                    /\ UNCHANGED << locked, contended, qlock, queue, w, 
                                    acquired, pushed, head >>

Resume(self) == /\ pc[self] = "Resume"
                /\ runnable[self]
                /\ node' = [node EXCEPT ![self] = "Idle"]
                /\ pc' = [pc EXCEPT ![self] = "TryLock"]
                /\ UNCHANGED << locked, contended, qlock, queue, runnable, w, 
                                acquired, pushed, head >>

CS(self) == /\ pc[self] = "CS"
            /\ TRUE
            /\ pc' = [pc EXCEPT ![self] = "Unlock"]
            /\ UNCHANGED << locked, contended, qlock, queue, node, runnable, w, 
                            acquired, pushed, head >>

Unlock(self) == /\ pc[self] = "Unlock"
                /\ w' = [w EXCEPT ![self] = [l |-> locked, c |-> contended]]
                /\ locked' = FALSE
                /\ contended' = FALSE
                /\ Assert(w'[self].l, 
                          "Failure of assertion at line 93, column 9.")
                /\ IF ~w'[self].c
                      THEN /\ pc' = [pc EXCEPT ![self] = "Loop"]
                      ELSE /\ pc' = [pc EXCEPT ![self] = "UAcquireQ"]
                /\ UNCHANGED << qlock, queue, node, runnable, acquired, pushed, 
                                head >>

UAcquireQ(self) == /\ pc[self] = "UAcquireQ"
                   /\ ~qlock
                   /\ qlock' = TRUE
                   /\ pc' = [pc EXCEPT ![self] = "UPop"]
                   /\ UNCHANGED << locked, contended, queue, node, runnable, w, 
                                   acquired, pushed, head >>

UPop(self) == /\ pc[self] = "UPop"
              /\ IF queue /= << >>
                    THEN /\ head' = [head EXCEPT ![self] = Head(queue)]
                         /\ queue' = Tail(queue)
                    ELSE /\ head' = [head EXCEPT ![self] = None]
                         /\ queue' = queue
              /\ IF queue' /= << >>
                    THEN /\ contended' = TRUE
                    ELSE /\ TRUE
                         /\ UNCHANGED contended
              /\ pc' = [pc EXCEPT ![self] = "UReleaseQ"]
              /\ UNCHANGED << locked, qlock, node, runnable, w, acquired, 
                              pushed >>

UReleaseQ(self) == /\ pc[self] = "UReleaseQ"
                   /\ qlock' = FALSE
                   /\ pc' = [pc EXCEPT ![self] = "Unpark"]
                   /\ UNCHANGED << locked, contended, queue, node, runnable, w, 
                                   acquired, pushed, head >>

Unpark(self) == /\ pc[self] = "Unpark"
                /\ IF head[self] /= None
                      THEN /\ Assert(node[head[self]] \in {"Armed", "Parked"}, 
                                     "Failure of assertion at line 114, column 13.")
                           /\ IF node[head[self]] = "Parked"
                                 THEN /\ runnable' = [runnable EXCEPT ![head[self]] = TRUE]
                                 ELSE /\ TRUE
                                      /\ UNCHANGED runnable
                           /\ node' = [node EXCEPT ![head[self]] = "Woken"]
                           /\ head' = [head EXCEPT ![self] = None]
                      ELSE /\ TRUE
                           /\ UNCHANGED << node, runnable, head >>
                /\ pc' = [pc EXCEPT ![self] = "Loop"]
                /\ UNCHANGED << locked, contended, qlock, queue, w, acquired, 
                                pushed >>

fiber(self) == Loop(self) \/ TryLock(self) \/ AcquireQ(self) \/ Check(self)
                  \/ Cas(self) \/ ReleaseQ(self) \/ ParkSwitch(self)
                  \/ Resume(self) \/ CS(self) \/ Unlock(self)
                  \/ UAcquireQ(self) \/ UPop(self) \/ UReleaseQ(self)
                  \/ Unpark(self)

Next == (\E self \in Fibers: fiber(self))

Spec == /\ Init /\ [][Next]_vars
        /\ \A self \in Fibers : WF_vars(fiber(self))

\* END TRANSLATION 
=============================================================================
