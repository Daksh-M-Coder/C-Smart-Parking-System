/*
 * ============================================================
 *        SMART PARKING MANAGEMENT SYSTEM IN C
 * ============================================================
 *
 *  AUTHOR      : Smart Parking System
 *  LANGUAGE    : C (C99 Standard)
 *  DESCRIPTION : A complete, menu-driven Smart Parking System
 *                that simulates a real-world parking lot using
 *                multiple core Data Structures:
 *
 *    - ARRAY        : Fixed parking slots (physical lot grid)
 *    - STACK        : Entry/Exit log history (LIFO)
 *    - QUEUE        : Waiting list when lot is full (FIFO)
 *    - HASH MAP     : O(1) vehicle lookup by license plate
 *    - LINKED LIST  : Hash map chaining for collision handling
 *
 *  FEE STRUCTURE:
 *    - 2-Wheeler (BIKE) : Rs. 20 per hour
 *    - 4-Wheeler (CAR)  : Rs. 50 per hour
 *
 *  EDGE CASES HANDLED:
 *    - Duplicate vehicle entry
 *    - Parking when lot is full (auto-queue)
 *    - Removing a vehicle not in the lot
 *    - Searching a non-existent plate
 *    - Empty stack/queue operations
 *    - Invalid menu inputs
 *    - Buffer overflow on input strings
 *    - Waiting vehicle auto-assigned on slot free
 *    - Vehicle type mismatch (slot type vs vehicle type)
 *
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

/* ============================================================
 *                    CONSTANTS & MACROS
 * ============================================================ */

#define MAX_SLOTS        10       /* Total physical parking slots in the lot     */
#define MAX_PLATE_LEN    15       /* Max characters in a license plate + '\0'    */
#define MAX_OWNER_LEN    40       /* Max characters in owner name + '\0'         */
#define HASH_SIZE        20       /* Hash table bucket count (prime preferred)   */
#define STACK_CAPACITY   50       /* Max entries in the log stack                */
#define QUEUE_CAPACITY   20       /* Max vehicles in the waiting queue           */

#define RATE_BIKE        20       /* Rs. per hour for 2-wheelers                 */
#define RATE_CAR         50       /* Rs. per hour for 4-wheelers                 */

/* Vehicle type codes — used in slot and vehicle structs */
#define TYPE_BIKE        1
#define TYPE_CAR         2

/* Slot status flags */
#define SLOT_FREE        0
#define SLOT_OCCUPIED    1

/* Log event types for the stack */
#define EVENT_ENTRY      1
#define EVENT_EXIT       2

/* ============================================================
 *                    DATA STRUCTURES
 * ============================================================ */

/*
 * ── STRUCT: Vehicle ─────────────────────────────────────────
 * Represents a vehicle in the system.
 * Used inside parking slots, hash map, and waiting queue.
 */
typedef struct Vehicle {
    char  plate[MAX_PLATE_LEN];   /* License plate number (unique identifier)   */
    char  owner[MAX_OWNER_LEN];   /* Owner name                                 */
    int   type;                   /* TYPE_BIKE or TYPE_CAR                      */
    time_t entry_time;            /* Unix timestamp when vehicle entered         */
} Vehicle;

/*
 * ── STRUCT: ParkingSlot ─────────────────────────────────────
 * Represents one physical slot in the parking lot.
 * The lot is modelled as an array of ParkingSlots.
 */
typedef struct ParkingSlot {
    int     slot_id;              /* Slot number: 1 to MAX_SLOTS                */
    int     slot_type;            /* TYPE_BIKE or TYPE_CAR (fixed at init)      */
    int     status;               /* SLOT_FREE or SLOT_OCCUPIED                 */
    Vehicle vehicle;              /* Vehicle currently parked (valid if OCCUPIED)*/
} ParkingSlot;

/*
 * ── STRUCT: HashNode ─────────────────────────────────────────
 * A node in the hash map's chained linked list.
 * Stores: plate → slot_id mapping for O(1) lookup.
 */
typedef struct HashNode {
    char  plate[MAX_PLATE_LEN];   /* Key: license plate                         */
    int   slot_id;                /* Value: which slot this vehicle is parked in */
    struct HashNode* next;        /* Pointer to next node (chaining)            */
} HashNode;

/*
 * ── STRUCT: HashMap ──────────────────────────────────────────
 * Hash table with separate chaining.
 * Provides O(1) average-case lookup, insert, delete.
 */
typedef struct HashMap {
    HashNode* buckets[HASH_SIZE]; /* Array of linked list heads                 */
} HashMap;

/*
 * ── STRUCT: LogEntry ─────────────────────────────────────────
 * One record pushed onto the log stack.
 * Captures every entry and exit event.
 */
typedef struct LogEntry {
    int   event_type;             /* EVENT_ENTRY or EVENT_EXIT                  */
    int   slot_id;                /* Which slot was involved                    */
    char  plate[MAX_PLATE_LEN];   /* License plate of the vehicle               */
    char  owner[MAX_OWNER_LEN];   /* Owner name                                 */
    int   vehicle_type;           /* TYPE_BIKE or TYPE_CAR                      */
    time_t timestamp;             /* When the event happened                    */
    float fee_charged;            /* Fee paid on exit (0 for entry events)      */
} LogEntry;

