------------------- MODULE AtomicRefCountedIntrusive -------------------
(* Model of the split reference count (Arci<T>) in
   AtomicRefCountedIntrusive.hpp beside this file: pin_copy, flush,
   fold, reset and move-assign. One object; its counter C = live * 2^32 + net
   is kept as the pair [live, net] (the header's |net| < 2^31 shortcut makes
   C = 0 <=> live = 0 /\ net = 0). A word is [p, t]: p says it points at the
   object, t is the 16-bit pin count. Slots shared by all agents can be copied
   from, reset and move-assigned; each agent also owns one private word that
   it copy-constructs from a shared slot, destroys, or moves into a shared
   slot. pin_flush_at is the constant FlushAt. Folded steps: the null check
   before the pin (a plain load), and fold's fetch_add plus the delete that
   follows it (nothing else observes the object between them).
   Not modelled: arci_from_this (one RMW adding live + 1 for a fresh word),
   copying a private word that another agent may destroy (a contract
   violation per the header), the weak-memory pairings marked "unverified"
   (Relacy's job).                                                          *)
EXTENDS Integers, FiniteSets, TLC

CONSTANTS Agents, Slots, FlushAt, NoSlot  \* model values; Agents and Slots are symmetric
Null == [p |-> FALSE, t |-> 0]
One == [p |-> TRUE, t |-> 0]
Sym == Permutations(Agents) \cup Permutations(Slots)

(* --algorithm Arci
variables
    cnt = [live |-> Cardinality(Agents), net |-> Cardinality(Agents)],
    shared = [s \in Slots |-> Null],
    mine = [a \in Agents |-> One],      \* every agent starts holding one word
    old = [a \in Agents |-> Null],      \* word returned by the pin RMW
    cur = [a \in Agents |-> Null],      \* flush's CAS expected value
    w = [a \in Agents |-> Null],        \* word about to be folded
    destroyed = 0;                      \* number of deletes

define
    Quiet == \A a \in Agents : pc[a] = "Loop"
    LiveWords == Cardinality({s \in Slots : shared[s].p}) + Cardinality({a \in Agents : mine[a].p})
    RECURSIVE NetOf(_, _)
    NetOf(f, I) == IF I = {} THEN 0 ELSE
        LET i == CHOOSE i \in I : TRUE IN (IF f[i].p THEN 1 - f[i].t ELSE 0) + NetOf(f, I \ {i})
    DestroyedOnce == destroyed <= 1
    NoDangling == destroyed = 1 =>
        /\ \A s \in Slots : ~shared[s].p
        /\ \A a \in Agents : ~mine[a].p /\ ~old[a].p /\ ~cur[a].p /\ ~w[a].p
    NoUseAfterFree == destroyed = 1 => cnt = [live |-> 0, net |-> 0]
    CountConsistent == Quiet =>
        cnt = [live |-> LiveWords, net |-> NetOf(shared, Slots) + NetOf(mine, Agents)]
    PinBound == \A s \in Slots : shared[s].t <= FlushAt + Cardinality(Agents)
end define;

fair process agent \in Agents
variables sl = NoSlot, pins = 0;
begin
Loop:
    while TRUE do
        either                          \* Arci copy = pin_copy(shared[sl])
            await ~mine[self].p;
            with x \in Slots do sl := x; end with;
            goto CRead;
        or                              \* ~Arci() on the private word
            await mine[self].p;
            w[self] := mine[self];
            mine[self] := Null;
            goto Fold;
        or                              \* shared[sl].reset()
            with x \in Slots do sl := x; end with;
            goto RXchg;
        or                              \* shared[sl] = move(mine)
            with x \in Slots do sl := x; end with;
            goto MTake;
        end either;

    \* ---- pin_copy() ----
    CRead:                              \* null word: no pin, result is null
        if ~shared[sl].p then
            sl := NoSlot;
            goto Loop;
        end if;
    CPin:                               \* old = slot.fetch_add(1)
        old[self] := IF shared[sl].p THEN shared[sl] ELSE Null;
        shared[sl].t := shared[sl].t + 1;
        if ~old[self].p then
            sl := NoSlot;
            goto Loop;
        end if;
    CLive:                              \* subject->ref_fetch_add(live_one)
        cnt.live := cnt.live + 1;
        if old[self].t < FlushAt then
            goto CDone;
        end if;
    FRead:                              \* flush(): current = slot.load()
        cur[self] := shared[sl];
    FLoop:
        if cur[self].p /\ cur[self].t /= 0 then
            pins := cur[self].t;
            cnt.net := cnt.net + pins;  \* credit before the pins leave the word
        else
            goto CDone;
        end if;
    FCas:                               \* compare_exchange_strong(current, encode(subject, 0))
        if shared[sl] = cur[self] then
            shared[sl] := One;
            goto CDone;
        else
            cur[self] := shared[sl];
        end if;
    FUndo:
        cnt.net := cnt.net - pins;
        goto FLoop;
    CDone:
        mine[self] := One;
        old[self] := Null; cur[self] := Null; pins := 0; sl := NoSlot;
        goto Loop;

    \* ---- reset(): fold(exchange(0)) ----
    RXchg:
        w[self] := shared[sl];
        shared[sl] := Null;
        goto Fold;

    \* ---- operator=(Arci&&): fold(exchange(normalize(other.exchange(0)))) ----
    MTake:
        w[self] := mine[self];
        mine[self] := Null;
    MPut:
        old[self] := shared[sl];
        shared[sl] := w[self];
        w[self] := old[self];

    \* ---- fold(w): one fetch_add decides, delete follows ----
    Fold:
        if w[self].p then
            cnt := [live |-> cnt.live - 1, net |-> cnt.net + w[self].t - 1];
            if cnt.live = 0 /\ cnt.net = 0 then   \* PlusCal reads the value just assigned
                destroyed := destroyed + 1;
            end if;
        end if;
        w[self] := Null; old[self] := Null; sl := NoSlot;
        goto Loop;
    end while;
end process;
end algorithm; *)
\* BEGIN TRANSLATION (chksum(pcal) = "b1bde461" /\ chksum(tla) = "d286acf")
VARIABLES cnt, shared, mine, old, cur, w, destroyed, pc

(* define statement *)
Quiet == \A a \in Agents : pc[a] = "Loop"
LiveWords == Cardinality({s \in Slots : shared[s].p}) + Cardinality({a \in Agents : mine[a].p})
RECURSIVE NetOf(_, _)
NetOf(f, I) == IF I = {} THEN 0 ELSE
    LET i == CHOOSE i \in I : TRUE IN (IF f[i].p THEN 1 - f[i].t ELSE 0) + NetOf(f, I \ {i})
DestroyedOnce == destroyed <= 1
NoDangling == destroyed = 1 =>
    /\ \A s \in Slots : ~shared[s].p
    /\ \A a \in Agents : ~mine[a].p /\ ~old[a].p /\ ~cur[a].p /\ ~w[a].p
NoUseAfterFree == destroyed = 1 => cnt = [live |-> 0, net |-> 0]
CountConsistent == Quiet =>
    cnt = [live |-> LiveWords, net |-> NetOf(shared, Slots) + NetOf(mine, Agents)]
PinBound == \A s \in Slots : shared[s].t <= FlushAt + Cardinality(Agents)

VARIABLES sl, pins

vars == << cnt, shared, mine, old, cur, w, destroyed, pc, sl, pins >>

ProcSet == (Agents)

Init == (* Global variables *)
        /\ cnt = [live |-> Cardinality(Agents), net |-> Cardinality(Agents)]
        /\ shared = [s \in Slots |-> Null]
        /\ mine = [a \in Agents |-> One]
        /\ old = [a \in Agents |-> Null]
        /\ cur = [a \in Agents |-> Null]
        /\ w = [a \in Agents |-> Null]
        /\ destroyed = 0
        (* Process agent *)
        /\ sl = [self \in Agents |-> NoSlot]
        /\ pins = [self \in Agents |-> 0]
        /\ pc = [self \in ProcSet |-> "Loop"]

Loop(self) == /\ pc[self] = "Loop"
              /\ \/ /\ ~mine[self].p
                    /\ \E x \in Slots:
                         sl' = [sl EXCEPT ![self] = x]
                    /\ pc' = [pc EXCEPT ![self] = "CRead"]
                    /\ UNCHANGED <<mine, w>>
                 \/ /\ mine[self].p
                    /\ w' = [w EXCEPT ![self] = mine[self]]
                    /\ mine' = [mine EXCEPT ![self] = Null]
                    /\ pc' = [pc EXCEPT ![self] = "Fold"]
                    /\ sl' = sl
                 \/ /\ \E x \in Slots:
                         sl' = [sl EXCEPT ![self] = x]
                    /\ pc' = [pc EXCEPT ![self] = "RXchg"]
                    /\ UNCHANGED <<mine, w>>
                 \/ /\ \E x \in Slots:
                         sl' = [sl EXCEPT ![self] = x]
                    /\ pc' = [pc EXCEPT ![self] = "MTake"]
                    /\ UNCHANGED <<mine, w>>
              /\ UNCHANGED << cnt, shared, old, cur, destroyed, pins >>

CRead(self) == /\ pc[self] = "CRead"
               /\ IF ~shared[sl[self]].p
                     THEN /\ sl' = [sl EXCEPT ![self] = NoSlot]
                          /\ pc' = [pc EXCEPT ![self] = "Loop"]
                     ELSE /\ pc' = [pc EXCEPT ![self] = "CPin"]
                          /\ sl' = sl
               /\ UNCHANGED << cnt, shared, mine, old, cur, w, destroyed, pins >>

CPin(self) == /\ pc[self] = "CPin"
              /\ old' = [old EXCEPT ![self] = IF shared[sl[self]].p THEN shared[sl[self]] ELSE Null]
              /\ shared' = [shared EXCEPT ![sl[self]].t = shared[sl[self]].t + 1]
              /\ IF ~old'[self].p
                    THEN /\ sl' = [sl EXCEPT ![self] = NoSlot]
                         /\ pc' = [pc EXCEPT ![self] = "Loop"]
                    ELSE /\ pc' = [pc EXCEPT ![self] = "CLive"]
                         /\ sl' = sl
              /\ UNCHANGED << cnt, mine, cur, w, destroyed, pins >>

CLive(self) == /\ pc[self] = "CLive"
               /\ cnt' = [cnt EXCEPT !.live = cnt.live + 1]
               /\ IF old[self].t < FlushAt
                     THEN /\ pc' = [pc EXCEPT ![self] = "CDone"]
                     ELSE /\ pc' = [pc EXCEPT ![self] = "FRead"]
               /\ UNCHANGED << shared, mine, old, cur, w, destroyed, sl, pins >>

FRead(self) == /\ pc[self] = "FRead"
               /\ cur' = [cur EXCEPT ![self] = shared[sl[self]]]
               /\ pc' = [pc EXCEPT ![self] = "FLoop"]
               /\ UNCHANGED << cnt, shared, mine, old, w, destroyed, sl, pins >>

FLoop(self) == /\ pc[self] = "FLoop"
               /\ IF cur[self].p /\ cur[self].t /= 0
                     THEN /\ pins' = [pins EXCEPT ![self] = cur[self].t]
                          /\ cnt' = [cnt EXCEPT !.net = cnt.net + pins'[self]]
                          /\ pc' = [pc EXCEPT ![self] = "FCas"]
                     ELSE /\ pc' = [pc EXCEPT ![self] = "CDone"]
                          /\ UNCHANGED << cnt, pins >>
               /\ UNCHANGED << shared, mine, old, cur, w, destroyed, sl >>

FCas(self) == /\ pc[self] = "FCas"
              /\ IF shared[sl[self]] = cur[self]
                    THEN /\ shared' = [shared EXCEPT ![sl[self]] = One]
                         /\ pc' = [pc EXCEPT ![self] = "CDone"]
                         /\ cur' = cur
                    ELSE /\ cur' = [cur EXCEPT ![self] = shared[sl[self]]]
                         /\ pc' = [pc EXCEPT ![self] = "FUndo"]
                         /\ UNCHANGED shared
              /\ UNCHANGED << cnt, mine, old, w, destroyed, sl, pins >>

FUndo(self) == /\ pc[self] = "FUndo"
               /\ cnt' = [cnt EXCEPT !.net = cnt.net - pins[self]]
               /\ pc' = [pc EXCEPT ![self] = "FLoop"]
               /\ UNCHANGED << shared, mine, old, cur, w, destroyed, sl, pins >>

CDone(self) == /\ pc[self] = "CDone"
               /\ mine' = [mine EXCEPT ![self] = One]
               /\ old' = [old EXCEPT ![self] = Null]
               /\ cur' = [cur EXCEPT ![self] = Null]
               /\ pins' = [pins EXCEPT ![self] = 0]
               /\ sl' = [sl EXCEPT ![self] = NoSlot]
               /\ pc' = [pc EXCEPT ![self] = "Loop"]
               /\ UNCHANGED << cnt, shared, w, destroyed >>

RXchg(self) == /\ pc[self] = "RXchg"
               /\ w' = [w EXCEPT ![self] = shared[sl[self]]]
               /\ shared' = [shared EXCEPT ![sl[self]] = Null]
               /\ pc' = [pc EXCEPT ![self] = "Fold"]
               /\ UNCHANGED << cnt, mine, old, cur, destroyed, sl, pins >>

MTake(self) == /\ pc[self] = "MTake"
               /\ w' = [w EXCEPT ![self] = mine[self]]
               /\ mine' = [mine EXCEPT ![self] = Null]
               /\ pc' = [pc EXCEPT ![self] = "MPut"]
               /\ UNCHANGED << cnt, shared, old, cur, destroyed, sl, pins >>

MPut(self) == /\ pc[self] = "MPut"
              /\ old' = [old EXCEPT ![self] = shared[sl[self]]]
              /\ shared' = [shared EXCEPT ![sl[self]] = w[self]]
              /\ w' = [w EXCEPT ![self] = old'[self]]
              /\ pc' = [pc EXCEPT ![self] = "Fold"]
              /\ UNCHANGED << cnt, mine, cur, destroyed, sl, pins >>

