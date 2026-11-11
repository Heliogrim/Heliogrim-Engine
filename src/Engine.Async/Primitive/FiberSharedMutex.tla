-------------------------- MODULE FiberSharedMutex --------------------------
(* Model of src/Engine.Async/Primitive/FiberSharedMutex.cpp plus the
   park/unpark handshake (Park.cpp, Worker.cpp). N agents loop, each time
   choosing to take the lock exclusively or shared. The two ParkQueues are
   always locked together (lockBoth), so one qlock models both. _word is the
   record [wr, ww, rd] for eWriter, eWriterWaiting and the reader count.
   Bounded spins are folded into one step. Ownership handoff is checked:
   a woken waiter enters its critical section without touching _word.      *)
EXTENDS Naturals, Sequences, FiniteSets, TLC

CONSTANTS Agents, None  \* model values; Agents is symmetric, see the .cfg
Free == [wr |-> FALSE, ww |-> FALSE, rd |-> 0]
Sym == Permutations(Agents)

(* --algorithm FiberSharedMutex
variables
    word = Free,
    qlock = FALSE,
    readersLast = FALSE,                    \* which queue unlock() served last; guarded by qlock
    readersQ = << >>,
    writersQ = << >>,
    wholder = None,                         \* agent in the exclusive section
    holders = {},                           \* agents in a shared section
    node = [a \in Agents |-> "Idle"],
    runnable = [a \in Agents |-> TRUE];

define
    Exclusion == wholder /= None => holders = {}
    WordWriter == wholder /= None => word.wr
    WordReaders == holders /= {} => (~word.wr /\ word.rd >= Cardinality(holders))
    WaitingFlag == writersQ /= << >> => word.ww
    QueuedReaders == readersQ /= << >> => (word.wr \/ word.ww)
    \* A parked agent is never orphaned: while it is parked some other agent is
    \* still inside lock/unlock and will reach the wake path.
    NoOrphanedWaiter ==
        \A a \in Agents :
            (node[a] = "Parked" /\ ~runnable[a])
                => \E o \in Agents \ {a} : pc[o] /= "Loop"
    \* Every parked agent is eventually woken. This holds only because the slow
    \* path parks behind queued writers and unlock() alternates between the two
    \* queues; the earlier barging version starved a second writer (TLC cycle).
    NoLostWakeup ==
        \A a \in Agents : (node[a] = "Parked" /\ ~runnable[a]) ~> runnable[a]
end define;

macro unpark(h) begin
    assert node[h] \in {"Armed", "Parked"};
    if node[h] = "Parked" then
        runnable[h] := TRUE;
    end if;
    node[h] := "Woken";
end macro;

\* unlock_shared(): also used by lock_shared() to drop its transient count.
procedure unlockShared()
variables p = Free, s = Free, h = None;
begin
USub:                                       \* prev = fetch_sub(1)
    p := word;
    word.rd := word.rd - 1;
    holders := holders \ {self};
    assert p.rd /= 0;
    if p.rd /= 1 \/ ~p.ww then
        goto URet;
    end if;
UAcquireQ:
    await ~qlock;
    qlock := TRUE;
UCheck:                                     \* re-check under the lock, then hand off
    s := word;
    if s.wr \/ s.rd /= 0 \/ writersQ = << >> then
        h := None;
    else
        h := Head(writersQ);
        if Len(writersQ) = 1 then
            word := [word EXCEPT !.wr = TRUE, !.ww = FALSE];
        else
            word.wr := TRUE;
        end if;
        writersQ := Tail(writersQ);
    end if;
UReleaseQ:
    qlock := FALSE;
UWake:
    if h /= None then
        unpark(h);
        h := None;
    end if;
URet:
    return;
end procedure;

fair process agent \in Agents
variables role = "W", w = Free, prev = Free, pushed = FALSE, head = None, batch = << >>;
begin
Loop:
    while TRUE do
        either
            role := "W";
            goto WTry;
        or
            role := "R";
            goto RFetchAdd;
        end either;

    \* ---- lock() ----
    WTry:                                   \* try_lock(): CAS 0 -> eWriter
        if word = Free then
            word.wr := TRUE;
            goto WCS;
        end if;
    WAcquireQ:
        await ~qlock;
        qlock := TRUE;
    WFetchOr:                               \* w = fetch_or(eWriterWaiting) | eWriterWaiting
        w := [word EXCEPT !.ww = TRUE];
        word := w;
    WCas:                                   \* while queue empty, no writer, no readers: CAS to eWriter
        if writersQ = << >> /\ ~w.wr /\ w.rd = 0 then
            if word = w then
                word := [wr |-> TRUE, ww |-> FALSE, rd |-> 0];
                pushed := FALSE;
                goto WReleaseQ;
            else
                w := word;
                goto WCas;
            end if;
        end if;
    WPush:
        node[self] := "Armed";
        writersQ := Append(writersQ, self);
        pushed := TRUE;
    WReleaseQ:
        qlock := FALSE;
        if pushed then
            goto Park;
        end if;
    WCS:
        assert wholder = None /\ holders = {};
        wholder := self;
        w := Free; pushed := FALSE;         \* dead locals; stale values only multiply states

    \* ---- unlock() ----
    WUnlock:
        await ~qlock;
        qlock := TRUE;
    WDrain:
        wholder := None;
        if readersQ /= << >> /\ (writersQ = << >> \/ ~readersLast) then
            batch := readersQ;              \* fetch_sub(eWriter - n): admit the batch
            readersQ := << >>;
            readersLast := TRUE;
            word := [word EXCEPT !.wr = FALSE, !.rd = @ + Len(batch)];
        elsif writersQ /= << >> then        \* hand eWriter to the next writer
            readersLast := FALSE;
            head := Head(writersQ);
            if Len(writersQ) = 1 then
                word.ww := FALSE;
            end if;
            writersQ := Tail(writersQ);
        else
            word.wr := FALSE;
        end if;
    WReleaseQ2:
        qlock := FALSE;
    WWake:
        while batch /= << >> do
            unpark(Head(batch));
            batch := Tail(batch);
        end while;
        if head /= None then
            unpark(head);
            head := None;
        end if;
        role := "W";
        goto Loop;

    \* ---- lock_shared() ----
    RFetchAdd:                              \* prev = fetch_add(1)
        prev := word;
        word.rd := word.rd + 1;
        if ~prev.wr /\ ~prev.ww then
            goto RCS;
        end if;
    RUndo:                                  \* unlock_shared() on the transient count
        call unlockShared();
    RAcquireQ:
        await ~qlock;
        qlock := TRUE;
    RTry:                                   \* try_lock_shared() under the queue lock
        if ~word.wr /\ ~word.ww then
            word.rd := word.rd + 1;
            pushed := FALSE;
        else
            node[self] := "Armed";
            readersQ := Append(readersQ, self);
            pushed := TRUE;
        end if;
    RReleaseQ:
        qlock := FALSE;
        if pushed then
            goto Park;
        end if;
    RCS:
        assert wholder = None;
        holders := holders \cup {self};
        prev := Free; pushed := FALSE;      \* dead locals; stale values only multiply states
    RUnlockShared:
        role := "W";
        call unlockShared();
        goto Loop;

    \* ---- park(): shared by both paths; the waker already set our ownership ----
    Park:
        if node[self] = "Woken" then
            node[self] := "Parked";
        else
            node[self] := "Parked";
            runnable[self] := FALSE;
        end if;
    Resume:
        await runnable[self];
        node[self] := "Idle";
        if role = "W" then
            goto WCS;
        else
            goto RCS;
        end if;
    end while;
end process;
end algorithm; *)
\* BEGIN TRANSLATION (chksum(pcal) = "fc19ea00" /\ chksum(tla) = "9fd6e18")
VARIABLES word, qlock, readersLast, readersQ, writersQ, wholder, holders, 
          node, runnable, pc, stack

