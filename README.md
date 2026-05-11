<div align="center">

```
  ███████╗███╗   ███╗ █████╗ ██████╗ ████████╗    ██████╗  █████╗ ██████╗ ██╗  ██╗
  ██╔════╝████╗ ████║██╔══██╗██╔══██╗╚══██╔══╝    ██╔══██╗██╔══██╗██╔══██╗██║ ██╔╝
  ███████╗██╔████╔██║███████║██████╔╝   ██║       ██████╔╝███████║██████╔╝█████╔╝ 
  ╚════██║██║╚██╔╝██║██╔══██║██╔══██╗   ██║       ██╔═══╝ ██╔══██║██╔══██╗██╔═██╗ 
  ███████║██║ ╚═╝ ██║██║  ██║██║  ██║   ██║       ██║     ██║  ██║██║  ██║██║  ██╗
  ╚══════╝╚═╝     ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝       ╚═╝     ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝
```

### *A terminal-based Smart Parking Management System in pure C — with live Graphviz diagram generation and full file-based persistence*

<p>
  <img src="https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white"/>
  <img src="https://img.shields.io/badge/Graphviz-2596BE?style=for-the-badge&logo=graphviz&logoColor=white"/>
  <img src="https://img.shields.io/badge/Windows-0078D4?style=for-the-badge&logo=windows&logoColor=white"/>
  <img src="https://img.shields.io/badge/GCC-A42E2B?style=for-the-badge&logo=gnu&logoColor=white"/>
</p>

<p>
  <img src="https://img.shields.io/badge/Data_Structures-5_Types-2ECC71?style=for-the-badge"/>
  <img src="https://img.shields.io/badge/Diagrams-5_Auto_Generated-9B59B6?style=for-the-badge"/>
  <img src="https://img.shields.io/badge/Persistence-File_Based_TXT-E67E22?style=for-the-badge"/>
  <img src="https://img.shields.io/badge/License-MIT-4ecdc4?style=for-the-badge"/>
</p>

<div style="width: 100%; height: 2px; margin: 20px 0; background: linear-gradient(90deg, transparent, #00599C, #2ECC71, #00599C, transparent);"></div>

> **"Every data structure visualised. Every operation logged. Data that survives restarts."**

</div>

---

## 📋 Table of Contents