/*
 * ── STRUCT: Stack ────────────────────────────────────────────
 * LIFO stack for the entry/exit log.
 * Top of stack = most recent event.
 */
typedef struct Stack {
    LogEntry entries[STACK_CAPACITY]; /* Fixed-size array of log entries        */
    int      top;                     /* Index of the top element (-1 = empty)  */
} Stack;

/*
 * ── STRUCT: WaitingNode ──────────────────────────────────────
 * A node in the waiting queue's linked list.
 */
typedef struct WaitingNode {
    Vehicle           vehicle;    /* The waiting vehicle's data                 */
    struct WaitingNode* next;     /* Pointer to next in queue                   */
} WaitingNode;

/*
 * ── STRUCT: Queue ────────────────────────────────────────────
 * FIFO queue for vehicles waiting for a free slot.
 * Implemented as a linked list (dynamic, no fixed-size waste).
 */
typedef struct Queue {
    WaitingNode* front;           /* Front of queue (dequeue from here)         */
    WaitingNode* rear;            /* Rear of queue  (enqueue here)              */
    int          count;           /* Current number of vehicles waiting         */
} Queue;

/* ============================================================
 *             GLOBAL SYSTEM STATE
 * ============================================================ */

ParkingSlot lot[MAX_SLOTS];       /* The parking lot: array of slots            */
HashMap     vehicle_map;          /* Hash map: plate → slot_id                  */
Stack       log_stack;            /* Log stack: history of entry/exit events     */
Queue       waiting_queue;        /* Waiting queue: vehicles awaiting a slot    */

/* ============================================================
 *             UTILITY FUNCTION PROTOTYPES
 * ============================================================ */

void  init_system(void);
void  init_slots(void);
void  init_hashmap(void);
void  init_stack(void);
void  init_queue(void);

/* Hash Map Operations */
int       hash_function(const char* plate);
void      hashmap_insert(const char* plate, int slot_id);
int       hashmap_search(const char* plate);
void      hashmap_delete(const char* plate);

/* Stack Operations */
int       stack_is_full(void);
int       stack_is_empty(void);
void      stack_push(LogEntry entry);
LogEntry  stack_pop(void);

/* Queue Operations */
int       queue_is_full(void);
int       queue_is_empty(void);
void      queue_enqueue(Vehicle v);
Vehicle   queue_dequeue(void);

/* Core Parking Operations */
void      park_vehicle(void);
void      remove_vehicle(void);
void      search_vehicle(void);
void      display_parking_status(void);
void      display_waiting_list(void);
void      display_log(void);

/* Helper Utilities */
int       find_free_slot(int vehicle_type);
float     calculate_fee(Vehicle v);
void      to_uppercase(char* str);
void      flush_input_buffer(void);
void      print_separator(void);
void      print_vehicle_type(int type);
void      print_event_type(int type);
char*     format_time(time_t t);

/* ============================================================
 *                    MAIN FUNCTION
 * ============================================================ */

int main(void) {
    int choice;

    /* Initialize all data structures before any operation */
    init_system();

    printf("\n");
    print_separator();
    printf("      SMART PARKING MANAGEMENT SYSTEM\n");
    print_separator();

    /* ── Main Menu Loop ── */
    while (1) {
        printf("\n  MAIN MENU\n");
        print_separator();
        printf("  1. Park a Vehicle\n");
        printf("  2. Remove a Vehicle\n");
        printf("  3. Search Vehicle by Plate\n");
        printf("  4. View Parking Lot Status\n");
        printf("  5. View Waiting List\n");
        printf("  6. View Entry/Exit Log\n");
        printf("  0. Exit System\n");
        print_separator();
        printf("  Enter your choice: ");

        /* ── Edge Case: Invalid (non-integer) input ── */
        if (scanf("%d", &choice) != 1) {
            printf("  [ERROR] Invalid input. Please enter a number.\n");
            flush_input_buffer(); /* Clear leftover characters from stdin       */
            continue;
        }
        flush_input_buffer(); /* Flush newline left after scanf                 */

        switch (choice) {
            case 1: park_vehicle();          break;
            case 2: remove_vehicle();        break;
            case 3: search_vehicle();        break;
            case 4: display_parking_status();break;
            case 5: display_waiting_list();  break;
            case 6: display_log();           break;
            case 0:
                printf("\n  System shutting down. Goodbye!\n\n");
                /* ── Cleanup: Free all dynamically allocated queue nodes ── */
                while (!queue_is_empty()) {
                    queue_dequeue(); /* Internally frees each WaitingNode       */
                }
                /* ── Cleanup: Free all hash map chain nodes ── */
                for (int i = 0; i < HASH_SIZE; i++) {
                    HashNode* curr = vehicle_map.buckets[i];
                    while (curr != NULL) {
                        HashNode* temp = curr;
                        curr = curr->next;
                        free(temp);
                    }
                }
                return 0;
            default:
                printf("  [ERROR] Invalid choice. Please select 0–6.\n");
        }
    }
}