(* define statement *)
Exclusion == wholder /= None => holders = {}
WordWriter == wholder /= None => word.wr
WordReaders == holders /= {} => (~word.wr /\ word.rd >= Cardinality(holders))
WaitingFlag == writersQ /= << >> => word.ww
QueuedReaders == readersQ /= << >> => (word.wr \/ word.ww)


NoOrphanedWaiter ==
    \A a \in Agents :
        (node[a] = "Parked" /\ ~runnable[a])
            => \E o \in Agents \ {a} : pc[o] /= "Loop"



NoLostWakeup ==
    \A a \in Agents : (node[a] = "Parked" /\ ~runnable[a]) ~> runnable[a]

VARIABLES p, s, h, role, w, prev, pushed, head, batch

vars == << word, qlock, readersLast, readersQ, writersQ, wholder, holders, 
           node, runnable, pc, stack, p, s, h, role, w, prev, pushed, head, 
           batch >>

ProcSet == (Agents)

Init == (* Global variables *)
        /\ word = Free
        /\ qlock = FALSE
        /\ readersLast = FALSE
        /\ readersQ = << >>
        /\ writersQ = << >>
        /\ wholder = None
        /\ holders = {}
        /\ node = [a \in Agents |-> "Idle"]
        /\ runnable = [a \in Agents |-> TRUE]
        (* Procedure unlockShared *)
        /\ p = [ self \in ProcSet |-> Free]
        /\ s = [ self \in ProcSet |-> Free]
        /\ h = [ self \in ProcSet |-> None]
        (* Process agent *)
        /\ role = [self \in Agents |-> "W"]
        /\ w = [self \in Agents |-> Free]
        /\ prev = [self \in Agents |-> Free]
        /\ pushed = [self \in Agents |-> FALSE]
        /\ head = [self \in Agents |-> None]
        /\ batch = [self \in Agents |-> << >>]
        /\ stack = [self \in ProcSet |-> << >>]
        /\ pc = [self \in ProcSet |-> "Loop"]