- [What Is This?](#-what-is-this)
- [Features At A Glance](#-features-at-a-glance)
- [Data Structures Used](#-data-structures-used)
- [Auto-Generated Diagrams](#-auto-generated-diagrams)
- [File Persistence System](#-file-persistence-system)
- [Project Structure](#-project-structure)
- [Quick Start](#-quick-start)
- [Menu Reference](#-menu-reference)
- [Sample Test Data](#-sample-test-data)
- [DS Deep Dive Doc](#-ds-deep-dive-doc)

---

## 🎯 What Is This?

**Smart Parking Management System** is a terminal application written in **pure C** (no external libraries beyond the C standard library and Graphviz) that simulates a real parking lot with:

- **10 slots** — 5 for bikes (Slots 1–5), 5 for cars (Slots 6–10)
- A **waiting queue** when the lot is full (up to 20 vehicles)
- An **entry/exit log** of up to 50 events
- **Automatic fee calculation** based on hours parked
- **5 Graphviz diagrams** auto-regenerated after every operation
- **File-based persistence** — close and reopen, your data is still there

Built as a learning project to demonstrate **5 core data structures** in a real working system — not toy textbook examples, but actual code doing actual work.

---

## ✨ Features At A Glance

| Feature | Detail |
|---|---|
| **Park a vehicle** | Assigns correct slot type (bike/car), rejects duplicates, queues if full |
| **Remove a vehicle** | Calculates fee, logs exit, auto-assigns next waiting vehicle |
| **Search by plate** | O(1) average lookup via hash map — case insensitive |
| **Lot status view** | Full grid — slot, type, status, plate, owner |
| **Waiting list view** | Queue visualised front to rear |
| **Entry/exit log** | Stack displayed top (newest) to bottom (oldest) |
| **5 auto diagrams** | Parking grid, queue, stack, hash map, code flow — PNG + SVG |
| **Persistence** | Slots, queue, log saved to `data/` folder on every change |
| **Case-insensitive plates** | Input `mh12ab1234` or `MH12AB1234` — both work |

---

## 🧠 Data Structures Used

Five data structures, five distinct jobs. None of them are interchangeable.

```
+------------------+-------------------+------------------------------------------+
| Data Structure   | Where It Lives    | What Job It Does                         |
+------------------+-------------------+------------------------------------------+
| Array            | lot[10]           | Stores the 10 parking slots directly     |
|                  |                   | addressable by index                     |
+------------------+-------------------+------------------------------------------+
| Hash Map         | plate_map.buckets | Maps licence plate -> slot ID in O(1)    |
|                  | [20 buckets]      | avg. Uses djb2 hash + chaining           |
+------------------+-------------------+------------------------------------------+
| Stack (LIFO)     | log_stack         | Entry/exit event history. Most recent    |
|                  | [50 capacity]     | event always on top                      |
+------------------+-------------------+------------------------------------------+
| Queue (FIFO)     | waiting_queue     | Vehicles waiting when lot is full.       |
|                  | [20 capacity]     | First come, first served                 |
+------------------+-------------------+------------------------------------------+
| Linked List      | HashNode* chains  | Hash map collision chaining + queue      |
|                  | WaitingNode* next | node links. Dynamic, no wasted space     |
+------------------+-------------------+------------------------------------------+
```

### Why This Combination?

- The **Array** gives constant-time access to any slot by number — `lot[5]` is Slot 6 instantly
- The **Hash Map** means removing a car never scans the lot — it jumps straight to the slot
- The **Stack** keeps the log in natural reverse-chronological order automatically
- The **Queue** enforces fairness — no vehicle cuts the waiting line
- The **Linked List** lets both the hash map and queue grow dynamically without pre-allocating worst-case memory

---

## 🖼️ Auto-Generated Diagrams

After every park/remove operation, 5 Graphviz diagrams are regenerated automatically. No manual steps needed.

| # | File | What It Shows |
|---|---|---|
| 1 | `png/01_parking_lot.png` | Slot grid — GREEN = free, RED = occupied |
| 2 | `png/02_queue.png` | Waiting list as a left-to-right linked list |
| 3 | `png/03_stack.png` | Log stack — top entry first, going downward |
| 4 | `png/04_hashmap.png` | All 20 buckets + collision chains |
| 5 | `png/05_flow.png` | Full code flow, TEAL = current step |

SVG versions are also generated in `svg/` for browser viewing at infinite resolution. The raw `.dot` source files live in `dot/` if you want to modify the Graphviz layout.

**Requires Graphviz installed** — see [Quick Start](#-quick-start).

---

## 💾 File Persistence System

The system saves state to 3 plain-text files under `data/` on every park or remove operation.

```
data/
├── parking_slots.txt    <- all occupied slots (plate, owner, slot, entry time)
├── waiting_queue.txt    <- waiting vehicles in FIFO order
└── log_stack.txt        <- all log entries, bottom to top
```

**File format — human-readable KEY=VALUE blocks:**

```
SLOT_ID=6
SLOT_TYPE=2
PLATE=DL4CAB5678
OWNER=Priya Singh
VTYPE=2
ENTRY_TIME=1746703000
---
```

On startup, all three files are read and the entire DS state (slots, hash map, queue, stack) is rebuilt from them. If no files exist yet (first run), the system starts fresh and creates them after your first park.

**Entry time is a Unix timestamp** — seconds since January 1 1970. Fee = `(now - entry_time) / 3600 * rate`. Rates: Rs.20/hr bikes, Rs.50/hr cars.

> ⚠️ **When loading sample data files:** Set `ENTRY_TIME` to a recent Unix timestamp so fees are realistic. Run `[DateTimeOffset]::UtcNow.ToUnixTimeSeconds()` in PowerShell to get the current value.

---

## 📂 Project Structure

```
C Smart Parking/
│
├── smart_parking_viz.c          <- entire system in one file (~1800 lines)
├── smart_parking_viz.exe        <- compiled binary (after gcc)
│
├── data/                        <- PERSISTENCE (auto-created on first run)
│   ├── parking_slots.txt        <- occupied slot records
│   ├── waiting_queue.txt        <- waiting queue vehicles
│   └── log_stack.txt            <- entry/exit log history
│
├── dot/                         <- Graphviz source files (auto-generated)
│   ├── 01_parking_lot.dot
│   ├── 02_queue.dot
│   ├── 03_stack.dot
│   ├── 04_hashmap.dot
│   └── 05_flow.dot
│
├── png/                         <- PNG diagrams (auto-generated)
│   ├── 01_parking_lot.png
│   ├── 02_queue.png
│   ├── 03_stack.png
│   ├── 04_hashmap.png
│   └── 05_flow.png
│
├── svg/                         <- SVG diagrams — open in browser (auto-generated)
│   ├── 01_parking_lot.svg
│   └── ...
│
├── README.md                    <- you are here
└── DS_EXPLAINER.md              <- deep-dive data structures documentation
```

---

## 🚀 Quick Start

### Prerequisites

| Tool | Version | Install |
|---|---|---|
| **GCC** | Any modern | MinGW-w64 or MSYS2 on Windows |
| **Graphviz** | 2.x+ | [graphviz.org/download](https://graphviz.org/download/) |

> **Critical Graphviz step:** During install, check **"Add Graphviz to system PATH"**. Without this, the diagrams will not generate (the `dot` command won't be found).

### Verify Setup

```powershell
gcc --version       # should print GCC version
dot -V              # should print Graphviz version
```

### Build and Run

```powershell
# Navigate to project folder
cd "C:\Users\YourName\...\C Smart Parking"

# Compile
gcc smart_parking_viz.c -o smart_parking_viz

# Run
.\smart_parking_viz
```

### One-liner (compile + run)

```powershell
gcc smart_parking_viz.c -o smart_parking_viz ; .\smart_parking_viz
```

### Load the Sample Test Data (optional)

Drop the three pre-filled data files into a `data/` folder next to the `.exe` before running:

```
data/parking_slots.txt    <- 10 vehicles across all slots
data/waiting_queue.txt    <- 2 vehicles in the waiting queue
data/log_stack.txt        <- 10 log entries
```

On startup you'll see:
```
[DATA] Loaded 10 occupied slot(s) from data/parking_slots.txt
[DATA] Loaded 2 waiting vehicle(s) from data/waiting_queue.txt
[DATA] Loaded 10 log entry/entries from data/log_stack.txt
```

---

## 📋 Menu Reference

```
------------------------------------------------------------
    SMART PARKING MANAGEMENT SYSTEM
    [Graphviz Diagrams Auto-Generated]
------------------------------------------------------------
MAIN MENU
------------------------------------------------------------
1. Park a Vehicle
2. Remove a Vehicle
3. Search Vehicle by Plate
4. View Parking Lot Status
5. View Waiting List
6. View Entry/Exit Log
0. Exit System
------------------------------------------------------------
```

| Option | Input Required | What Happens |
|---|---|---|
| **1 — Park** | Plate, Owner name, Type (1=bike / 2=car) | Assigns slot or queues; saves; regenerates all 5 diagrams |
| **2 — Remove** | Plate (case insensitive) | Calculates fee, frees slot, auto-assigns next in queue; saves |
| **3 — Search** | Plate | Hash map O(1) lookup; shows slot and entry time |
| **4 — Lot Status** | None | Prints full slot grid |
| **5 — Waiting List** | None | Prints queue front to rear |
| **6 — Log** | None | Prints stack top (newest) to bottom (oldest) |
| **0 — Exit** | None | Exits cleanly |

---

## 🧪 Sample Test Data

10 vehicles — all different plates, names, types, and regions.

### Parking Slots

| Slot | Type | Plate | Owner |
|---|---|---|---|
| 1 | Bike | MH12AB1234 | Rahul Sharma |
| 2 | Bike | KA03MN9012 | Arun Nair |
| 3 | Bike | GJ01HH7890 | Vikram Patel |
| 4 | Bike | UP32EF6789 | Ajay Yadav |
| 5 | Bike | WB20IJ4567 | Debashis Roy |
| 6 | Car | DL4CAB5678 | Priya Singh |
| 7 | Car | TN09XY3456 | Meena Rajan |
| 8 | Car | MH43CD2345 | Sneha Desai |
| 9 | Car | RJ14GH0123 | Kavita Joshi |
| 10 | Car | HR26KL8901 | Pooja Malik |

### Waiting Queue (2 vehicles)

| Position | Plate | Owner | Type |
|---|---|---|---|
| Front (1st) | PB10ZZ1111 | Harpreet Kaur | Bike |
| Rear (2nd) | MH01XY9999 | Suresh Iyer | Car |

> **Fee tip:** Set all `ENTRY_TIME` values to the current Unix timestamp (`[DateTimeOffset]::UtcNow.ToUnixTimeSeconds()` in PowerShell) before loading, otherwise fees will be enormous (parked for 1 year).

### Good Things to Test

```
Option 3 -> Search "MH12AB1234"       -> finds Slot 1 instantly (hash map demo)
Option 2 -> Remove "DL4CAB5678"       -> frees Slot 6, Suresh Iyer auto-assigned (queue demo)
Option 2 -> Remove "MH12AB1234"       -> frees Slot 1, Harpreet Kaur auto-assigned (queue demo)
           -> Close and reopen program -> all data still there (persistence demo)
Option 1 -> Park a 3rd waiting bike   -> joins queue (queue full eventually demo)
Option 6 -> View log                  -> newest events at top (stack demo)
```

---

## 📖 DS Deep Dive Doc

See **[DS_EXPLAINER.md](DS_EXPLAINER.md)** for a full research-style breakdown of every data structure used — with ASCII diagrams, best/average/worst case examples for each, and explanations written so anyone (even without CS background) can follow.

Topics covered:
- Array — the parking lot grid
- Hash Map — O(1) plate lookup with djb2 + collision chaining
- Stack (LIFO) — the entry/exit log
- Queue (FIFO) — the fair waiting list
- Linked List — the backbone of hash chains and queue nodes
- How all 5 work together in a single operation

---

<div align="center">

Built as a C data structures learning project — every decision documented, every structure visualised.

<img src="https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white"/>
<img src="https://img.shields.io/badge/Data_Structures-Array_|_HashMap_|_Stack_|_Queue_|_LinkedList-2ECC71?style=for-the-badge"/>

</div>