/* ============================================================
 *                  INITIALIZATION FUNCTIONS
 * ============================================================ */

/*
 * init_system()
 * Master initializer — calls all sub-initializers.
 * Must be called once at program start before any operation.
 */
void init_system(void) {
    init_slots();    /* Set up the parking lot array                            */
    init_hashmap();  /* Set all hash buckets to NULL                            */
    init_stack();    /* Set stack top to -1 (empty)                             */
    init_queue();    /* Set queue front/rear to NULL, count to 0                */
}

/*
 * init_slots()
 * Initializes the parking lot array.
 * Slots 1–5 are assigned for BIKEs, slots 6–10 for CARs.
 * All slots start as SLOT_FREE.
 */
void init_slots(void) {
    for (int i = 0; i < MAX_SLOTS; i++) {
        lot[i].slot_id   = i + 1;          /* Slot IDs are 1-indexed           */
        lot[i].status    = SLOT_FREE;

        /*
         * Simple zoning strategy:
         *   First half  → BIKE slots
         *   Second half → CAR slots
         */
        if (i < MAX_SLOTS / 2) {
            lot[i].slot_type = TYPE_BIKE;
        } else {
            lot[i].slot_type = TYPE_CAR;
        }

        /* Zero out the vehicle struct to avoid garbage values                  */
        memset(&lot[i].vehicle, 0, sizeof(Vehicle));
    }
}

/*
 * init_hashmap()
 * Sets every bucket pointer to NULL.
 * An empty bucket means no vehicle is mapped there yet.
 */
void init_hashmap(void) {
    for (int i = 0; i < HASH_SIZE; i++) {
        vehicle_map.buckets[i] = NULL;
    }
}

/*
 * init_stack()
 * Sets top = -1 which is the standard "empty stack" convention.
 */
void init_stack(void) {
    log_stack.top = -1;
}

/*
 * init_queue()
 * Sets front and rear to NULL, count to 0.
 */
void init_queue(void) {
    waiting_queue.front = NULL;
    waiting_queue.rear  = NULL;
    waiting_queue.count = 0;
}

/* ============================================================
 *                   HASH MAP OPERATIONS
 * ============================================================
 *
 * We use a simple polynomial hash on the plate string.
 * Collisions are handled by chaining (linked list per bucket).
 * This gives O(1) average for insert, search, delete.
 */

/*
 * hash_function()
 * Computes a bucket index for a given license plate string.
 *
 * Algorithm: djb2-style rolling hash
 *   hash = hash * 31 + char_value
 * Then modulo HASH_SIZE to stay in bounds.
 *
 * Returns: bucket index in [0, HASH_SIZE-1]
 */
int hash_function(const char* plate) {
    unsigned long hash = 5381; /* A well-known prime seed for djb2 hash        */
    int c;

    while ((c = *plate++) != '\0') {
        /* hash * 33 + c  — equivalent to (hash << 5) + hash + c              */
        hash = ((hash << 5) + hash) + (unsigned char)c;
    }

    /* Modulo gives us the bucket index                                         */
    return (int)(hash % HASH_SIZE);
}

/*
 * hashmap_insert()
 * Inserts a (plate → slot_id) pair into the hash map.
 *
 * Steps:
 *  1. Compute bucket index via hash_function
 *  2. Allocate a new HashNode
 *  3. Prepend it to the bucket's linked list (O(1) insert)
 */
void hashmap_insert(const char* plate, int slot_id) {
    int idx = hash_function(plate);

    /* Allocate a new node on the heap                                          */
    HashNode* node = (HashNode*)malloc(sizeof(HashNode));
    if (node == NULL) {
        /* ── Edge Case: Memory allocation failure ── */
        printf("  [ERROR] Memory allocation failed for hash node.\n");
        return;
    }

    /* Copy the plate string safely — avoid buffer overflow                     */
    strncpy(node->plate, plate, MAX_PLATE_LEN - 1);
    node->plate[MAX_PLATE_LEN - 1] = '\0'; /* Ensure null termination          */
    node->slot_id = slot_id;

    /* Prepend to chain: new node → existing head                               */
    node->next = vehicle_map.buckets[idx];
    vehicle_map.buckets[idx] = node;
}

/*
 * hashmap_search()
 * Searches for a plate in the hash map.
 *
 * Returns: slot_id (1-indexed) if found, -1 if not found.
 */
int hashmap_search(const char* plate) {
    int idx = hash_function(plate);
    HashNode* curr = vehicle_map.buckets[idx];

    /* Traverse the chain at this bucket                                        */
    while (curr != NULL) {
        if (strncmp(curr->plate, plate, MAX_PLATE_LEN) == 0) {
            return curr->slot_id; /* Found! Return the slot                    */
        }
        curr = curr->next;
    }

    return -1; /* Not found                                                     */
}

/*
 * hashmap_delete()
 * Removes a plate entry from the hash map.
 *
 * Uses a "previous pointer" technique to unlink the node safely.
 */