USub(self) == /\ pc[self] = "USub"
              /\ p' = [p EXCEPT ![self] = word]
              /\ word' = [word EXCEPT !.rd = word.rd - 1]
              /\ holders' = holders \ {self}
              /\ Assert(p'[self].rd /= 0, 
                        "Failure of assertion at line 62, column 5.")
              /\ IF p'[self].rd /= 1 \/ ~p'[self].ww
                    THEN /\ pc' = [pc EXCEPT ![self] = "URet"]
                    ELSE /\ pc' = [pc EXCEPT ![self] = "UAcquireQ"]
              /\ UNCHANGED << qlock, readersLast, readersQ, writersQ, wholder, 
                              node, runnable, stack, s, h, role, w, prev, 
                              pushed, head, batch >>

UAcquireQ(self) == /\ pc[self] = "UAcquireQ"
                   /\ ~qlock
                   /\ qlock' = TRUE
                   /\ pc' = [pc EXCEPT ![self] = "UCheck"]
                   /\ UNCHANGED << word, readersLast, readersQ, writersQ, 
                                   wholder, holders, node, runnable, stack, p, 
                                   s, h, role, w, prev, pushed, head, batch >>

UCheck(self) == /\ pc[self] = "UCheck"
                /\ s' = [s EXCEPT ![self] = word]
                /\ IF s'[self].wr \/ s'[self].rd /= 0 \/ writersQ = << >>
                      THEN /\ h' = [h EXCEPT ![self] = None]
                           /\ UNCHANGED << word, writersQ >>
                      ELSE /\ h' = [h EXCEPT ![self] = Head(writersQ)]
                           /\ IF Len(writersQ) = 1
                                 THEN /\ word' = [word EXCEPT !.wr = TRUE, !.ww = FALSE]
                                 ELSE /\ word' = [word EXCEPT !.wr = TRUE]
                           /\ writersQ' = Tail(writersQ)
                /\ pc' = [pc EXCEPT ![self] = "UReleaseQ"]
                /\ UNCHANGED << qlock, readersLast, readersQ, wholder, holders, 
                                node, runnable, stack, p, role, w, prev, 
                                pushed, head, batch >>

UReleaseQ(self) == /\ pc[self] = "UReleaseQ"
                   /\ qlock' = FALSE
                   /\ pc' = [pc EXCEPT ![self] = "UWake"]
                   /\ UNCHANGED << word, readersLast, readersQ, writersQ, 
                                   wholder, holders, node, runnable, stack, p, 
                                   s, h, role, w, prev, pushed, head, batch >>

