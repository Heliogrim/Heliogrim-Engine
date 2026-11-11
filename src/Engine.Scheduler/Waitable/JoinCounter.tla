---------------------------- MODULE JoinCounter ----------------------------
(* Model of src/Engine.Scheduler/Waitable/JoinCounter.cpp over the Signal in
   Signal.cpp. K children call done(); the creator calls wait(), which drops
   its own guard and then waits on the signal. Whoever brings _pending to 0
   runs Signal::set(). add() is not modelled (contract: before children start).
   Checks: set() runs exactly once, only after every child's done(), and the
   creator's wait() returns.                                                 *)
EXTENDS Naturals, Sequences, FiniteSets, TLC

CONSTANT K
Children == 1..K
Creator == 0
None == 0
Procs == Children \cup {Creator}

(* --algorithm JoinCounter
variables
    pending = K + 1,        \* _pending (children + creator guard)
    state = 0,              \* _done._state
    closed = FALSE,
    qlock = FALSE,
    queue = << >>,
    setCount = 0,           \* times Signal::set() ran
    childDone = [c \in Children |-> FALSE],
    node = [p \in Procs |-> "Idle"],
    runnable = [p \in Procs |-> TRUE];

define
    SetOnce == setCount <= 1
    SetAfterAllChildren == state = 1 => \A c \in Children : childDone[c]
    CreatorReturns == <>(pc[Creator] = "Done")
    NoLostWakeup ==
        (node[Creator] = "Parked" /\ ~runnable[Creator]) ~> runnable[Creator]
end define;

\* done(): fetch_sub(1); prev == 1 runs set().
procedure done()
variables prev = 0, batch = << >>, head = None;
begin
DSub:
    prev := pending;
    pending := pending - 1;
    assert prev /= 0;
    if prev /= 1 then
        goto DRet;
    end if;
SAssert:
    assert state = 0;
    setCount := setCount + 1;
SAcquireQ:
    await ~qlock;
    qlock := TRUE;
STakeAll:
    closed := TRUE;
    batch := queue;
    queue := << >>;
SReleaseQ:
    qlock := FALSE;
SStore:
    state := 1;
SUnpark:
    while batch /= << >> do
        head := Head(batch);
        batch := Tail(batch);
        assert node[head] \in {"Armed", "Parked"};
        if node[head] = "Parked" then
            runnable[head] := TRUE;
        end if;
        node[head] := "Woken";
    end while;
DRet:
    return;
end procedure;

fair process child \in Children
begin
CDone:
    childDone[self] := TRUE;
    call done();
end process;

fair process creator = Creator
variables pushed = FALSE;
begin
CrDone:                                     \* wait(): drop the creator guard
    call done();
WFast:                                      \* _done.wait()
    if state = 1 then
        goto Done;
    end if;
WArm:
    node[Creator] := "Armed";
WAcquireQ:
    await ~qlock;
    qlock := TRUE;
WPush:
    if closed then
        pushed := FALSE;
    else
        queue := Append(queue, Creator);
        pushed := TRUE;
    end if;
WReleaseQ:
    qlock := FALSE;
    if ~pushed then
        goto Spin;
    end if;
ParkSwitch:
    if node[Creator] = "Woken" then
        node[Creator] := "Parked";
    else
        node[Creator] := "Parked";
        runnable[Creator] := FALSE;
    end if;
Resume:
    await runnable[Creator];
    node[Creator] := "Idle";
    goto Done;
Spin:
    if state = 1 then
        node[Creator] := "Idle";
        goto Done;
    else
        goto Spin;
    end if;
end process;
end algorithm; *)
\* BEGIN TRANSLATION (chksum(pcal) = "586dde83" /\ chksum(tla) = "9e35558c")
VARIABLES pending, state, closed, qlock, queue, setCount, childDone, node, 
          runnable, pc, stack

(* define statement *)
SetOnce == setCount <= 1
SetAfterAllChildren == state = 1 => \A c \in Children : childDone[c]
CreatorReturns == <>(pc[Creator] = "Done")
NoLostWakeup ==
    (node[Creator] = "Parked" /\ ~runnable[Creator]) ~> runnable[Creator]

VARIABLES prev, batch, head, pushed

vars == << pending, state, closed, qlock, queue, setCount, childDone, node, 
           runnable, pc, stack, prev, batch, head, pushed >>

ProcSet == (Children) \cup {Creator}

Init == (* Global variables *)
        /\ pending = K + 1
        /\ state = 0
        /\ closed = FALSE
        /\ qlock = FALSE
        /\ queue = << >>
        /\ setCount = 0
        /\ childDone = [c \in Children |-> FALSE]
        /\ node = [p \in Procs |-> "Idle"]
        /\ runnable = [p \in Procs |-> TRUE]
        (* Procedure done *)
        /\ prev = [ self \in ProcSet |-> 0]
        /\ batch = [ self \in ProcSet |-> << >>]
        /\ head = [ self \in ProcSet |-> None]
        (* Process creator *)
        /\ pushed = FALSE
        /\ stack = [self \in ProcSet |-> << >>]
        /\ pc = [self \in ProcSet |-> CASE self \in Children -> "CDone"
                                        [] self = Creator -> "CrDone"]

DSub(self) == /\ pc[self] = "DSub"
              /\ prev' = [prev EXCEPT ![self] = pending]
              /\ pending' = pending - 1
              /\ Assert(prev'[self] /= 0, 
                        "Failure of assertion at line 43, column 5.")
              /\ IF prev'[self] /= 1
                    THEN /\ pc' = [pc EXCEPT ![self] = "DRet"]
                    ELSE /\ pc' = [pc EXCEPT ![self] = "SAssert"]
              /\ UNCHANGED << state, closed, qlock, queue, setCount, childDone, 
                              node, runnable, stack, batch, head, pushed >>

SAssert(self) == /\ pc[self] = "SAssert"
                 /\ Assert(state = 0, 
                           "Failure of assertion at line 48, column 5.")
                 /\ setCount' = setCount + 1
                 /\ pc' = [pc EXCEPT ![self] = "SAcquireQ"]
                 /\ UNCHANGED << pending, state, closed, qlock, queue, 
                                 childDone, node, runnable, stack, prev, batch, 
                                 head, pushed >>

SAcquireQ(self) == /\ pc[self] = "SAcquireQ"
                   /\ ~qlock
                   /\ qlock' = TRUE
                   /\ pc' = [pc EXCEPT ![self] = "STakeAll"]
                   /\ UNCHANGED << pending, state, closed, queue, setCount, 
                                   childDone, node, runnable, stack, prev, 
                                   batch, head, pushed >>

STakeAll(self) == /\ pc[self] = "STakeAll"
                  /\ closed' = TRUE
                  /\ batch' = [batch EXCEPT ![self] = queue]
                  /\ queue' = << >>
                  /\ pc' = [pc EXCEPT ![self] = "SReleaseQ"]
                  /\ UNCHANGED << pending, state, qlock, setCount, childDone, 
                                  node, runnable, stack, prev, head, pushed >>

SReleaseQ(self) == /\ pc[self] = "SReleaseQ"
                   /\ qlock' = FALSE
                   /\ pc' = [pc EXCEPT ![self] = "SStore"]
                   /\ UNCHANGED << pending, state, closed, queue, setCount, 
                                   childDone, node, runnable, stack, prev, 
                                   batch, head, pushed >>

SStore(self) == /\ pc[self] = "SStore"
                /\ state' = 1
                /\ pc' = [pc EXCEPT ![self] = "SUnpark"]
                /\ UNCHANGED << pending, closed, qlock, queue, setCount, 
                                childDone, node, runnable, stack, prev, batch, 
                                head, pushed >>

SUnpark(self) == /\ pc[self] = "SUnpark"
                 /\ IF batch[self] /= << >>
                       THEN /\ head' = [head EXCEPT ![self] = Head(batch[self])]
                            /\ batch' = [batch EXCEPT ![self] = Tail(batch[self])]
                            /\ Assert(node[head'[self]] \in {"Armed", "Parked"}, 
                                      "Failure of assertion at line 65, column 9.")
                            /\ IF node[head'[self]] = "Parked"
                                  THEN /\ runnable' = [runnable EXCEPT ![head'[self]] = TRUE]
                                  ELSE /\ TRUE
                                       /\ UNCHANGED runnable
                            /\ node' = [node EXCEPT ![head'[self]] = "Woken"]
                            /\ pc' = [pc EXCEPT ![self] = "SUnpark"]
                       ELSE /\ pc' = [pc EXCEPT ![self] = "DRet"]
                            /\ UNCHANGED << node, runnable, batch, head >>
                 /\ UNCHANGED << pending, state, closed, qlock, queue, 
                                 setCount, childDone, stack, prev, pushed >>