void hashmap_delete(const char* plate) {
    int idx = hash_function(plate);
    HashNode* curr = vehicle_map.buckets[idx];
    HashNode* prev = NULL;

    while (curr != NULL) {
        if (strncmp(curr->plate, plate, MAX_PLATE_LEN) == 0) {
            /* Found the node — unlink it from the chain                        */
            if (prev == NULL) {
                /* Node is the head of the chain                                */
                vehicle_map.buckets[idx] = curr->next;
            } else {
                /* Node is in the middle or end                                 */
                prev->next = curr->next;
            }
            free(curr); /* Release memory                                       */
            return;
        }
        prev = curr;
        curr = curr->next;
    }
    /* If we reach here, the plate wasn't in the map — silent return            */
}

/* ============================================================
 *                      STACK OPERATIONS
 * ============================================================
 *
 * The stack records every parking event (entry/exit) in LIFO order.
 * Displaying the log shows most-recent events first.
 */

/* stack_is_full(): Returns 1 if stack has no more space                        */
int stack_is_full(void) {
    return log_stack.top == STACK_CAPACITY - 1;
}

/* stack_is_empty(): Returns 1 if no events have been logged                    */
int stack_is_empty(void) {
    return log_stack.top == -1;
}

/*
 * stack_push()
 * Pushes a LogEntry onto the top of the stack.
 * Edge case: silently skips if stack is full (log overflow).
 */
void stack_push(LogEntry entry) {
    if (stack_is_full()) {
        /* ── Edge Case: Log is full — oldest entry gets overwritten in real
         *    systems; here we simply drop and warn.                           ── */
        printf("  [WARN] Log stack is full. Oldest log will not be overwritten.\n");
        return;
    }
    log_stack.entries[++log_stack.top] = entry; /* Pre-increment then assign    */
}

/*
 * stack_pop()
 * Pops and returns the top LogEntry.
 * Caller must check stack_is_empty() before calling.
 */
LogEntry stack_pop(void) {
    /* ── Edge Case: Popping from empty stack ── */
    if (stack_is_empty()) {
        LogEntry empty;
        memset(&empty, 0, sizeof(LogEntry));
        return empty;
    }
    return log_stack.entries[log_stack.top--]; /* Return then post-decrement    */
}

/* ============================================================
 *                     QUEUE OPERATIONS
 * ============================================================
 *
 * The waiting queue is a FIFO linked list.
 * When a slot frees up, the vehicle at the FRONT gets it first.
 */

/* queue_is_full(): Returns 1 if waiting list has hit the cap                   */
int queue_is_full(void) {
    return waiting_queue.count >= QUEUE_CAPACITY;
}

/* queue_is_empty(): Returns 1 if no vehicles are waiting                       */
int queue_is_empty(void) {
    return waiting_queue.front == NULL;
}

/*
 * queue_enqueue()
 * Adds a vehicle to the REAR of the waiting queue.
 * Dynamically allocates a new WaitingNode.
 */
void queue_enqueue(Vehicle v) {
    /* ── Edge Case: Queue is at capacity ── */
    if (queue_is_full()) {
        printf("  [ERROR] Waiting list is full (%d vehicles). Cannot add more.\n",
               QUEUE_CAPACITY);
        return;
    }

    WaitingNode* node = (WaitingNode*)malloc(sizeof(WaitingNode));
    if (node == NULL) {
        printf("  [ERROR] Memory allocation failed for queue node.\n");
        return;
    }

    node->vehicle = v;    /* Copy the entire vehicle struct                     */
    node->next    = NULL; /* This will be the new tail                          */

    if (queue_is_empty()) {
        /* First element: both front and rear point to it                       */
        waiting_queue.front = node;
        waiting_queue.rear  = node;
    } else {
        /* Append to the current rear and update rear pointer                   */
        waiting_queue.rear->next = node;
        waiting_queue.rear       = node;
    }

    waiting_queue.count++;
}

/*
 * queue_dequeue()
 * Removes and returns the vehicle at the FRONT of the queue.
 * Caller must check queue_is_empty() before calling.
 * Frees the memory of the dequeued node.
 */
Vehicle queue_dequeue(void) {
    Vehicle v;
    memset(&v, 0, sizeof(Vehicle)); /* Default empty vehicle                    */

    /* ── Edge Case: Dequeue from empty queue ── */
    if (queue_is_empty()) {
        return v;
    }

    WaitingNode* temp   = waiting_queue.front;
    v                   = temp->vehicle; /* Copy out the vehicle data           */
    waiting_queue.front = temp->next;    /* Advance front pointer               */

    /* If queue is now empty, rear must also be NULL                            */
    if (waiting_queue.front == NULL) {
        waiting_queue.rear = NULL;
    }

    free(temp); /* Release the dequeued node                                    */
    waiting_queue.count--;
    return v;
}

/* ============================================================
 *                  CORE PARKING OPERATIONS
 * ============================================================ */

/*
 * park_vehicle()
 * Handles parking a new vehicle:
 *   1. Read and validate vehicle details
 *   2. Check for duplicate plate (edge case)
 *   3. Find a free slot of matching type
 *   4. If no slot, add to waiting queue
 *   5. If slot found, park vehicle and log the event
 */