UWake(self) == /\ pc[self] = "UWake"
               /\ IF h[self] /= None
                     THEN /\ Assert(node[h[self]] \in {"Armed", "Parked"}, 
                                    "Failure of assertion at line 47, column 5 of macro called at line 86, column 9.")
                          /\ IF node[h[self]] = "Parked"
                                THEN /\ runnable' = [runnable EXCEPT ![h[self]] = TRUE]
                                ELSE /\ TRUE
                                     /\ UNCHANGED runnable
                          /\ node' = [node EXCEPT ![h[self]] = "Woken"]
                          /\ h' = [h EXCEPT ![self] = None]
                     ELSE /\ TRUE
                          /\ UNCHANGED << node, runnable, h >>
               /\ pc' = [pc EXCEPT ![self] = "URet"]
               /\ UNCHANGED << word, qlock, readersLast, readersQ, writersQ, 
                               wholder, holders, stack, p, s, role, w, prev, 
                               pushed, head, batch >>

URet(self) == /\ pc[self] = "URet"
              /\ pc' = [pc EXCEPT ![self] = Head(stack[self]).pc]
              /\ p' = [p EXCEPT ![self] = Head(stack[self]).p]
              /\ s' = [s EXCEPT ![self] = Head(stack[self]).s]
              /\ h' = [h EXCEPT ![self] = Head(stack[self]).h]
              /\ stack' = [stack EXCEPT ![self] = Tail(stack[self])]
              /\ UNCHANGED << word, qlock, readersLast, readersQ, writersQ, 
                              wholder, holders, node, runnable, role, w, prev, 
                              pushed, head, batch >>

unlockShared(self) == USub(self) \/ UAcquireQ(self) \/ UCheck(self)
                         \/ UReleaseQ(self) \/ UWake(self) \/ URet(self)

Loop(self) == /\ pc[self] = "Loop"
              /\ \/ /\ role' = [role EXCEPT ![self] = "W"]
                    /\ pc' = [pc EXCEPT ![self] = "WTry"]
                 \/ /\ role' = [role EXCEPT ![self] = "R"]
                    /\ pc' = [pc EXCEPT ![self] = "RFetchAdd"]
              /\ UNCHANGED << word, qlock, readersLast, readersQ, writersQ, 
                              wholder, holders, node, runnable, stack, p, s, h, 
                              w, prev, pushed, head, batch >>

WTry(self) == /\ pc[self] = "WTry"
              /\ IF word = Free
                    THEN /\ word' = [word EXCEPT !.wr = TRUE]
                         /\ pc' = [pc EXCEPT ![self] = "WCS"]
                    ELSE /\ pc' = [pc EXCEPT ![self] = "WAcquireQ"]
                         /\ word' = word
              /\ UNCHANGED << qlock, readersLast, readersQ, writersQ, wholder, 
                              holders, node, runnable, stack, p, s, h, role, w, 
                              prev, pushed, head, batch >>

WAcquireQ(self) == /\ pc[self] = "WAcquireQ"
                   /\ ~qlock
                   /\ qlock' = TRUE
                   /\ pc' = [pc EXCEPT ![self] = "WFetchOr"]
                   /\ UNCHANGED << word, readersLast, readersQ, writersQ, 
                                   wholder, holders, node, runnable, stack, p, 
                                   s, h, role, w, prev, pushed, head, batch >>

WFetchOr(self) == /\ pc[self] = "WFetchOr"
                  /\ w' = [w EXCEPT ![self] = [word EXCEPT !.ww = TRUE]]
                  /\ word' = w'[self]
                  /\ pc' = [pc EXCEPT ![self] = "WCas"]
                  /\ UNCHANGED << qlock, readersLast, readersQ, writersQ, 
                                  wholder, holders, node, runnable, stack, p, 
                                  s, h, role, prev, pushed, head, batch >>

WCas(self) == /\ pc[self] = "WCas"
              /\ IF writersQ = << >> /\ ~w[self].wr /\ w[self].rd = 0
                    THEN /\ IF word = w[self]
                               THEN /\ word' = [wr |-> TRUE, ww |-> FALSE, rd |-> 0]
                                    /\ pushed' = [pushed EXCEPT ![self] = FALSE]
                                    /\ pc' = [pc EXCEPT ![self] = "WReleaseQ"]
                                    /\ w' = w
                               ELSE /\ w' = [w EXCEPT ![self] = word]
                                    /\ pc' = [pc EXCEPT ![self] = "WCas"]
                                    /\ UNCHANGED << word, pushed >>
                    ELSE /\ pc' = [pc EXCEPT ![self] = "WPush"]
                         /\ UNCHANGED << word, w, pushed >>
              /\ UNCHANGED << qlock, readersLast, readersQ, writersQ, wholder, 
                              holders, node, runnable, stack, p, s, h, role, 
                              prev, head, batch >>