DRet(self) == /\ pc[self] = "DRet"
              /\ pc' = [pc EXCEPT ![self] = Head(stack[self]).pc]
              /\ prev' = [prev EXCEPT ![self] = Head(stack[self]).prev]
              /\ batch' = [batch EXCEPT ![self] = Head(stack[self]).batch]
              /\ head' = [head EXCEPT ![self] = Head(stack[self]).head]
              /\ stack' = [stack EXCEPT ![self] = Tail(stack[self])]
              /\ UNCHANGED << pending, state, closed, qlock, queue, setCount, 
                              childDone, node, runnable, pushed >>

done(self) == DSub(self) \/ SAssert(self) \/ SAcquireQ(self)
                 \/ STakeAll(self) \/ SReleaseQ(self) \/ SStore(self)
                 \/ SUnpark(self) \/ DRet(self)

CDone(self) == /\ pc[self] = "CDone"
               /\ childDone' = [childDone EXCEPT ![self] = TRUE]
               /\ stack' = [stack EXCEPT ![self] = << [ procedure |->  "done",
                                                        pc        |->  "Done",
                                                        prev      |->  prev[self],
                                                        batch     |->  batch[self],
                                                        head      |->  head[self] ] >>
                                                    \o stack[self]]
               /\ prev' = [prev EXCEPT ![self] = 0]
               /\ batch' = [batch EXCEPT ![self] = << >>]
               /\ head' = [head EXCEPT ![self] = None]
               /\ pc' = [pc EXCEPT ![self] = "DSub"]
               /\ UNCHANGED << pending, state, closed, qlock, queue, setCount, 
                               node, runnable, pushed >>