void park_vehicle(void) {
    Vehicle v;
    memset(&v, 0, sizeof(Vehicle)); /* Zero out to avoid garbage values          */

    printf("\n  ── PARK VEHICLE ──\n");

    /* ── Input: License Plate ── */
    printf("  Enter License Plate: ");
    if (fgets(v.plate, MAX_PLATE_LEN, stdin) == NULL) {
        printf("  [ERROR] Failed to read plate number.\n");
        return;
    }
    /* Remove trailing newline from fgets                                        */
    v.plate[strcspn(v.plate, "\n")] = '\0';

    /* ── Edge Case: Empty plate input ── */
    if (strlen(v.plate) == 0) {
        printf("  [ERROR] Plate number cannot be empty.\n");
        return;
    }

    /* Normalize to uppercase for consistent comparisons                         */
    to_uppercase(v.plate);

    /* ── Edge Case: Duplicate plate — vehicle already in lot ── */
    if (hashmap_search(v.plate) != -1) {
        printf("  [ERROR] Vehicle with plate '%s' is already parked in the lot.\n",
               v.plate);
        return;
    }

    /* ── Input: Owner Name ── */
    printf("  Enter Owner Name   : ");
    if (fgets(v.owner, MAX_OWNER_LEN, stdin) == NULL) {
        printf("  [ERROR] Failed to read owner name.\n");
        return;
    }
    v.owner[strcspn(v.owner, "\n")] = '\0';

    /* ── Edge Case: Empty owner name ── */
    if (strlen(v.owner) == 0) {
        printf("  [ERROR] Owner name cannot be empty.\n");
        return;
    }

    /* ── Input: Vehicle Type ── */
    int type_input;
    printf("  Vehicle Type (1=Bike, 2=Car): ");
    if (scanf("%d", &type_input) != 1 ||
        (type_input != TYPE_BIKE && type_input != TYPE_CAR)) {
        printf("  [ERROR] Invalid vehicle type. Enter 1 for Bike or 2 for Car.\n");
        flush_input_buffer();
        return;
    }
    flush_input_buffer();
    v.type = type_input;

    /* Record the current time as entry time                                     */
    v.entry_time = time(NULL);

    /* ── Find a free slot matching this vehicle type ── */
    int slot_idx = find_free_slot(v.type);

    if (slot_idx == -1) {
        /*
         * ── Edge Case: No free slot of this type ──
         * Add vehicle to the waiting queue instead.
         */
        printf("\n  [INFO] No free slot available for ");
        print_vehicle_type(v.type);
        printf(".\n");

        if (queue_is_full()) {
            printf("  [ERROR] Waiting list is also full. Cannot accommodate vehicle.\n");
            return;
        }

        queue_enqueue(v);
        printf("  [QUEUED] Vehicle '%s' added to waiting list. Position: %d\n",
               v.plate, waiting_queue.count);
        return;
    }

    /* ── Park the vehicle in the found slot ── */
    lot[slot_idx].status  = SLOT_OCCUPIED;
    lot[slot_idx].vehicle = v;

    /* Register in hash map for O(1) future lookups                              */
    hashmap_insert(v.plate, lot[slot_idx].slot_id);

    /* ── Push entry event to log stack ── */
    LogEntry log;
    log.event_type   = EVENT_ENTRY;
    log.slot_id      = lot[slot_idx].slot_id;
    log.vehicle_type = v.type;
    log.fee_charged  = 0.0f;  /* No fee on entry                               */
    log.timestamp    = v.entry_time;
    strncpy(log.plate, v.plate, MAX_PLATE_LEN - 1);
    log.plate[MAX_PLATE_LEN - 1] = '\0';
    strncpy(log.owner, v.owner, MAX_OWNER_LEN - 1);
    log.owner[MAX_OWNER_LEN - 1] = '\0';
    stack_push(log);

    /* ── Print confirmation ── */
    printf("\n  [SUCCESS] Vehicle Parked!\n");
    print_separator();
    printf("  Plate    : %s\n", v.plate);
    printf("  Owner    : %s\n", v.owner);
    printf("  Type     : "); print_vehicle_type(v.type); printf("\n");
    printf("  Slot     : %d\n", lot[slot_idx].slot_id);
    printf("  Entry At : %s\n", format_time(v.entry_time));
    print_separator();
}

/*
 * remove_vehicle()
 * Handles a vehicle leaving the parking lot:
 *   1. Search by plate number using hash map
 *   2. Calculate the parking fee based on duration
 *   3. Free the slot
 *   4. Log the exit event
 *   5. If waiting queue has vehicles of this type, auto-assign
 */