WPush(self) == /\ pc[self] = "WPush"
               /\ node' = [node EXCEPT ![self] = "Armed"]
               /\ writersQ' = Append(writersQ, self)
               /\ pushed' = [pushed EXCEPT ![self] = TRUE]
               /\ pc' = [pc EXCEPT ![self] = "WReleaseQ"]
               /\ UNCHANGED << word, qlock, readersLast, readersQ, wholder, 
                               holders, runnable, stack, p, s, h, role, w, 
                               prev, head, batch >>

WReleaseQ(self) == /\ pc[self] = "WReleaseQ"
                   /\ qlock' = FALSE
                   /\ IF pushed[self]
                         THEN /\ pc' = [pc EXCEPT ![self] = "Park"]
                         ELSE /\ pc' = [pc EXCEPT ![self] = "WCS"]
                   /\ UNCHANGED << word, readersLast, readersQ, writersQ, 
                                   wholder, holders, node, runnable, stack, p, 
                                   s, h, role, w, prev, pushed, head, batch >>

WCS(self) == /\ pc[self] = "WCS"
             /\ Assert(wholder = None /\ holders = {}, 
                       "Failure of assertion at line 139, column 9.")
             /\ wholder' = self
             /\ w' = [w EXCEPT ![self] = Free]
             /\ pushed' = [pushed EXCEPT ![self] = FALSE]
             /\ pc' = [pc EXCEPT ![self] = "WUnlock"]
             /\ UNCHANGED << word, qlock, readersLast, readersQ, writersQ, 
                             holders, node, runnable, stack, p, s, h, role, 
                             prev, head, batch >>

WUnlock(self) == /\ pc[self] = "WUnlock"
                 /\ ~qlock
                 /\ qlock' = TRUE
                 /\ pc' = [pc EXCEPT ![self] = "WDrain"]
                 /\ UNCHANGED << word, readersLast, readersQ, writersQ, 
                                 wholder, holders, node, runnable, stack, p, s, 
                                 h, role, w, prev, pushed, head, batch >>

WDrain(self) == /\ pc[self] = "WDrain"
                /\ wholder' = None
                /\ IF readersQ /= << >> /\ (writersQ = << >> \/ ~readersLast)
                      THEN /\ batch' = [batch EXCEPT ![self] = readersQ]
                           /\ readersQ' = << >>
                           /\ readersLast' = TRUE
                           /\ word' = [word EXCEPT !.wr = FALSE, !.rd = @ + Len(batch'[self])]
                           /\ UNCHANGED << writersQ, head >>
                      ELSE /\ IF writersQ /= << >>
                                 THEN /\ readersLast' = FALSE
                                      /\ head' = [head EXCEPT ![self] = Head(writersQ)]
                                      /\ IF Len(writersQ) = 1
                                            THEN /\ word' = [word EXCEPT !.ww = FALSE]
                                            ELSE /\ TRUE
                                                 /\ word' = word
                                      /\ writersQ' = Tail(writersQ)
                                 ELSE /\ word' = [word EXCEPT !.wr = FALSE]
                                      /\ UNCHANGED << readersLast, writersQ, 
                                                      head >>
                           /\ UNCHANGED << readersQ, batch >>
                /\ pc' = [pc EXCEPT ![self] = "WReleaseQ2"]
                /\ UNCHANGED << qlock, holders, node, runnable, stack, p, s, h, 
                                role, w, prev, pushed >>

WReleaseQ2(self) == /\ pc[self] = "WReleaseQ2"
                    /\ qlock' = FALSE
                    /\ pc' = [pc EXCEPT ![self] = "WWake"]
                    /\ UNCHANGED << word, readersLast, readersQ, writersQ, 
                                    wholder, holders, node, runnable, stack, p, 
                                    s, h, role, w, prev, pushed, head, batch >>