child(self) == CDone(self)

CrDone == /\ pc[Creator] = "CrDone"
          /\ stack' = [stack EXCEPT ![Creator] = << [ procedure |->  "done",
                                                      pc        |->  "WFast",
                                                      prev      |->  prev[Creator],
                                                      batch     |->  batch[Creator],
                                                      head      |->  head[Creator] ] >>
                                                  \o stack[Creator]]
          /\ prev' = [prev EXCEPT ![Creator] = 0]
          /\ batch' = [batch EXCEPT ![Creator] = << >>]
          /\ head' = [head EXCEPT ![Creator] = None]
          /\ pc' = [pc EXCEPT ![Creator] = "DSub"]
          /\ UNCHANGED << pending, state, closed, qlock, queue, setCount, 
                          childDone, node, runnable, pushed >>

WFast == /\ pc[Creator] = "WFast"
         /\ IF state = 1
               THEN /\ pc' = [pc EXCEPT ![Creator] = "Done"]
               ELSE /\ pc' = [pc EXCEPT ![Creator] = "WArm"]
         /\ UNCHANGED << pending, state, closed, qlock, queue, setCount, 
                         childDone, node, runnable, stack, prev, batch, head, 
                         pushed >>

WArm == /\ pc[Creator] = "WArm"
        /\ node' = [node EXCEPT ![Creator] = "Armed"]
        /\ pc' = [pc EXCEPT ![Creator] = "WAcquireQ"]
        /\ UNCHANGED << pending, state, closed, qlock, queue, setCount, 
                        childDone, runnable, stack, prev, batch, head, pushed >>