void remove_vehicle(void) {
    char plate[MAX_PLATE_LEN];

    printf("\n  ── REMOVE VEHICLE ──\n");
    printf("  Enter License Plate: ");

    if (fgets(plate, MAX_PLATE_LEN, stdin) == NULL) {
        printf("  [ERROR] Failed to read plate number.\n");
        return;
    }
    plate[strcspn(plate, "\n")] = '\0';

    /* ── Edge Case: Empty input ── */
    if (strlen(plate) == 0) {
        printf("  [ERROR] Plate number cannot be empty.\n");
        return;
    }

    to_uppercase(plate);

    /* ── Hash Map Lookup: O(1) average ── */
    int slot_id = hashmap_search(plate);

    /* ── Edge Case: Vehicle not found in the lot ── */
    if (slot_id == -1) {
        printf("  [ERROR] Vehicle with plate '%s' not found in the parking lot.\n",
               plate);
        return;
    }

    /* slot_id is 1-indexed; array index is slot_id - 1                          */
    int idx = slot_id - 1;

    /* Calculate parking fee                                                     */
    float fee = calculate_fee(lot[idx].vehicle);

    /* ── Print exit summary ── */
    printf("\n  [EXIT SUMMARY]\n");
    print_separator();
    printf("  Plate    : %s\n", lot[idx].vehicle.plate);
    printf("  Owner    : %s\n", lot[idx].vehicle.owner);
    printf("  Type     : "); print_vehicle_type(lot[idx].vehicle.type); printf("\n");
    printf("  Slot     : %d\n", slot_id);
    printf("  Entry At : %s\n", format_time(lot[idx].vehicle.entry_time));
    printf("  Exit At  : %s\n", format_time(time(NULL)));
    printf("  Fee Due  : Rs. %.2f\n", fee);
    print_separator();

    /* Save vehicle type before we clear the slot                                */
    int freed_type = lot[idx].vehicle.type;

    /* ── Push exit event to log stack ── */
    LogEntry log;
    log.event_type   = EVENT_EXIT;
    log.slot_id      = slot_id;
    log.vehicle_type = freed_type;
    log.fee_charged  = fee;
    log.timestamp    = time(NULL);
    strncpy(log.plate, lot[idx].vehicle.plate, MAX_PLATE_LEN - 1);
    log.plate[MAX_PLATE_LEN - 1] = '\0';
    strncpy(log.owner, lot[idx].vehicle.owner, MAX_OWNER_LEN - 1);
    log.owner[MAX_OWNER_LEN - 1] = '\0';
    stack_push(log);

    /* ── Free the slot ── */
    lot[idx].status = SLOT_FREE;
    memset(&lot[idx].vehicle, 0, sizeof(Vehicle));

    /* ── Remove from hash map ── */
    hashmap_delete(plate);

    printf("  [SUCCESS] Vehicle removed. Thank you!\n");

    /*
     * ── Auto-assign from waiting queue ──
     * After freeing a slot, check if any waiting vehicle
     * of the same type can now be accommodated.
     */
    if (!queue_is_empty()) {
        /*
         * We need to find a waiting vehicle that matches the freed slot type.
         * Traverse the queue to find the first matching vehicle.
         * NOTE: We check the queue's front first since it's FIFO.
         * For simplicity, we dequeue the front and check type match.
         * If mismatch, we re-enqueue at the rear (fair ordering preserved).
         */
        int   attempts = waiting_queue.count;
        int   assigned = 0;

        for (int i = 0; i < attempts && !assigned; i++) {
            Vehicle waiting = queue_dequeue();

            /* Does this waiting vehicle match the freed slot type?              */
            int new_slot_idx = find_free_slot(waiting.type);

            if (new_slot_idx != -1) {
                /* Assign to slot */
                waiting.entry_time = time(NULL); /* Reset timer — fresh entry   */
                lot[new_slot_idx].status  = SLOT_OCCUPIED;
                lot[new_slot_idx].vehicle = waiting;
                hashmap_insert(waiting.plate, lot[new_slot_idx].slot_id);

                /* Log the auto-assigned entry                                   */
                LogEntry auto_log;
                auto_log.event_type   = EVENT_ENTRY;
                auto_log.slot_id      = lot[new_slot_idx].slot_id;
                auto_log.vehicle_type = waiting.type;
                auto_log.fee_charged  = 0.0f;
                auto_log.timestamp    = waiting.entry_time;
                strncpy(auto_log.plate, waiting.plate, MAX_PLATE_LEN - 1);
                auto_log.plate[MAX_PLATE_LEN - 1] = '\0';
                strncpy(auto_log.owner, waiting.owner, MAX_OWNER_LEN - 1);
                auto_log.owner[MAX_OWNER_LEN - 1] = '\0';
                stack_push(auto_log);

                printf("\n  [AUTO-ASSIGN] Waiting vehicle '%s' (%s) assigned to Slot %d.\n",
                       waiting.plate, waiting.owner, lot[new_slot_idx].slot_id);
                assigned = 1;
            } else {
                /* No matching slot yet — put vehicle back at the rear           */
                queue_enqueue(waiting);
            }
        }
    }
}

/*
 * search_vehicle()
 * Finds a vehicle by license plate using the hash map.
 * Displays full details if found.
 */