WWake(self) == /\ pc[self] = "WWake"
               /\ IF batch[self] /= << >>
                     THEN /\ Assert(node[(Head(batch[self]))] \in {"Armed", "Parked"}, 
                                    "Failure of assertion at line 47, column 5 of macro called at line 168, column 13.")
                          /\ IF node[(Head(batch[self]))] = "Parked"
                                THEN /\ runnable' = [runnable EXCEPT ![(Head(batch[self]))] = TRUE]
                                ELSE /\ TRUE
                                     /\ UNCHANGED runnable
                          /\ node' = [node EXCEPT ![(Head(batch[self]))] = "Woken"]
                          /\ batch' = [batch EXCEPT ![self] = Tail(batch[self])]
                          /\ pc' = [pc EXCEPT ![self] = "WWake"]
                          /\ UNCHANGED << role, head >>
                     ELSE /\ IF head[self] /= None
                                THEN /\ Assert(node[head[self]] \in {"Armed", "Parked"}, 
                                               "Failure of assertion at line 47, column 5 of macro called at line 172, column 13.")
                                     /\ IF node[head[self]] = "Parked"
                                           THEN /\ runnable' = [runnable EXCEPT ![head[self]] = TRUE]
                                           ELSE /\ TRUE
                                                /\ UNCHANGED runnable
                                     /\ node' = [node EXCEPT ![head[self]] = "Woken"]
                                     /\ head' = [head EXCEPT ![self] = None]
                                ELSE /\ TRUE
                                     /\ UNCHANGED << node, runnable, head >>
                          /\ role' = [role EXCEPT ![self] = "W"]
                          /\ pc' = [pc EXCEPT ![self] = "Loop"]
                          /\ batch' = batch
               /\ UNCHANGED << word, qlock, readersLast, readersQ, writersQ, 
                               wholder, holders, stack, p, s, h, w, prev, 
                               pushed >>

RFetchAdd(self) == /\ pc[self] = "RFetchAdd"
                   /\ prev' = [prev EXCEPT ![self] = word]
                   /\ word' = [word EXCEPT !.rd = word.rd + 1]
                   /\ IF ~prev'[self].wr /\ ~prev'[self].ww
                         THEN /\ pc' = [pc EXCEPT ![self] = "RCS"]
                         ELSE /\ pc' = [pc EXCEPT ![self] = "RUndo"]
                   /\ UNCHANGED << qlock, readersLast, readersQ, writersQ, 
                                   wholder, holders, node, runnable, stack, p, 
                                   s, h, role, w, pushed, head, batch >>

RUndo(self) == /\ pc[self] = "RUndo"
               /\ stack' = [stack EXCEPT ![self] = << [ procedure |->  "unlockShared",
                                                        pc        |->  "RAcquireQ",
                                                        p         |->  p[self],
                                                        s         |->  s[self],
                                                        h         |->  h[self] ] >>
                                                    \o stack[self]]
               /\ p' = [p EXCEPT ![self] = Free]
               /\ s' = [s EXCEPT ![self] = Free]
               /\ h' = [h EXCEPT ![self] = None]
               /\ pc' = [pc EXCEPT ![self] = "USub"]
               /\ UNCHANGED << word, qlock, readersLast, readersQ, writersQ, 
                               wholder, holders, node, runnable, role, w, prev, 
                               pushed, head, batch >>

RAcquireQ(self) == /\ pc[self] = "RAcquireQ"
                   /\ ~qlock
                   /\ qlock' = TRUE
                   /\ pc' = [pc EXCEPT ![self] = "RTry"]
                   /\ UNCHANGED << word, readersLast, readersQ, writersQ, 
                                   wholder, holders, node, runnable, stack, p, 
                                   s, h, role, w, prev, pushed, head, batch >>

RTry(self) == /\ pc[self] = "RTry"
              /\ IF ~word.wr /\ ~word.ww
                    THEN /\ word' = [word EXCEPT !.rd = word.rd + 1]
                         /\ pushed' = [pushed EXCEPT ![self] = FALSE]
                         /\ UNCHANGED << readersQ, node >>
                    ELSE /\ node' = [node EXCEPT ![self] = "Armed"]
                         /\ readersQ' = Append(readersQ, self)
                         /\ pushed' = [pushed EXCEPT ![self] = TRUE]
                         /\ word' = word
              /\ pc' = [pc EXCEPT ![self] = "RReleaseQ"]
              /\ UNCHANGED << qlock, readersLast, writersQ, wholder, holders, 
                              runnable, stack, p, s, h, role, w, prev, head, 
                              batch >>

