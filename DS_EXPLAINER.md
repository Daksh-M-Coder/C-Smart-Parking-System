# 🧠 Data Structures Deep Dive — Smart Parking System

> *Why each data structure was chosen, how it works, what happens in the best, average, and worst case — explained so a 6-year-old who has never heard the words "Big O" or "algorithm" can follow along. Every concept has a real-world toy story. Every case has working ASCII diagrams.*

---

## 📋 Table of Contents

- [What Even IS a Data Structure?](#-what-even-is-a-data-structure)
- [DS 1 — Array (The Parking Lot Grid)](#-ds-1--array-the-parking-lot-grid)
- [DS 2 — Hash Map (The Magic Phonebook)](#-ds-2--hash-map-the-magic-phonebook)
- [DS 3 — Stack — LIFO (The Undo Button)](#-ds-3--stack--lifo-the-undo-button)
- [DS 4 — Queue — FIFO (The Fair Line)](#-ds-4--queue--fifo-the-fair-line)
- [DS 5 — Linked List (The Treasure Hunt Chain)](#-ds-5--linked-list-the-treasure-hunt-chain)
- [How All 5 Work Together](#-how-all-5-work-together)
- [Speed Cheat Sheet](#-speed-cheat-sheet)

---

## 🌟 What Even IS a Data Structure?

Imagine you have 100 marbles. You need to find the red one.

**Option A:** Throw all 100 into a bag, shake, dig, hope.
**Option B:** Sort them into labeled cups by colour.

A **data structure** is just *how you organise your stuff* so finding, adding, or removing things is easy and fast. Different problems need different organisers — just like you'd use a backpack for school, a fridge for food, and a wallet for money. You wouldn't store your sandwiches in your wallet.

This project uses **5 different data structures**, each chosen for a specific job. Let's meet them one by one.

---

## 🅰️ DS 1 — Array (The Parking Lot Grid)

### The Toy Story Explanation

Picture a row of 10 numbered toy boxes at school: Box 1, Box 2, Box 3... Box 10. Every box has a name tag saying whether it's EMPTY or TAKEN. When a car wants a spot, you look at Box 1 first, then Box 2, and so on — you find the first empty one.

That's exactly an **array** — a row of boxes with numbers on them.

### Why We Used It Here

The parking lot has exactly **10 fixed slots**. Slot 1–5 are for bikes, Slot 6–10 are for cars. The number never changes. Arrays are perfect when you know the exact size upfront because every slot gets a permanent address (like a house number on a street).

```
CODE:   ParkingSlot lot[MAX_SLOTS];    // MAX_SLOTS = 10
        lot[0] = Slot 1 (BIKE)
        lot[1] = Slot 2 (BIKE)
        ...
        lot[9] = Slot 10 (CAR)
```

### What Each Slot Holds

```
+----------------------------------------------------------+
|  lot[5]  (= Slot 6, CAR slot)                           |
|  +---------------+------------------+------------------+ |
|  | slot_id = 6   | status = OCCUPIED| slot_type = CAR  | |
|  +---------------+------------------+------------------+ |
|  | plate = "DL4CAB5678"             | owner = "Priya"  | |
|  +----------------------------------+------------------+ |
|  | entry_time = 1746703000          |                  | |
|  +----------------------------------+------------------+ |
+----------------------------------------------------------+
```

### The Find-a-Free-Slot Operation

When a CAR arrives, the code calls `find_free_slot(TYPE_CAR)`. It starts at index 0 and checks each slot:

```
find_free_slot(CAR):

lot[0] BIKE slot  -> skip (wrong type)
lot[1] BIKE slot  -> skip (wrong type)
lot[2] BIKE slot  -> skip (wrong type)
lot[3] BIKE slot  -> skip (wrong type)
lot[4] BIKE slot  -> skip (wrong type)
lot[5] CAR  slot  -> OCCUPIED -> skip
lot[6] CAR  slot  -> OCCUPIED -> skip
lot[7] CAR  slot  -> FREE!  -> return index 7
```

---

### 📊 Example 1 — BEST CASE

**Situation:** A CAR arrives and Slot 6 (the very first CAR slot, index 5) is free.

```
BEFORE:
+------+------+------+------+------+------+------+------+------+------+
| S1   | S2   | S3   | S4   | S5   | S6   | S7   | S8   | S9   | S10  |
| BIKE | BIKE | BIKE | BIKE | BIKE | CAR  | CAR  | CAR  | CAR  | CAR  |
| FREE | FREE | FREE | FREE | FREE | FREE | OCC  | OCC  | OCC  | OCC  |
+------+------+------+------+------+------+------+------+------+------+
                              ^
                              start checking CAR slots here

Steps taken: 1  (found it immediately at the first CAR slot)
```

**Plain English:** You walk up to the first CAR box, peek inside, it's empty, done. One peek and you're finished. It's like reaching into your bag for your water bottle and it's right on top.

---

### 📊 Example 2 — AVERAGE CASE

**Situation:** A CAR arrives and the first 3 CAR slots are occupied, slot 9 is free.

```
BEFORE:
+------+------+------+------+------+------+------+------+------+------+
| S1   | S2   | S3   | S4   | S5   | S6   | S7   | S8   | S9   | S10  |
| BIKE | BIKE | BIKE | BIKE | BIKE | CAR  | CAR  | CAR  | CAR  | CAR  |
| FREE | FREE | FREE | FREE | FREE | OCC  | OCC  | OCC  | FREE | OCC  |
+------+------+------+------+------+------+------+------+------+------+
                                    X      X      X      ^
                                  skip   skip   skip   FOUND! (4th check)
```

**Plain English:** You peek into 4 boxes before finding an empty one. Not instant, not terrible. Like checking your bag pockets one by one until you find your keys.

---

### 📊 Example 3 — WORST CASE

**Situation:** A BIKE arrives but ALL 5 bike slots are full.

```
BEFORE:
+------+------+------+------+------+------+------+------+------+------+
| S1   | S2   | S3   | S4   | S5   | S6   | S7   | S8   | S9   | S10  |
| BIKE | BIKE | BIKE | BIKE | BIKE | CAR  | CAR  | CAR  | CAR  | CAR  |
| OCC  | OCC  | OCC  | OCC  | OCC  | OCC  | OCC  | OCC  | OCC  | OCC  |
+------+------+------+------+------+------+------+------+------+------+
  X      X      X      X      X
check  check  check  check  check  -> reach end -> NO SLOT FOUND -> join queue
                                      (5 checks, nothing found)
```

**Plain English:** You open every single bike box, all 5 are taken. You had to check all of them before knowing there's no room. This is the slowest it can get — like reading a full dictionary to find out a word isn't in it.

**What happens next:** The bike joins the waiting **Queue** (DS 4 below) — like taking a token at a crowded DMV.

---

## 🗂️ DS 2 — Hash Map (The Magic Phonebook)

### The Toy Story Explanation

Imagine a school with 20 cubbyholes on a wall (numbered 0–19). Every kid's name gets run through a "name machine" that spits out a cubbyhole number. So "RAHUL" goes in, the machine says "7", and Rahul's stuff goes in cubbyhole 7. Next time you need Rahul's stuff, run "RAHUL" through the same machine — it says "7" — you go straight to cubbyhole 7. No searching, no guessing.

That machine is called a **hash function**. The wall of cubbyholes is the **hash map**.

### Why We Used It Here

When a car leaves, you need to find *which slot it's parked in, given only its licence plate*. Without a hash map, you'd scan all 10 slots every time. With the hash map, you just run the plate through the hash function and jump straight to its cubbyhole in the `buckets[]` array of size 20 (`HASH_SIZE = 20`).

```
CODE:   HashMap   plate_map;       // 20 buckets
        buckets[0]  -> NULL
        buckets[1]  -> [MH12AB1234 -> Slot 1] -> NULL
        ...
        buckets[7]  -> [DL4CAB5678 -> Slot 6] -> [GJ01HH7890 -> Slot 3] -> NULL
        ...
        buckets[19] -> NULL
```

### The Hash Function (djb2)

```c
unsigned long hash = 5381;
for each character c in plate:
    hash = hash * 33 + c

final_bucket = hash % 20   // keep it within 0-19
```

**Plain English:** It's like a very specific recipe. You feed in each letter of the plate one at a time, mix it with the current total, and at the end you get a bucket number between 0 and 19. Same plate ALWAYS gives the same bucket.

### What a Collision Is

Sometimes two different plates get the same bucket number. That's called a **collision**. Like two kids whose names both map to cubbyhole 7 — so you chain them together like a mini-list inside the same cubbyhole.

```
bucket[7]:  [MH12AB1234->Slot1] --> [GJ01HH7890->Slot3] --> NULL
                                         ^
                                    collision! two plates
                                    hashed to bucket 7
```

---

### 📊 Example 1 — BEST CASE (no collision)

**Situation:** Search for plate `MH12AB1234`. It hashes to bucket 3. Bucket 3 has exactly one entry and it's the one we want.

```
hashmap_search("MH12AB1234"):

Step 1: hash("MH12AB1234") % 20 = 3
Step 2: Go to bucket[3]

bucket[3]: [MH12AB1234 -> Slot 1] -> NULL
                ^
             MATCH! Return Slot 1

Total nodes checked: 1
```

**Plain English:** Run the name through the machine, open that one cubbyhole, first thing inside is exactly what you wanted. Done in one move. Like unlocking your own locker — no searching.

---

### 📊 Example 2 — AVERAGE CASE (small chain)

**Situation:** Three plates all hashed to bucket 7. You're looking for the second one.

```
hashmap_search("GJ01HH7890"):

Step 1: hash("GJ01HH7890") % 20 = 7
Step 2: Go to bucket[7]

bucket[7]: [MH12AB1234 -> Slot 1] --> [GJ01HH7890 -> Slot 3] --> [WB20IJ4567 -> Slot 5] --> NULL
                 |                           |
               no match,                  MATCH!
               move to next           Return Slot 3

Total nodes checked: 2
```

**Plain English:** You open cubbyhole 7, the first thing is someone else's stuff. You look at the note on it that says "check the next one" — you do — that's the one. Two checks total.

---

### 📊 Example 3 — WORST CASE (all in one bucket)

**Situation:** Pathological input — somehow all 10 vehicles hash to bucket 12. You're looking for the last one.

```
hashmap_search("LAST_PLATE"):

Step 1: hash("LAST_PLATE") % 20 = 12
Step 2: Go to bucket[12]

bucket[12]: [plate1->S1] -> [plate2->S2] -> [plate3->S3] -> ... -> [LAST_PLATE->S10] -> NULL
              check           check           check          ...        MATCH!

Total nodes checked: 10  (scanned the entire chain)
```

**Plain English:** Every single kid's name somehow got assigned to the same cubbyhole. You have to dig through all 10 items piled on top of each other. This almost never happens with a good hash function — which is why the code uses the battle-tested djb2 algorithm. But mathematically it *could* happen.

---

## 📚 DS 3 — Stack — LIFO (The Undo Button)

### The Toy Story Explanation

Picture a stack of plates in a cafeteria. You always put a new plate ON TOP and always take from the TOP. You can never take from the bottom without removing everything above it first.

**L.I.F.O. = Last In, First Out.** The most recently added thing is the first one you get back.

Your phone's UNDO button works exactly like this. Every action you take is pushed onto the stack. When you hit Undo, it pops the most recent action off the top.

### Why We Used It Here

Every time a car parks or leaves, we create a **log entry** and push it onto the stack. When you view the Entry/Exit Log (menu option 6), it shows the most recent event first — exactly what makes sense for a log/history. The stack holds up to `STACK_CAPACITY = 50` entries.

```
CODE:   LogStack  log_stack;
        log_stack.top     = index of most recent entry
        log_stack.entries = array of LogEntry[50]
```

### Visual — How Stack Grows

```
START:              AFTER CAR1 PARKS:       AFTER CAR2 PARKS:       AFTER CAR2 LEAVES:
                                                                     (pop from top)
top = -1            top = 0                 top = 1                  top = 0

|        |          | ENTRY CAR1 |  <- top  | ENTRY CAR2 |  <- top
|        |          |            |          | ENTRY CAR1 |
|        |          |            |          |            |
+--------+          +------------+          +------------+          +------------+
(empty)                                                              | ENTRY CAR1 | <- top
                                                                     +------------+
                                              ^ EXIT CAR2 was here,
                                                got popped off, gone
```

**Note:** When we pop on a removal, we also write a new EXIT entry. The stack grows by 1 on park, grows by 1 on remove (the EXIT log), and displaying is read-only (no pop).

---

### 📊 Example 1 — BEST CASE (push when stack is empty)

**Situation:** Car parks for the first time. Stack is empty (`top = -1`).

```
BEFORE push:                        AFTER push (log: ENTRY, Slot 1, MH12AB1234):
                                    
top = -1                            top = 0
+---------------------------+       +---------------------------+
| entries[49]   [empty]     |       | entries[49]   [empty]     |
| entries[48]   [empty]     |       | entries[48]   [empty]     |
| ...                       |       | ...                       |
| entries[1]    [empty]     |       | entries[1]    [empty]     |
| entries[0]    [empty]     |       | entries[0]    ENTRY,S1,   |  <- top
+---------------------------+       |               MH12AB1234  |
                                    +---------------------------+

Steps: 1  (just write to entries[top+1] and increment top)
```

**Plain English:** The stack is empty. You place the first plate down. One move, instant, can't be easier.

---

### 📊 Example 2 — AVERAGE CASE (push mid-way through)

**Situation:** 25 events already logged. A new event comes in.

```
BEFORE:                             AFTER:
top = 24                            top = 25

entries[25]  [empty]                entries[25]  EXIT, Slot6, HR26KL8901  <- top
entries[24]  ENTRY,S3,[...]  <-top  entries[24]  ENTRY, S3, [...]
...                                 ...
entries[0]   ENTRY,S1,[...]         entries[0]   ENTRY, S1, [...]

Steps: 1  (always just one step regardless of how full it is)
```

**Plain English:** No matter how many plates are already in the stack, adding a new one is always exactly one move. You always know exactly where the top is.

---

### 📊 Example 3 — WORST CASE (stack is full)

**Situation:** All 50 slots are used. A new park event tries to push.

```
top = 49  (= STACK_CAPACITY - 1, the last valid index)

entries[49]  EXIT, S10, HR26KL8901  <- top (FULL)
entries[48]  ENTRY, S9, RJ14GH...
...
entries[0]   ENTRY, S1, MH12AB...

stack_is_full() returns TRUE
push() is blocked -- log entry is dropped with a warning
```

**Plain English:** The plate pile is at the ceiling. You try to put one more on — it falls. The system is designed to handle this by checking `stack_is_full()` before every push. In real life you'd archive old logs, but for a 50-event demo system this is fine.

---

## 🎫 DS 4 — Queue — FIFO (The Fair Line)

### The Toy Story Explanation

Picture kids lining up for ice cream. The first kid who got in line is the first kid who gets ice cream. Nobody cuts. The person at the **front** of the line leaves first; new people join at the **back**.

**F.I.F.O. = First In, First Out.** Totally fair. The one who waited longest gets served first.

### Why We Used It Here

When the parking lot is full, arriving vehicles don't just give up — they join a **waiting list**. The first vehicle that joined the wait gets the next available spot when one frees up. This is the Queue. It can hold up to `QUEUE_CAPACITY = 20` waiting vehicles.

```
CODE:   WaitingQueue waiting_queue;
        waiting_queue.front -> first vehicle in line
        waiting_queue.rear  -> last vehicle in line
        waiting_queue.count -> how many are waiting
```

### Visual — Queue Movement

```
FRONT                                          REAR
  |                                              |
  v                                              v
+----------+     +----------+     +----------+
| Harpreet |---->| Suresh   |---->| Ravi     |
| (arrived |     | (arrived |     | (arrived |
|  first)  |     |  second) |     |  third)  |
+----------+     +----------+     +----------+

Slot frees up -> Harpreet dequeues from FRONT, gets the slot
Harpreet leaves queue:

FRONT                             REAR
  |                                 |
  v                                 v
+----------+     +----------+
| Suresh   |---->| Ravi     |
+----------+     +----------+

New vehicle arrives -> joins at REAR:

FRONT                                          REAR
  |                                              |
  v                                              v
+----------+     +----------+     +----------+
| Suresh   |---->| Ravi     |---->| Neha     |
+----------+     +----------+     +----------+
```

---

### 📊 Example 1 — BEST CASE (enqueue to empty queue)

**Situation:** Lot is full. First vehicle (Harpreet, BIKE) joins the empty waiting list.

```
BEFORE:                         AFTER:
front = NULL                    front ----> [Harpreet, BIKE]
rear  = NULL                    rear  ----> [Harpreet, BIKE]
count = 0                       count = 1

                                Note: front and rear both point to
                                the SAME node when only 1 is waiting.

Steps: 1  (create node, set both front and rear to it)
```

**Plain English:** Nobody was waiting. Harpreet is first. You write her name on a piece of paper and that paper is both the "front of line" AND the "back of line" at the same time, because she's alone.

---

### 📊 Example 2 — AVERAGE CASE (dequeue from middle-sized queue)

**Situation:** 3 people waiting. A slot frees up. The front person (Harpreet) gets assigned.

```
BEFORE:
front -> [Harpreet] --> [Suresh] --> [Ravi] <- rear
count = 3

Step 1: Read front node (Harpreet, BIKE)
Step 2: Move front pointer to next node (Suresh)
Step 3: Free the old front node (Harpreet)
Step 4: Assign Harpreet to the freed slot

AFTER:
front -> [Suresh] --> [Ravi] <- rear
count = 2

Steps: 1  (always just reads and moves the front pointer)
```

**Plain English:** The first person in line steps forward, takes their ice cream, and leaves. The second person is now first. One move, done.

---

### 📊 Example 3 — WORST CASE (queue is completely full)

**Situation:** 20 vehicles are already waiting (`QUEUE_CAPACITY = 20`). A 21st vehicle tries to join.

```
BEFORE:
front -> [V1] --> [V2] --> ... --> [V20] <- rear
count = 20

New vehicle arrives (V21):
queue_is_full() returns TRUE  (count >= QUEUE_CAPACITY)
enqueue() blocked

System message: "Waiting list is also full. Cannot accommodate this vehicle."

AFTER: no change
count = 20
```

**Plain English:** The ice cream line has 20 kids and the teacher said maximum 20. Kid 21 shows up and is told to come back later. The line doesn't grow past its limit.

**Design note:** In a real parking system you'd tell the driver to try again later or direct them to a different lot. Here the system simply rejects them with a message.

---

## 🔗 DS 5 — Linked List (The Treasure Hunt Chain)

### The Toy Story Explanation

Imagine a treasure hunt. Clue #1 is under your pillow. When you find it, it says "next clue is in the fridge." The fridge clue says "next clue is in the garden." Each clue (node) holds the treasure AND a direction to the next clue.

That's a **Linked List**. Each piece of data carries a pointer (arrow) saying where the next piece lives in memory. They don't need to be next to each other physically — they just know who comes next.

### Why We Used It TWICE Here

Linked lists appear in **two places** in this project:

**Place 1 — Hash Map Chaining:** When two plates hash to the same bucket, they form a mini linked list inside that bucket (each node points to the next collision victim).

**Place 2 — The Waiting Queue Nodes:** Each vehicle in the queue is a `WaitingNode` containing the vehicle data + a `next` pointer to the next waiting vehicle.

```c
// Hash map node
struct HashNode {
    char      plate[15];
    int       slot_id;
    HashNode* next;       // <- pointer to next node in same bucket
};

// Queue node
struct WaitingNode {
    Vehicle      vehicle;
    WaitingNode* next;    // <- pointer to next vehicle in queue
};
```

### The Key Difference from an Array

```
ARRAY (fixed houses on a street):
+----+----+----+----+----+
| S1 | S2 | S3 | S4 | S5 |
+----+----+----+----+----+
 [0]  [1]  [2]  [3]  [4]     <- addresses fixed, never move

LINKED LIST (treasure hunt):
[NodeA] ---next---> [NodeB] ---next---> [NodeC] ---next---> NULL
  |                   |                   |
 anywhere           anywhere           anywhere
 in memory          in memory          in memory
```

**Array:** All spots exist upfront, empty or occupied. Fixed size. Direct jump to any index.
**Linked List:** Nodes are created on demand and chained together. Grows and shrinks freely. But to get to node 5, you must pass through nodes 1, 2, 3, 4 first.

---

### 📊 Example 1 — BEST CASE (insert/search at head of chain)

**Situation:** Plate `MH12AB1234` is inserted and there's nothing in bucket 3 yet.

```
BEFORE inserting MH12AB1234:
bucket[3] -> NULL

Step 1: Create new HashNode (plate="MH12AB1234", slot_id=1, next=NULL)
Step 2: Set bucket[3] = new node

AFTER:
bucket[3] -> [MH12AB1234 | Slot 1 | next=NULL]

Later, searching for MH12AB1234:
Go to bucket[3], first node IS the match -> return Slot 1 immediately

Nodes traversed: 1
```

**Plain English:** First clue is exactly what you were looking for. You didn't even need to follow the chain. One look and done.

---

### 📊 Example 2 — AVERAGE CASE (insert as new head, search in mid-chain)

**Situation:** Insert a new plate that collides with an existing one. The code inserts it at the HEAD (front) of the chain (faster than walking to the tail).

```
BEFORE:
bucket[7] -> [GJ01HH7890 | Slot 3 | next=NULL]

Inserting [MH43CD2345 | Slot 8]:
Step 1: new_node->next = bucket[7]     (point new node to old head)
Step 2: bucket[7] = new_node           (new node becomes new head)

AFTER:
bucket[7] -> [MH43CD2345 | Slot 8 | next ->] -> [GJ01HH7890 | Slot 3 | next=NULL]

Searching for GJ01HH7890:
bucket[7] first node: MH43CD2345 -- not a match, follow next
bucket[7] second node: GJ01HH7890 -- MATCH, return Slot 3

Nodes traversed: 2
```

**Plain English:** The newest clue is put at the entrance of the treasure trail, so if you're looking for the new thing you find it fast. But the old clue is now one step deeper in the chain.

---

### 📊 Example 3 — WORST CASE (long chain traversal, deletion)

**Situation:** Delete plate `DL4CAB5678` which is buried at the END of a 5-node collision chain in bucket 12.

```
bucket[12]:
[plate1->S1] -> [plate2->S2] -> [plate3->S3] -> [plate4->S4] -> [DL4CAB5678->S6] -> NULL
    prev=NULL        prev            prev             prev              TARGET

Search process:
Node 1: plate1    != DL4CAB5678, move on  (track prev = node1)
Node 2: plate2    != DL4CAB5678, move on  (track prev = node2)
Node 3: plate3    != DL4CAB5678, move on  (track prev = node3)
Node 4: plate4    != DL4CAB5678, move on  (track prev = node4)
Node 5: DL4CAB5678 == MATCH!

Deletion:
prev(node4)->next = node5->next  (= NULL)
free(node5)

AFTER:
[plate1->S1] -> [plate2->S2] -> [plate3->S3] -> [plate4->S4] -> NULL
```

**Plain English:** You have to follow 5 clues before you find the right one, then carefully re-link the chain to skip it — like removing a link from a paper chain without tearing the rest. Each step is cheap but you have no shortcut to the end.

---

## 🔄 How All 5 Work Together

This is the magic moment — all five DS working as one machine for a single operation:

### Operation: Vehicle Arrives

```
1. ARRAY scan:       find_free_slot() scans lot[] for a free slot
                     |
                     v
2. HASH MAP check:   hashmap_search(plate) -- is this plate already parked?
                     If yes -> reject (duplicate)
                     |
                     v
3a. Slot found:      lot[idx].status = OCCUPIED   (ARRAY write)
                     hashmap_insert(plate, slot_id) (HASH MAP write)
                     stack_push(log_entry)          (STACK push)
                     |
3b. No slot found:   queue_enqueue(vehicle)         (QUEUE + LINKED LIST)
```

```
FULL FLOW DIAGRAM:

Vehicle arrives
     |
     v
+------------------+
| HASH MAP search  |   Is this plate already inside?
| O(1) avg lookup  |   YES -> "Already parked!" reject
+------------------+
     | NO
     v
+------------------+
| ARRAY scan       |   Is there a free slot of correct type?
| scan lot[0..9]   |   NO -> go to queue
+------------------+
     | YES
     |                  +-----------------------------+
     |                  | QUEUE enqueue               |
     |                  | LINKED LIST node created    |
     |                  | Waits for next free slot    |
     |                  +-----------------------------+
     v
+------------------+
| ARRAY write      |   Mark slot as OCCUPIED
| lot[idx].status  |   Save plate, owner, entry_time
+------------------+
     |
     v
+------------------+
| HASH MAP insert  |   plate -> slot_id stored
| O(1) future look |
+------------------+
     |
     v
+------------------+
| STACK push       |   ENTRY event logged at top
| instant O(1)     |
+------------------+
     |
     v
  DONE. Diagrams generated.
```

---

## ⚡ Speed Cheat Sheet

> No jargon, just plain "how many steps does this take?"

| Operation | Data Structure Used | Best Steps | Typical Steps | Worst Steps |
|---|---|---|---|---|
| Find free slot | Array (scan) | 1 | ~half | All slots |
| Look up plate | Hash Map | 1 | 1–3 | All in bucket |
| Add log entry | Stack (push) | 1 | 1 | blocked if full |
| Read last log | Stack (top) | 1 | 1 | 1 |
| Add to wait list | Queue + Linked List | 1 | 1 | blocked if full |
| Serve next waiter | Queue (dequeue) | 1 | 1 | 1 |
| Remove from chain | Linked List | 1 | ~chain/2 | Full chain length |

**The big picture in one sentence:**
> The Hash Map keeps lookup at near-instant speed. The Array keeps the lot state always visible. The Stack keeps history honest (most recent first). The Queue keeps the waiting list fair. The Linked List lets the Hash Map and Queue grow and shrink without wasting fixed space.

Each structure does one job perfectly so the others don't have to try to do it badly.

---

<div align="center">

*Built for learning. Every diagram, every line of C code, every structure here was a deliberate choice.*

</div>