void search_vehicle(void) {
    char plate[MAX_PLATE_LEN];

    printf("\n  ── SEARCH VEHICLE ──\n");
    printf("  Enter License Plate: ");

    if (fgets(plate, MAX_PLATE_LEN, stdin) == NULL) {
        printf("  [ERROR] Failed to read plate number.\n");
        return;
    }
    plate[strcspn(plate, "\n")] = '\0';

    /* ── Edge Case: Empty input ── */
    if (strlen(plate) == 0) {
        printf("  [ERROR] Plate number cannot be empty.\n");
        return;
    }

    to_uppercase(plate);

    int slot_id = hashmap_search(plate);

    if (slot_id == -1) {
        printf("  [NOT FOUND] No vehicle with plate '%s' in the lot.\n", plate);

        /*
         * ── Extra check: is the vehicle in the waiting queue? ──
         * Traverse the queue (read-only, no dequeue).
         */
        WaitingNode* curr = waiting_queue.front;
        int pos = 1;
        int found_in_queue = 0;
        while (curr != NULL) {
            if (strncmp(curr->vehicle.plate, plate, MAX_PLATE_LEN) == 0) {
                printf("  [INFO] Vehicle '%s' is in the waiting list at position %d.\n",
                       plate, pos);
                found_in_queue = 1;
                break;
            }
            curr = curr->next;
            pos++;
        }
        if (!found_in_queue) {
            printf("  [INFO] Vehicle is not in the waiting list either.\n");
        }
        return;
    }

    int idx = slot_id - 1;
    float fee = calculate_fee(lot[idx].vehicle);

    printf("\n  [FOUND]\n");
    print_separator();
    printf("  Plate       : %s\n", lot[idx].vehicle.plate);
    printf("  Owner       : %s\n", lot[idx].vehicle.owner);
    printf("  Type        : "); print_vehicle_type(lot[idx].vehicle.type); printf("\n");
    printf("  Slot        : %d\n", slot_id);
    printf("  Entry At    : %s\n", format_time(lot[idx].vehicle.entry_time));
    printf("  Accrued Fee : Rs. %.2f (so far)\n", fee);
    print_separator();
}

/*
 * display_parking_status()
 * Shows the full state of the parking lot array.
 * Each slot shows: ID, type, status, and if occupied — vehicle info.
 */
void display_parking_status(void) {
    int free_count = 0, occupied_count = 0;

    printf("\n  ── PARKING LOT STATUS ──\n");
    print_separator();
    printf("  %-6s %-8s %-10s %-15s %-20s\n",
           "SLOT", "TYPE", "STATUS", "PLATE", "OWNER");
    print_separator();

    for (int i = 0; i < MAX_SLOTS; i++) {
        printf("  %-6d ", lot[i].slot_id);

        /* Print slot type */
        printf("%-8s ", lot[i].slot_type == TYPE_BIKE ? "BIKE" : "CAR");

        if (lot[i].status == SLOT_FREE) {
            printf("%-10s %-15s %-20s\n", "FREE", "---", "---");
            free_count++;
        } else {
            printf("%-10s %-15s %-20s\n",
                   "OCCUPIED",
                   lot[i].vehicle.plate,
                   lot[i].vehicle.owner);
            occupied_count++;
        }
    }

    print_separator();
    printf("  Total Slots: %d  |  Occupied: %d  |  Free: %d  |  Waiting: %d\n",
           MAX_SLOTS, occupied_count, free_count, waiting_queue.count);
    print_separator();
}

/*
 * display_waiting_list()
 * Traverses the waiting queue (linked list) without dequeuing.
 * Shows all vehicles in FIFO order.
 */
void display_waiting_list(void) {
    printf("\n  ── WAITING LIST ──\n");

    /* ── Edge Case: Empty queue ── */
    if (queue_is_empty()) {
        printf("  No vehicles in the waiting list.\n");
        return;
    }

    print_separator();
    printf("  %-5s %-15s %-20s %-8s\n", "POS", "PLATE", "OWNER", "TYPE");
    print_separator();

    WaitingNode* curr = waiting_queue.front;
    int pos = 1;

    /* Traverse the linked list without modifying it                             */
    while (curr != NULL) {
        printf("  %-5d %-15s %-20s ",
               pos, curr->vehicle.plate, curr->vehicle.owner);
        print_vehicle_type(curr->vehicle.type);
        printf("\n");
        curr = curr->next;
        pos++;
    }

    print_separator();
    printf("  Total Waiting: %d\n", waiting_queue.count);
    print_separator();
}

/*
 * display_log()
 * Displays the entry/exit log from the stack.
 * Stack is traversed top-to-bottom (most recent first).
 * Stack contents are NOT modified (we read, not pop).
 */
void display_log(void) {
    printf("\n  ── ENTRY / EXIT LOG (Most Recent First) ──\n");

    /* ── Edge Case: Empty log ── */
    if (stack_is_empty()) {
        printf("  No log entries yet.\n");
        return;
    }

    print_separator();
    printf("  %-8s %-5s %-15s %-20s %-8s %-10s %s\n",
           "EVENT", "SLOT", "PLATE", "OWNER", "TYPE", "FEE", "TIME");
    print_separator();

    /*
     * Read from top to bottom without popping.
     * top is the index of the most recent entry.
     */
    for (int i = log_stack.top; i >= 0; i--) {
        LogEntry* e = &log_stack.entries[i];

        printf("  %-8s %-5d %-15s %-20s ",
               e->event_type == EVENT_ENTRY ? "ENTRY" : "EXIT",
               e->slot_id,
               e->plate,
               e->owner);
        print_vehicle_type(e->vehicle_type);
        printf(" %-10.2f %s\n",
               e->fee_charged,
               format_time(e->timestamp));
    }

    print_separator();
    printf("  Total Log Entries: %d\n", log_stack.top + 1);
    print_separator();
}