Fold(self) == /\ pc[self] = "Fold"
              /\ IF w[self].p
                    THEN /\ cnt' = [live |-> cnt.live - 1, net |-> cnt.net + w[self].t - 1]
                         /\ IF cnt'.live = 0 /\ cnt'.net = 0
                               THEN /\ destroyed' = destroyed + 1
                               ELSE /\ TRUE
                                    /\ UNCHANGED destroyed
                    ELSE /\ TRUE
                         /\ UNCHANGED << cnt, destroyed >>
              /\ w' = [w EXCEPT ![self] = Null]
              /\ old' = [old EXCEPT ![self] = Null]
              /\ sl' = [sl EXCEPT ![self] = NoSlot]
              /\ pc' = [pc EXCEPT ![self] = "Loop"]
              /\ UNCHANGED << shared, mine, cur, pins >>

agent(self) == Loop(self) \/ CRead(self) \/ CPin(self) \/ CLive(self)
                  \/ FRead(self) \/ FLoop(self) \/ FCas(self)
                  \/ FUndo(self) \/ CDone(self) \/ RXchg(self)
                  \/ MTake(self) \/ MPut(self) \/ Fold(self)

Next == (\E self \in Agents: agent(self))

Spec == /\ Init /\ [][Next]_vars
        /\ \A self \in Agents : WF_vars(agent(self))

\* END TRANSLATION 
=============================================================================