WAcquireQ == /\ pc[Creator] = "WAcquireQ"
             /\ ~qlock
             /\ qlock' = TRUE
             /\ pc' = [pc EXCEPT ![Creator] = "WPush"]
             /\ UNCHANGED << pending, state, closed, queue, setCount, 
                             childDone, node, runnable, stack, prev, batch, 
                             head, pushed >>

WPush == /\ pc[Creator] = "WPush"
         /\ IF closed
               THEN /\ pushed' = FALSE
                    /\ queue' = queue
               ELSE /\ queue' = Append(queue, Creator)
                    /\ pushed' = TRUE
         /\ pc' = [pc EXCEPT ![Creator] = "WReleaseQ"]
         /\ UNCHANGED << pending, state, closed, qlock, setCount, childDone, 
                         node, runnable, stack, prev, batch, head >>

WReleaseQ == /\ pc[Creator] = "WReleaseQ"
             /\ qlock' = FALSE
             /\ IF ~pushed
                   THEN /\ pc' = [pc EXCEPT ![Creator] = "Spin"]
                   ELSE /\ pc' = [pc EXCEPT ![Creator] = "ParkSwitch"]
             /\ UNCHANGED << pending, state, closed, queue, setCount, 
                             childDone, node, runnable, stack, prev, batch, 
                             head, pushed >>

ParkSwitch == /\ pc[Creator] = "ParkSwitch"
              /\ IF node[Creator] = "Woken"
                    THEN /\ node' = [node EXCEPT ![Creator] = "Parked"]
                         /\ UNCHANGED runnable
                    ELSE /\ node' = [node EXCEPT ![Creator] = "Parked"]
                         /\ runnable' = [runnable EXCEPT ![Creator] = FALSE]
              /\ pc' = [pc EXCEPT ![Creator] = "Resume"]
              /\ UNCHANGED << pending, state, closed, qlock, queue, setCount, 
                              childDone, stack, prev, batch, head, pushed >>

Resume == /\ pc[Creator] = "Resume"
          /\ runnable[Creator]
          /\ node' = [node EXCEPT ![Creator] = "Idle"]
          /\ pc' = [pc EXCEPT ![Creator] = "Done"]
          /\ UNCHANGED << pending, state, closed, qlock, queue, setCount, 
                          childDone, runnable, stack, prev, batch, head, 
                          pushed >>

Spin == /\ pc[Creator] = "Spin"
        /\ IF state = 1
              THEN /\ node' = [node EXCEPT ![Creator] = "Idle"]
                   /\ pc' = [pc EXCEPT ![Creator] = "Done"]
              ELSE /\ pc' = [pc EXCEPT ![Creator] = "Spin"]
                   /\ node' = node
        /\ UNCHANGED << pending, state, closed, qlock, queue, setCount, 
                        childDone, runnable, stack, prev, batch, head, pushed >>

creator == CrDone \/ WFast \/ WArm \/ WAcquireQ \/ WPush \/ WReleaseQ
              \/ ParkSwitch \/ Resume \/ Spin

(* Allow infinite stuttering to prevent deadlock on termination. *)
Terminating == /\ \A self \in ProcSet: pc[self] = "Done"
               /\ UNCHANGED vars

Next == creator
           \/ (\E self \in ProcSet: done(self))
           \/ (\E self \in Children: child(self))
           \/ Terminating

Spec == /\ Init /\ [][Next]_vars
        /\ \A self \in Children : WF_vars(child(self)) /\ WF_vars(done(self))
        /\ WF_vars(creator) /\ WF_vars(done(Creator))

Termination == <>(\A self \in ProcSet: pc[self] = "Done")

\* END TRANSLATION 
=============================================================================