/* ============================================================
 *                    HELPER FUNCTIONS
 * ============================================================ */

/*
 * find_free_slot()
 * Scans the lot array for a free slot matching the vehicle type.
 *
 * Returns: array index (0-based) of the first free matching slot,
 *          or -1 if no such slot exists.
 */
int find_free_slot(int vehicle_type) {
    for (int i = 0; i < MAX_SLOTS; i++) {
        if (lot[i].status == SLOT_FREE && lot[i].slot_type == vehicle_type) {
            return i; /* Found a matching free slot                              */
        }
    }
    return -1; /* No free slot of this type                                      */
}

/*
 * calculate_fee()
 * Computes the parking fee based on:
 *   - Duration = current time - entry_time (in seconds → hours)
 *   - Rate depends on vehicle type (BIKE or CAR)
 *
 * Minimum charge: 1 hour (common real-world policy).
 * Returns: fee in Rupees as a float.
 */
float calculate_fee(Vehicle v) {
    time_t now      = time(NULL);
    double duration = difftime(now, v.entry_time); /* Duration in seconds       */

    /*
     * Convert seconds to hours.
     * ceil-like behavior: if parked for any part of an hour, charge full hour.
     * We use integer ceiling division.
     */
    int hours = (int)(duration / 3600);
    if (duration > 0 && (int)duration % 3600 > 0) {
        hours++; /* Partial hour counts as full hour                            */
    }
    if (hours < 1) hours = 1; /* Minimum 1 hour charge                          */

    float rate = (v.type == TYPE_BIKE) ? RATE_BIKE : RATE_CAR;
    return rate * hours;
}

/*
 * to_uppercase()
 * Converts every character of a string to uppercase in-place.
 * Used to normalize license plate input.
 */
void to_uppercase(char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        str[i] = (char)toupper((unsigned char)str[i]);
    }
}

/*
 * flush_input_buffer()
 * Discards all characters remaining in stdin up to and including '\n'.
 * Prevents leftover input from affecting subsequent scanf/fgets calls.
 */
void flush_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/*
 * print_separator()
 * Prints a horizontal divider line for UI formatting.
 */
void print_separator(void) {
    printf("  ─────────────────────────────────────────────────────────────\n");
}

/*
 * print_vehicle_type()
 * Prints the human-readable vehicle type string.
 */
void print_vehicle_type(int type) {
    if (type == TYPE_BIKE) {
        printf("BIKE");
    } else if (type == TYPE_CAR) {
        printf("CAR ");
    } else {
        printf("UNKNOWN");
    }
}

/*
 * print_event_type()
 * Prints the human-readable event type string.
 */
void print_event_type(int type) {
    if (type == EVENT_ENTRY) {
        printf("ENTRY");
    } else {
        printf("EXIT ");
    }
}

/*
 * format_time()
 * Converts a time_t value to a human-readable string.
 * Returns a static buffer — suitable for printf, not for storing.
 *
 * Example output: "2024-03-15 14:35:22"
 */
char* format_time(time_t t) {
    static char buf[32]; /* Static buffer — not thread-safe, fine for this use  */
    struct tm* tm_info = localtime(&t);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_info);
    return buf;
}

/*
 * ============================================================
 * END OF SMART PARKING MANAGEMENT SYSTEM
 * ============================================================
 *
 * DATA STRUCTURES SUMMARY:
 * ┌─────────────────┬────────────────────────────────────────┐
 * │ Data Structure  │ Purpose                                │
 * ├─────────────────┼────────────────────────────────────────┤
 * │ Array           │ Parking lot slots (fixed physical grid)│
 * │ Hash Map        │ O(1) lookup by license plate           │
 * │ Linked List     │ Hash map chaining + Queue nodes        │
 * │ Stack (LIFO)    │ Entry/exit event log (most recent 1st) │
 * │ Queue (FIFO)    │ Waiting list when lot is full          │
 * └─────────────────┴────────────────────────────────────────┘
 *
 * EDGE CASES HANDLED:
 *  ✓ Duplicate vehicle entry attempt
 *  ✓ Parking when lot is full → auto-queue
 *  ✓ Queue full when lot is full
 *  ✓ Removing non-existent vehicle
 *  ✓ Searching a plate not in the lot
 *  ✓ Searching a plate in the waiting queue
 *  ✓ Invalid menu input (non-integer)
 *  ✓ Empty owner/plate input
 *  ✓ Invalid vehicle type input
 *  ✓ Buffer overflow protection (strncpy, fgets)
 *  ✓ Stack overflow (log capacity exceeded)
 *  ✓ Malloc failure (heap exhausted)
 *  ✓ Auto-assign from waiting queue on slot free
 *  ✓ Memory cleanup on exit (queue nodes + hash map nodes)
 *  ✓ Minimum 1-hour fee policy
 * ============================================================
 */