RReleaseQ(self) == /\ pc[self] = "RReleaseQ"
                   /\ qlock' = FALSE
                   /\ IF pushed[self]
                         THEN /\ pc' = [pc EXCEPT ![self] = "Park"]
                         ELSE /\ pc' = [pc EXCEPT ![self] = "RCS"]
                   /\ UNCHANGED << word, readersLast, readersQ, writersQ, 
                                   wholder, holders, node, runnable, stack, p, 
                                   s, h, role, w, prev, pushed, head, batch >>

RCS(self) == /\ pc[self] = "RCS"
             /\ Assert(wholder = None, 
                       "Failure of assertion at line 205, column 9.")
             /\ holders' = (holders \cup {self})
             /\ prev' = [prev EXCEPT ![self] = Free]
             /\ pushed' = [pushed EXCEPT ![self] = FALSE]
             /\ pc' = [pc EXCEPT ![self] = "RUnlockShared"]
             /\ UNCHANGED << word, qlock, readersLast, readersQ, writersQ, 
                             wholder, node, runnable, stack, p, s, h, role, w, 
                             head, batch >>

RUnlockShared(self) == /\ pc[self] = "RUnlockShared"
                       /\ role' = [role EXCEPT ![self] = "W"]
                       /\ stack' = [stack EXCEPT ![self] = << [ procedure |->  "unlockShared",
                                                                pc        |->  "Loop",
                                                                p         |->  p[self],
                                                                s         |->  s[self],
                                                                h         |->  h[self] ] >>
                                                            \o stack[self]]
                       /\ p' = [p EXCEPT ![self] = Free]
                       /\ s' = [s EXCEPT ![self] = Free]
                       /\ h' = [h EXCEPT ![self] = None]
                       /\ pc' = [pc EXCEPT ![self] = "USub"]
                       /\ UNCHANGED << word, qlock, readersLast, readersQ, 
                                       writersQ, wholder, holders, node, 
                                       runnable, w, prev, pushed, head, batch >>

Park(self) == /\ pc[self] = "Park"
              /\ IF node[self] = "Woken"
                    THEN /\ node' = [node EXCEPT ![self] = "Parked"]
                         /\ UNCHANGED runnable
                    ELSE /\ node' = [node EXCEPT ![self] = "Parked"]
                         /\ runnable' = [runnable EXCEPT ![self] = FALSE]
              /\ pc' = [pc EXCEPT ![self] = "Resume"]
              /\ UNCHANGED << word, qlock, readersLast, readersQ, writersQ, 
                              wholder, holders, stack, p, s, h, role, w, prev, 
                              pushed, head, batch >>

Resume(self) == /\ pc[self] = "Resume"
                /\ runnable[self]
                /\ node' = [node EXCEPT ![self] = "Idle"]
                /\ IF role[self] = "W"
                      THEN /\ pc' = [pc EXCEPT ![self] = "WCS"]
                      ELSE /\ pc' = [pc EXCEPT ![self] = "RCS"]
                /\ UNCHANGED << word, qlock, readersLast, readersQ, writersQ, 
                                wholder, holders, runnable, stack, p, s, h, 
                                role, w, prev, pushed, head, batch >>

agent(self) == Loop(self) \/ WTry(self) \/ WAcquireQ(self)
                  \/ WFetchOr(self) \/ WCas(self) \/ WPush(self)
                  \/ WReleaseQ(self) \/ WCS(self) \/ WUnlock(self)
                  \/ WDrain(self) \/ WReleaseQ2(self) \/ WWake(self)
                  \/ RFetchAdd(self) \/ RUndo(self) \/ RAcquireQ(self)
                  \/ RTry(self) \/ RReleaseQ(self) \/ RCS(self)
                  \/ RUnlockShared(self) \/ Park(self) \/ Resume(self)

Next == (\E self \in ProcSet: unlockShared(self))
           \/ (\E self \in Agents: agent(self))

Spec == /\ Init /\ [][Next]_vars
        /\ \A self \in Agents : WF_vars(agent(self)) /\ WF_vars(unlockShared(self))

\* END TRANSLATION 
=============================================================================
