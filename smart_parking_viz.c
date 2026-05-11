/*
 * ============================================================
 *     SMART PARKING MANAGEMENT SYSTEM -- WITH VISUALIZATION
 * ============================================================
 *
 *  AUTHOR      : Smart Parking System
 *  LANGUAGE    : C (C99 Standard)
 *  OS TARGET   : Windows
 *  EXTRA TOOL  : Graphviz (must be installed + in PATH)
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
 *
 *  HOW TO INSTALL GRAPHVIZ (Windows):
 *  -----------------------------------------------------------------
 *  1. Go to: https://graphviz.org/download/
 *  2. Download the Windows installer (.exe)
 *  3. During install -> CHECK "Add Graphviz to system PATH"
 *  4. Restart your terminal
 *  5. Test: open CMD and type:  dot -V
 *     You should see: dot - graphviz version X.X.X
 *
 *  HOW TO COMPILE:
 *  -----------------------------
 *  gcc -Wall -Wextra -std=c99 -o smart_parking_viz smart_parking_viz.c
 *
 *  HOW TO RUN:
 *  ---------------------
 *  smart_parking_viz.exe
 *
 *  WHAT GETS GENERATED AUTOMATICALLY:
 *  ------------------------------------------------------------------------
 *  After EVERY operation (park/remove/search/view), these files
 *  are created/updated in numbered subfolders beside your .exe:
 *
 *   dot/01_parking_lot.dot  png/01_parking_lot.png  svg/01_parking_lot.svg
 *        -> Visual grid of all 10 slots (GREEN=free, RED=occupied)
 *
 *   dot/02_queue.dot        png/02_queue.png        svg/02_queue.svg
 *        -> Linked list of waiting vehicles (left to right)
 *
 *   dot/03_stack.dot        png/03_stack.png        svg/03_stack.svg
 *        -> Log history stack (top=most recent, going downward)
 *
 *   dot/04_hashmap.dot      png/04_hashmap.png      svg/04_hashmap.svg
 *        -> All 20 hash buckets + chained plate->slot nodes
 *
 *   dot/05_flow.dot         png/05_flow.png         svg/05_flow.svg
 *        -> Complete code flow from program start to current state
 *
 *  DATA STRUCTURES USED:
 *  -----------------------------------------
 *   Array       -> Parking lot slots
 *   Hash Map    -> O(1) plate -> slot lookup
 *   Linked List -> Hash map chaining + Queue nodes
 *   Stack(LIFO) -> Entry/Exit event log
 *   Queue(FIFO) -> Waiting list when lot is full
 *
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <direct.h>   /* _mkdir() -- Windows only */

/* ============================================================
 *                    CONSTANTS & MACROS
 * ============================================================ */

#define MAX_SLOTS        10
#define MAX_PLATE_LEN    15
#define MAX_OWNER_LEN    40
#define HASH_SIZE        20
#define STACK_CAPACITY   50
#define QUEUE_CAPACITY   20

#define RATE_BIKE        20
#define RATE_CAR         50

#define TYPE_BIKE        1
#define TYPE_CAR         2

#define SLOT_FREE        0
#define SLOT_OCCUPIED    1

#define EVENT_ENTRY      1
#define EVENT_EXIT       2

/* -- PERSISTENCE PATHS -- */
#define DATA_DIR          "data"
#define FILE_SLOTS        "data/parking_slots.txt"
#define FILE_QUEUE        "data/waiting_queue.txt"
#define FILE_LOG          "data/log_stack.txt"

/* ============================================================
 *                    DATA STRUCTURES
 * ============================================================ */

typedef struct Vehicle {
    char   plate[MAX_PLATE_LEN];
    char   owner[MAX_OWNER_LEN];
    int    type;
    time_t entry_time;
} Vehicle;

typedef struct ParkingSlot {
    int     slot_id;
    int     slot_type;
    int     status;
    Vehicle vehicle;
} ParkingSlot;

typedef struct HashNode {
    char  plate[MAX_PLATE_LEN];
    int   slot_id;
    struct HashNode* next;
} HashNode;

typedef struct HashMap {
    HashNode* buckets[HASH_SIZE];
} HashMap;

typedef struct LogEntry {
    int    event_type;
    int    slot_id;
    char   plate[MAX_PLATE_LEN];
    char   owner[MAX_OWNER_LEN];
    int    vehicle_type;
    time_t timestamp;
    float  fee_charged;
} LogEntry;

typedef struct Stack {
    LogEntry entries[STACK_CAPACITY];
    int      top;
} Stack;

typedef struct WaitingNode {
    Vehicle             vehicle;
    struct WaitingNode* next;
} WaitingNode;

typedef struct Queue {
    WaitingNode* front;
    WaitingNode* rear;
    int          count;
} Queue;

/* ============================================================
 *             GLOBAL SYSTEM STATE
 * ============================================================ */

ParkingSlot lot[MAX_SLOTS];
HashMap     vehicle_map;
Stack       log_stack;
Queue       waiting_queue;

/*
 * g_flow_step: tracks which step of the program flow we are at.
 * The flow diagram uses this to highlight the current stage.
 * Stages:
 *   0 = Program Started
 *   1 = System Initialized
 *   2 = Menu Shown
 *   3 = Park Vehicle
 *   4 = Remove Vehicle
 *   5 = Search Vehicle
 *   6 = View Parking Status
 *   7 = View Waiting List
 *   8 = View Log
 *   9 = Exit
 */
int g_flow_step = 0;

/* ============================================================
 *             FUNCTION PROTOTYPES
 * ============================================================ */

/* Initialization */
void      init_system(void);
void      init_slots(void);
void      init_hashmap(void);
void      init_stack(void);
void      init_queue(void);

/* Hash Map */
int       hash_function(const char* plate);
void      hashmap_insert(const char* plate, int slot_id);
int       hashmap_search(const char* plate);
void      hashmap_delete(const char* plate);

/* Stack */
int       stack_is_full(void);
int       stack_is_empty(void);
void      stack_push(LogEntry entry);
LogEntry  stack_pop(void);

/* Queue */
int       queue_is_full(void);
int       queue_is_empty(void);
void      queue_enqueue(Vehicle v);
Vehicle   queue_dequeue(void);

/* Core Operations */
void      park_vehicle(void);
void      remove_vehicle(void);
void      search_vehicle(void);
void      display_parking_status(void);
void      display_waiting_list(void);
void      display_log(void);

/* Helpers */
int       find_free_slot(int vehicle_type);
float     calculate_fee(Vehicle v);
void      to_uppercase(char* str);
void      flush_input_buffer(void);
void      print_separator(void);
void      print_vehicle_type(int type);
char*     format_time(time_t t);

/* ---- VISUALIZATION FUNCTIONS (all new) ---- */
void      viz_init_dirs(void);
void      viz_generate_all(void);
void      viz_parking_lot(void);
void      viz_queue(void);
void      viz_stack(void);
void      viz_hashmap(void);
void      viz_flow(int current_step);
void      viz_run_dot(const char* dot_file, const char* png_file, const char* svg_file);
void      viz_open_image(const char* filename);

/* -- PERSISTENCE FUNCTIONS -- */
void      data_init_dir(void);
void      data_save_slots(void);
void      data_save_queue(void);
void      data_save_log(void);
void      data_save_all(void);
void      data_load_all(void);

/* ============================================================
 *                    MAIN FUNCTION
 * ============================================================ */

int main(void) {
    int choice;

    init_system();

    /*
     * FLOW STEP 0 -> 1: Program started, system initialized.
     * Generate the initial flow diagram immediately so the
     * teacher can see the starting state of all DS.
     */
    g_flow_step = 1;
    viz_generate_all();

    printf("\n");
    print_separator();
    printf("      SMART PARKING MANAGEMENT SYSTEM\n");
    printf("      [Graphviz Diagrams Auto-Generated]\n");
    print_separator();

    while (1) {
        /* FLOW STEP 2: We are now showing the menu */
        g_flow_step = 2;

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

        if (scanf("%d", &choice) != 1) {
            printf("  [ERROR] Invalid input. Please enter a number.\n");
            flush_input_buffer();
            continue;
        }
        flush_input_buffer();

        switch (choice) {
            case 1: g_flow_step = 3; park_vehicle();           break;
            case 2: g_flow_step = 4; remove_vehicle();         break;
            case 3: g_flow_step = 5; search_vehicle();         break;
            case 4: g_flow_step = 6; display_parking_status(); break;
            case 5: g_flow_step = 7; display_waiting_list();   break;
            case 6: g_flow_step = 8; display_log();            break;
            case 0:
                g_flow_step = 9;
                /*
                 * Generate one final set of diagrams showing
                 * the EXIT state of all data structures.
                 */
                viz_generate_all();
                printf("\n  System shutting down. Goodbye!\n\n");
                /* Cleanup queue nodes */
                while (!queue_is_empty()) queue_dequeue();
                /* Cleanup hashmap nodes */
                for (int i = 0; i < HASH_SIZE; i++) {
                    HashNode* curr = vehicle_map.buckets[i];
                    while (curr) {
                        HashNode* tmp = curr;
                        curr = curr->next;
                        free(tmp);
                    }
                }
                return 0;
            default:
                printf("  [ERROR] Invalid choice. Enter 0-6.\n");
        }
    }
}

/* ============================================================
 *                  INITIALIZATION FUNCTIONS
 * ============================================================ */

void init_system(void) {
    init_slots();
    init_hashmap();
    init_stack();
    init_queue();
    data_load_all();   /* Restore saved state if data/ files exist */
}

/*
 * Slots 1-5  -> BIKE
 * Slots 6-10 -> CAR
 */
void init_slots(void) {
    for (int i = 0; i < MAX_SLOTS; i++) {
        lot[i].slot_id   = i + 1;
        lot[i].status    = SLOT_FREE;
        lot[i].slot_type = (i < MAX_SLOTS / 2) ? TYPE_BIKE : TYPE_CAR;
        memset(&lot[i].vehicle, 0, sizeof(Vehicle));
    }
}

void init_hashmap(void) {
    for (int i = 0; i < HASH_SIZE; i++)
        vehicle_map.buckets[i] = NULL;
}

void init_stack(void)  { log_stack.top = -1; }

void init_queue(void) {
    waiting_queue.front = NULL;
    waiting_queue.rear  = NULL;
    waiting_queue.count = 0;
}

/* ============================================================
 *                   HASH MAP OPERATIONS
 * ============================================================ */

/*
 * djb2 hash -- multiplies by 33 each step.
 * Gives good distribution for short strings like plates.
 */
int hash_function(const char* plate) {
    unsigned long hash = 5381;
    int c;
    while ((c = *plate++) != '\0')
        hash = ((hash << 5) + hash) + (unsigned char)c;
    return (int)(hash % HASH_SIZE);
}

void hashmap_insert(const char* plate, int slot_id) {
    int idx = hash_function(plate);
    HashNode* node = (HashNode*)malloc(sizeof(HashNode));
    if (!node) { printf("  [ERROR] malloc failed.\n"); return; }
    strncpy(node->plate, plate, MAX_PLATE_LEN - 1);
    node->plate[MAX_PLATE_LEN - 1] = '\0';
    node->slot_id = slot_id;
    node->next = vehicle_map.buckets[idx];
    vehicle_map.buckets[idx] = node;
}

int hashmap_search(const char* plate) {
    int idx = hash_function(plate);
    HashNode* curr = vehicle_map.buckets[idx];
    while (curr) {
        if (strncmp(curr->plate, plate, MAX_PLATE_LEN) == 0)
            return curr->slot_id;
        curr = curr->next;
    }
    return -1;
}

void hashmap_delete(const char* plate) {
    int idx = hash_function(plate);
    HashNode* curr = vehicle_map.buckets[idx];
    HashNode* prev = NULL;
    while (curr) {
        if (strncmp(curr->plate, plate, MAX_PLATE_LEN) == 0) {
            if (prev) prev->next = curr->next;
            else vehicle_map.buckets[idx] = curr->next;
            free(curr);
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

/* ============================================================
 *                      STACK OPERATIONS
 * ============================================================ */

int  stack_is_full(void)  { return log_stack.top == STACK_CAPACITY - 1; }
int  stack_is_empty(void) { return log_stack.top == -1; }

void stack_push(LogEntry entry) {
    if (stack_is_full()) {
        printf("  [WARN] Log stack full.\n");
        return;
    }
    log_stack.entries[++log_stack.top] = entry;
}

LogEntry stack_pop(void) {
    LogEntry empty;
    memset(&empty, 0, sizeof(LogEntry));
    if (stack_is_empty()) return empty;
    return log_stack.entries[log_stack.top--];
}

/* ============================================================
 *                     QUEUE OPERATIONS
 * ============================================================ */

int queue_is_full(void)  { return waiting_queue.count >= QUEUE_CAPACITY; }
int queue_is_empty(void) { return waiting_queue.front == NULL; }

void queue_enqueue(Vehicle v) {
    if (queue_is_full()) {
        printf("  [ERROR] Waiting list full.\n");
        return;
    }
    WaitingNode* node = (WaitingNode*)malloc(sizeof(WaitingNode));
    if (!node) { printf("  [ERROR] malloc failed.\n"); return; }
    node->vehicle = v;
    node->next    = NULL;
    if (queue_is_empty()) {
        waiting_queue.front = node;
        waiting_queue.rear  = node;
    } else {
        waiting_queue.rear->next = node;
        waiting_queue.rear       = node;
    }
    waiting_queue.count++;
}

Vehicle queue_dequeue(void) {
    Vehicle v;
    memset(&v, 0, sizeof(Vehicle));
    if (queue_is_empty()) return v;
    WaitingNode* temp   = waiting_queue.front;
    v                   = temp->vehicle;
    waiting_queue.front = temp->next;
    if (!waiting_queue.front) waiting_queue.rear = NULL;
    free(temp);
    waiting_queue.count--;
    return v;
}

/* ============================================================
 *                  CORE PARKING OPERATIONS
 * ============================================================ */

void park_vehicle(void) {
    Vehicle v;
    memset(&v, 0, sizeof(Vehicle));

    printf("\n  ---- PARK VEHICLE ----\n");
    printf("  Enter License Plate: ");
    if (!fgets(v.plate, MAX_PLATE_LEN, stdin)) { printf("  [ERROR] Read failed.\n"); return; }
    v.plate[strcspn(v.plate, "\n")] = '\0';
    if (strlen(v.plate) == 0) { printf("  [ERROR] Plate cannot be empty.\n"); return; }
    to_uppercase(v.plate);

    if (hashmap_search(v.plate) != -1) {
        printf("  [ERROR] '%s' is already parked.\n", v.plate);
        return;
    }

    printf("  Enter Owner Name   : ");
    if (!fgets(v.owner, MAX_OWNER_LEN, stdin)) { printf("  [ERROR] Read failed.\n"); return; }
    v.owner[strcspn(v.owner, "\n")] = '\0';
    if (strlen(v.owner) == 0) { printf("  [ERROR] Owner cannot be empty.\n"); return; }

    int type_input;
    printf("  Vehicle Type (1=Bike, 2=Car): ");
    if (scanf("%d", &type_input) != 1 || (type_input != 1 && type_input != 2)) {
        printf("  [ERROR] Invalid type.\n");
        flush_input_buffer();
        return;
    }
    flush_input_buffer();
    v.type       = type_input;
    v.entry_time = time(NULL);

    int slot_idx = find_free_slot(v.type);

    if (slot_idx == -1) {
        printf("\n  [INFO] No free slot for ");
        print_vehicle_type(v.type);
        printf(". Adding to waiting list.\n");
        if (queue_is_full()) {
            printf("  [ERROR] Waiting list also full.\n");
        } else {
            queue_enqueue(v);
            printf("  [QUEUED] '%s' at waiting position %d.\n", v.plate, waiting_queue.count);
        }
        /*
         * ---- VISUALIZE ----
         * Even when queued (not parked), regenerate diagrams
         * so the teacher sees the queue grow in real time.
         */
        data_save_all();
        viz_generate_all();
        return;
    }

    lot[slot_idx].status  = SLOT_OCCUPIED;
    lot[slot_idx].vehicle = v;
    hashmap_insert(v.plate, lot[slot_idx].slot_id);

    LogEntry log;
    log.event_type   = EVENT_ENTRY;
    log.slot_id      = lot[slot_idx].slot_id;
    log.vehicle_type = v.type;
    log.fee_charged  = 0.0f;
    log.timestamp    = v.entry_time;
    strncpy(log.plate, v.plate,  MAX_PLATE_LEN - 1); log.plate[MAX_PLATE_LEN-1] = '\0';
    strncpy(log.owner, v.owner, MAX_OWNER_LEN - 1); log.owner[MAX_OWNER_LEN-1] = '\0';
    stack_push(log);

    printf("\n  [SUCCESS] Parked!\n");
    print_separator();
    printf("  Plate : %s  |  Owner : %s  |  Slot : %d  |  Entry : %s\n",
           v.plate, v.owner, lot[slot_idx].slot_id, format_time(v.entry_time));
    print_separator();

    /* ---- VISUALIZE: regenerate all 5 diagrams after every successful park ---- */
    data_save_all();
    viz_generate_all();
}

void remove_vehicle(void) {
    char plate[MAX_PLATE_LEN];
    printf("\n  ---- REMOVE VEHICLE ----\n");
    printf("  Enter License Plate: ");
    if (!fgets(plate, MAX_PLATE_LEN, stdin)) { printf("  [ERROR] Read failed.\n"); return; }
    plate[strcspn(plate, "\n")] = '\0';
    if (strlen(plate) == 0) { printf("  [ERROR] Plate cannot be empty.\n"); return; }
    to_uppercase(plate);

    int slot_id = hashmap_search(plate);
    if (slot_id == -1) {
        printf("  [ERROR] '%s' not found in the lot.\n", plate);
        return;
    }

    int idx  = slot_id - 1;
    float fee = calculate_fee(lot[idx].vehicle);

    printf("\n  [EXIT SUMMARY]\n");
    print_separator();
    printf("  Plate : %s | Owner : %s | Slot : %d\n",
           lot[idx].vehicle.plate, lot[idx].vehicle.owner, slot_id);
    printf("  Entry : %s\n", format_time(lot[idx].vehicle.entry_time));
    printf("  Exit  : %s\n", format_time(time(NULL)));
    printf("  Fee   : Rs. %.2f\n", fee);
    print_separator();

    int freed_type = lot[idx].vehicle.type;

    LogEntry log;
    log.event_type   = EVENT_EXIT;
    log.slot_id      = slot_id;
    log.vehicle_type = freed_type;
    log.fee_charged  = fee;
    log.timestamp    = time(NULL);
    strncpy(log.plate, lot[idx].vehicle.plate, MAX_PLATE_LEN-1); log.plate[MAX_PLATE_LEN-1]='\0';
    strncpy(log.owner, lot[idx].vehicle.owner, MAX_OWNER_LEN-1); log.owner[MAX_OWNER_LEN-1]='\0';
    stack_push(log);

    lot[idx].status = SLOT_FREE;
    memset(&lot[idx].vehicle, 0, sizeof(Vehicle));
    hashmap_delete(plate);

    printf("  [SUCCESS] Vehicle removed.\n");

    /* Auto-assign from waiting queue */
    if (!queue_is_empty()) {
        int attempts = waiting_queue.count;
        int assigned = 0;
        for (int i = 0; i < attempts && !assigned; i++) {
            Vehicle waiting = queue_dequeue();
            int new_idx = find_free_slot(waiting.type);
            if (new_idx != -1) {
                waiting.entry_time = time(NULL);
                lot[new_idx].status  = SLOT_OCCUPIED;
                lot[new_idx].vehicle = waiting;
                hashmap_insert(waiting.plate, lot[new_idx].slot_id);
                LogEntry al;
                al.event_type=EVENT_ENTRY; al.slot_id=lot[new_idx].slot_id;
                al.vehicle_type=waiting.type; al.fee_charged=0.0f; al.timestamp=waiting.entry_time;
                strncpy(al.plate,waiting.plate,MAX_PLATE_LEN-1); al.plate[MAX_PLATE_LEN-1]='\0';
                strncpy(al.owner,waiting.owner,MAX_OWNER_LEN-1); al.owner[MAX_OWNER_LEN-1]='\0';
                stack_push(al);
                printf("\n  [AUTO-ASSIGN] '%s' (%s) -> Slot %d\n",
                       waiting.plate, waiting.owner, lot[new_idx].slot_id);
                assigned = 1;
            } else {
                queue_enqueue(waiting);
            }
        }
    }

    /* ---- VISUALIZE: show updated state after removal ---- */
    data_save_all();
    viz_generate_all();
}

void search_vehicle(void) {
    char plate[MAX_PLATE_LEN];
    printf("\n  ---- SEARCH VEHICLE ----\n");
    printf("  Enter License Plate: ");
    if (!fgets(plate, MAX_PLATE_LEN, stdin)) { printf("  [ERROR] Read failed.\n"); return; }
    plate[strcspn(plate, "\n")] = '\0';
    if (strlen(plate) == 0) { printf("  [ERROR] Plate empty.\n"); return; }
    to_uppercase(plate);

    int slot_id = hashmap_search(plate);
    if (slot_id == -1) {
        printf("  [NOT FOUND] '%s' not in lot.\n", plate);
        WaitingNode* curr = waiting_queue.front;
        int pos = 1, found = 0;
        while (curr) {
            if (strncmp(curr->vehicle.plate, plate, MAX_PLATE_LEN) == 0) {
                printf("  [INFO] '%s' is in waiting list at position %d.\n", plate, pos);
                found = 1; break;
            }
            curr = curr->next; pos++;
        }
        if (!found) printf("  [INFO] Not in waiting list either.\n");
    } else {
        int idx = slot_id - 1;
        float fee = calculate_fee(lot[idx].vehicle);
        printf("\n  [FOUND]\n");
        print_separator();
        printf("  Plate : %s | Owner : %s | Slot : %d\n",
               lot[idx].vehicle.plate, lot[idx].vehicle.owner, slot_id);
        printf("  Entry : %s | Accrued Fee : Rs. %.2f\n",
               format_time(lot[idx].vehicle.entry_time), fee);
        print_separator();
    }

    /* ---- VISUALIZE: highlight current hashmap state ---- */
    viz_generate_all();
}

void display_parking_status(void) {
    int free_c = 0, occ_c = 0;
    printf("\n  ---- PARKING LOT STATUS ----\n");
    print_separator();
    printf("  %-6s %-8s %-10s %-15s %-20s\n","SLOT","TYPE","STATUS","PLATE","OWNER");
    print_separator();
    for (int i = 0; i < MAX_SLOTS; i++) {
        printf("  %-6d %-8s ", lot[i].slot_id,
               lot[i].slot_type == TYPE_BIKE ? "BIKE" : "CAR");
        if (lot[i].status == SLOT_FREE) {
            printf("%-10s %-15s %-20s\n","FREE","---","---"); free_c++;
        } else {
            printf("%-10s %-15s %-20s\n","OCCUPIED",
                   lot[i].vehicle.plate, lot[i].vehicle.owner); occ_c++;
        }
    }
    print_separator();
    printf("  Total:%d  Occupied:%d  Free:%d  Waiting:%d\n",
           MAX_SLOTS, occ_c, free_c, waiting_queue.count);
    print_separator();
    viz_generate_all();
}

void display_waiting_list(void) {
    printf("\n  --- WAITING LIST ---\n");
    if (queue_is_empty()) { printf("  No vehicles waiting.\n"); viz_generate_all(); return; }
    print_separator();
    printf("  %-5s %-15s %-20s %-8s\n","POS","PLATE","OWNER","TYPE");
    print_separator();
    WaitingNode* curr = waiting_queue.front;
    int pos = 1;
    while (curr) {
        printf("  %-5d %-15s %-20s ", pos, curr->vehicle.plate, curr->vehicle.owner);
        print_vehicle_type(curr->vehicle.type);
        printf("\n");
        curr = curr->next; pos++;
    }
    print_separator();
    viz_generate_all();
}

void display_log(void) {
    printf("\n  --- ENTRY / EXIT LOG (Most Recent First) ---\n");
    if (stack_is_empty()) { printf("  No log entries yet.\n"); viz_generate_all(); return; }
    print_separator();
    printf("  %-8s %-5s %-15s %-8s %-10s %s\n","EVENT","SLOT","PLATE","TYPE","FEE","TIME");
    print_separator();
    for (int i = log_stack.top; i >= 0; i--) {
        LogEntry* e = &log_stack.entries[i];
        printf("  %-8s %-5d %-15s %-8s %-10.2f %s\n",
               e->event_type == EVENT_ENTRY ? "ENTRY" : "EXIT",
               e->slot_id, e->plate,
               e->vehicle_type == TYPE_BIKE ? "BIKE" : "CAR",
               e->fee_charged, format_time(e->timestamp));
    }
    print_separator();
    viz_generate_all();
}

/* ============================================================
 *                    HELPER FUNCTIONS
 * ============================================================ */

int find_free_slot(int vehicle_type) {
    for (int i = 0; i < MAX_SLOTS; i++)
        if (lot[i].status == SLOT_FREE && lot[i].slot_type == vehicle_type)
            return i;
    return -1;
}

float calculate_fee(Vehicle v) {
    time_t now = time(NULL);
    double dur = difftime(now, v.entry_time);
    int hours = (int)(dur / 3600);
    if (dur > 0 && (int)dur % 3600 > 0) hours++;
    if (hours < 1) hours = 1;
    return ((v.type == TYPE_BIKE) ? RATE_BIKE : RATE_CAR) * hours;
}

void to_uppercase(char* str) {
    for (int i = 0; str[i]; i++)
        str[i] = (char)toupper((unsigned char)str[i]);
}

void flush_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void print_separator(void) {
    printf("  ------------------------------------------------------------------\n");
}

void print_vehicle_type(int type) {
    printf("%s", type == TYPE_BIKE ? "BIKE" : "CAR");
}

char* format_time(time_t t) {
    static char buf[32];
    struct tm* tm_info = localtime(&t);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_info);
    return buf;
}

/* ============================================================
 *                  PERSISTENCE -- FILE I/O
 * ============================================================
 *
 * PURPOSE:
 *   Save and restore the entire parking system state to plain
 *   text files so data survives across program restarts.
 *
 * FILES (all inside data/ folder):
 *   data/parking_slots.txt  -- occupied slot records
 *   data/waiting_queue.txt  -- vehicles in waiting queue (FIFO order)
 *   data/log_stack.txt      -- entry/exit event log (push order)
 *
 * FORMAT (human-readable, one record per block):
 *   Each record is a set of KEY=VALUE lines followed by "---"
 *   Free slots are NOT written (they are re-inited from scratch).
 *   On load: DS is re-initialised first, then records are replayed
 *   so hashmap and queue pointers are rebuilt correctly.
 *
 * SAFETY RULES applied here:
 *   - All fopen() results checked before use
 *   - All fgets() results checked; buffer sizes respected
 *   - strncpy with explicit null-termination on every string field
 *   - On any read error the partial load is skipped gracefully
 *   - Files are always fclose()'d before returning
 * ============================================================ */

/*
 * data_init_dir()
 * Creates the data/ directory if it does not exist.
 */
void data_init_dir(void) {
    _mkdir(DATA_DIR);
}

/* ------------------------------------------------------------------
 * data_save_slots()
 * Writes every OCCUPIED slot to data/parking_slots.txt.
 * Free slots are skipped -- they are rebuilt by init_slots() on load.
 *
 * Record format (one block per occupied slot):
 *   SLOT_ID=6
 *   SLOT_TYPE=2
 *   PLATE=MH12AB1234
 *   OWNER=Rahul Sharma
 *   VTYPE=2
 *   ENTRY_TIME=1715000000
 *   ---
 * ------------------------------------------------------------------ */
void data_save_slots(void) {
    data_init_dir();
    FILE* fp = fopen(FILE_SLOTS, "w");
    if (!fp) { printf("  [DATA ERROR] Cannot write %s\n", FILE_SLOTS); return; }
    int saved = 0;
    for (int i = 0; i < MAX_SLOTS; i++) {
        if (lot[i].status == SLOT_OCCUPIED) {
            fprintf(fp, "SLOT_ID=%d\n",        lot[i].slot_id);
            fprintf(fp, "SLOT_TYPE=%d\n",      lot[i].slot_type);
            fprintf(fp, "PLATE=%s\n",          lot[i].vehicle.plate);
            fprintf(fp, "OWNER=%s\n",          lot[i].vehicle.owner);
            fprintf(fp, "VTYPE=%d\n",          lot[i].vehicle.type);
            fprintf(fp, "ENTRY_TIME=%lld\n",   (long long)lot[i].vehicle.entry_time);
            fprintf(fp, "---\n");
            saved++;
        }
    }
    fclose(fp);
    printf("  [DATA] Saved %d occupied slot(s) -> %s\n", saved, FILE_SLOTS);
}

/* ------------------------------------------------------------------
 * data_save_queue()
 * Writes every vehicle in the waiting queue (front to rear).
 *
 * Record format:
 *   PLATE=DL1CA0001
 *   OWNER=Priya Singh
 *   VTYPE=1
 *   ENTRY_TIME=1715000100
 *   ---
 * ------------------------------------------------------------------ */
void data_save_queue(void) {
    data_init_dir();
    FILE* fp = fopen(FILE_QUEUE, "w");
    if (!fp) { printf("  [DATA ERROR] Cannot write %s\n", FILE_QUEUE); return; }
    WaitingNode* curr = waiting_queue.front;
    int saved = 0;
    while (curr) {
        fprintf(fp, "PLATE=%s\n",        curr->vehicle.plate);
        fprintf(fp, "OWNER=%s\n",        curr->vehicle.owner);
        fprintf(fp, "VTYPE=%d\n",        curr->vehicle.type);
        fprintf(fp, "ENTRY_TIME=%lld\n", (long long)curr->vehicle.entry_time);
        fprintf(fp, "---\n");
        curr = curr->next;
        saved++;
    }
    fclose(fp);
    printf("  [DATA] Saved %d waiting vehicle(s) -> %s\n", saved, FILE_QUEUE);
}

/* ------------------------------------------------------------------
 * data_save_log()
 * Writes the log stack from BOTTOM (oldest) to TOP (newest).
 * On load we push them in this order so stack is restored correctly.
 *
 * Record format:
 *   EVENT=1
 *   SLOT_ID=3
 *   PLATE=MH14XY5678
 *   OWNER=Amit Kumar
 *   VTYPE=1
 *   FEE=20.00
 *   TIMESTAMP=1715000200
 *   ---
 * ------------------------------------------------------------------ */
void data_save_log(void) {
    data_init_dir();
    FILE* fp = fopen(FILE_LOG, "w");
    if (!fp) { printf("  [DATA ERROR] Cannot write %s\n", FILE_LOG); return; }
    for (int i = 0; i <= log_stack.top; i++) {
        LogEntry* e = &log_stack.entries[i];
        fprintf(fp, "EVENT=%d\n",        e->event_type);
        fprintf(fp, "SLOT_ID=%d\n",      e->slot_id);
        fprintf(fp, "PLATE=%s\n",        e->plate);
        fprintf(fp, "OWNER=%s\n",        e->owner);
        fprintf(fp, "VTYPE=%d\n",        e->vehicle_type);
        fprintf(fp, "FEE=%.2f\n",        e->fee_charged);
        fprintf(fp, "TIMESTAMP=%lld\n",  (long long)e->timestamp);
        fprintf(fp, "---\n");
    }
    fclose(fp);
    printf("  [DATA] Saved %d log entry/entries -> %s\n", log_stack.top + 1, FILE_LOG);
}

/* ------------------------------------------------------------------
 * data_save_all()
 * Master save -- call after every state-changing operation.
 * ------------------------------------------------------------------ */
void data_save_all(void) {
    data_save_slots();
    data_save_queue();
    data_save_log();
}

/* ------------------------------------------------------------------
 * data_load_all()
 * Reads all three files and rebuilds DS state on startup.
 * Called once in init_system() after all DS are initialised empty.
 * If a file doesn't exist (first run), fopen returns NULL and we
 * skip that section cleanly -- no error, fresh start.
 * ------------------------------------------------------------------ */
void data_load_all(void) {

    /* -- 1. LOAD SLOTS -- */
    {
        FILE* fp = fopen(FILE_SLOTS, "r");
        if (fp) {
            char line[128];
            int  slot_id = -1, slot_type = -1, vtype = -1;
            long long entry_ll = 0;
            char plate[MAX_PLATE_LEN] = {0};
            char owner[MAX_OWNER_LEN] = {0};
            int  loaded = 0;
            while (fgets(line, sizeof(line), fp)) {
                line[strcspn(line, "\r\n")] = '\0';
                if (strcmp(line, "---") == 0) {
                    if (slot_id >= 1 && slot_id <= MAX_SLOTS &&
                        plate[0] != '\0' && owner[0] != '\0' &&
                        (vtype == TYPE_BIKE || vtype == TYPE_CAR)) {
                        int idx = slot_id - 1;
                        lot[idx].status    = SLOT_OCCUPIED;
                        lot[idx].slot_type = slot_type;
                        strncpy(lot[idx].vehicle.plate, plate, MAX_PLATE_LEN - 1);
                        lot[idx].vehicle.plate[MAX_PLATE_LEN - 1] = '\0';
                        strncpy(lot[idx].vehicle.owner, owner, MAX_OWNER_LEN - 1);
                        lot[idx].vehicle.owner[MAX_OWNER_LEN - 1] = '\0';
                        lot[idx].vehicle.type       = vtype;
                        lot[idx].vehicle.entry_time = (time_t)entry_ll;
                        hashmap_insert(plate, slot_id);
                        loaded++;
                    }
                    slot_id = -1; slot_type = -1; vtype = -1; entry_ll = 0;
                    memset(plate, 0, sizeof(plate));
                    memset(owner, 0, sizeof(owner));
                } else if (strncmp(line, "SLOT_ID=",   8)  == 0) { sscanf(line + 8,  "%d",   &slot_id);
                } else if (strncmp(line, "SLOT_TYPE=", 10) == 0) { sscanf(line + 10, "%d",   &slot_type);
                } else if (strncmp(line, "PLATE=",     6)  == 0) { strncpy(plate, line + 6, MAX_PLATE_LEN - 1); plate[MAX_PLATE_LEN-1]='\0';
                } else if (strncmp(line, "OWNER=",     6)  == 0) { strncpy(owner, line + 6, MAX_OWNER_LEN - 1); owner[MAX_OWNER_LEN-1]='\0';
                } else if (strncmp(line, "VTYPE=",     6)  == 0) { sscanf(line + 6,  "%d",   &vtype);
                } else if (strncmp(line, "ENTRY_TIME=",11) == 0) { sscanf(line + 11, "%lld", &entry_ll);
                }
            }
            fclose(fp);
            if (loaded > 0)
                printf("  [DATA] Loaded %d occupied slot(s) from %s\n", loaded, FILE_SLOTS);
        }
    }

    /* -- 2. LOAD QUEUE -- */
    {
        FILE* fp = fopen(FILE_QUEUE, "r");
        if (fp) {
            char line[128];
            int  vtype = -1;
            long long entry_ll = 0;
            char plate[MAX_PLATE_LEN] = {0};
            char owner[MAX_OWNER_LEN] = {0};
            int  loaded = 0;
            while (fgets(line, sizeof(line), fp)) {
                line[strcspn(line, "\r\n")] = '\0';
                if (strcmp(line, "---") == 0) {
                    if (plate[0] != '\0' && owner[0] != '\0' &&
                        (vtype == TYPE_BIKE || vtype == TYPE_CAR)) {
                        Vehicle v;
                        memset(&v, 0, sizeof(Vehicle));
                        strncpy(v.plate, plate, MAX_PLATE_LEN - 1); v.plate[MAX_PLATE_LEN-1]='\0';
                        strncpy(v.owner, owner, MAX_OWNER_LEN - 1); v.owner[MAX_OWNER_LEN-1]='\0';
                        v.type       = vtype;
                        v.entry_time = (time_t)entry_ll;
                        queue_enqueue(v);
                        loaded++;
                    }
                    vtype = -1; entry_ll = 0;
                    memset(plate, 0, sizeof(plate));
                    memset(owner, 0, sizeof(owner));
                } else if (strncmp(line, "PLATE=",     6)  == 0) { strncpy(plate, line + 6, MAX_PLATE_LEN - 1); plate[MAX_PLATE_LEN-1]='\0';
                } else if (strncmp(line, "OWNER=",     6)  == 0) { strncpy(owner, line + 6, MAX_OWNER_LEN - 1); owner[MAX_OWNER_LEN-1]='\0';
                } else if (strncmp(line, "VTYPE=",     6)  == 0) { sscanf(line + 6,  "%d",   &vtype);
                } else if (strncmp(line, "ENTRY_TIME=",11) == 0) { sscanf(line + 11, "%lld", &entry_ll);
                }
            }
            fclose(fp);
            if (loaded > 0)
                printf("  [DATA] Loaded %d waiting vehicle(s) from %s\n", loaded, FILE_QUEUE);
        }
    }

    /* -- 3. LOAD LOG STACK -- */
    {
        FILE* fp = fopen(FILE_LOG, "r");
        if (fp) {
            char line[128];
            int  event_type = -1, slot_id = -1, vtype = -1;
            float fee = 0.0f;
            long long ts_ll = 0;
            char plate[MAX_PLATE_LEN] = {0};
            char owner[MAX_OWNER_LEN] = {0};
            int  loaded = 0;
            while (fgets(line, sizeof(line), fp)) {
                line[strcspn(line, "\r\n")] = '\0';
                if (strcmp(line, "---") == 0) {
                    if ((event_type == EVENT_ENTRY || event_type == EVENT_EXIT) &&
                        slot_id >= 1 && plate[0] != '\0' && !stack_is_full()) {
                        LogEntry e;
                        e.event_type   = event_type;
                        e.slot_id      = slot_id;
                        e.vehicle_type = vtype;
                        e.fee_charged  = fee;
                        e.timestamp    = (time_t)ts_ll;
                        strncpy(e.plate, plate, MAX_PLATE_LEN - 1); e.plate[MAX_PLATE_LEN-1]='\0';
                        strncpy(e.owner, owner, MAX_OWNER_LEN - 1); e.owner[MAX_OWNER_LEN-1]='\0';
                        stack_push(e);
                        loaded++;
                    }
                    event_type = -1; slot_id = -1; vtype = -1; fee = 0.0f; ts_ll = 0;
                    memset(plate, 0, sizeof(plate));
                    memset(owner, 0, sizeof(owner));
                } else if (strncmp(line, "EVENT=",     6)  == 0) { sscanf(line + 6,  "%d",   &event_type);
                } else if (strncmp(line, "SLOT_ID=",   8)  == 0) { sscanf(line + 8,  "%d",   &slot_id);
                } else if (strncmp(line, "PLATE=",     6)  == 0) { strncpy(plate, line + 6, MAX_PLATE_LEN - 1); plate[MAX_PLATE_LEN-1]='\0';
                } else if (strncmp(line, "OWNER=",     6)  == 0) { strncpy(owner, line + 6, MAX_OWNER_LEN - 1); owner[MAX_OWNER_LEN-1]='\0';
                } else if (strncmp(line, "VTYPE=",     6)  == 0) { sscanf(line + 6,  "%d",   &vtype);
                } else if (strncmp(line, "FEE=",       4)  == 0) { sscanf(line + 4,  "%f",   &fee);
                } else if (strncmp(line, "TIMESTAMP=", 10) == 0) { sscanf(line + 10, "%lld", &ts_ll);
                }
            }
            fclose(fp);
            if (loaded > 0)
                printf("  [DATA] Loaded %d log entry/entries from %s\n", loaded, FILE_LOG);
        }
    }
}

/* ============================================================ */

/* ============================================================
 *
 *         ##+   ##+##+#######+##+   ##+ #####+ ##+
 *         ##|   ##|##|+==###++##|   ##|##+==##+##|
 *         ##|   ##|##|  ###++ ##|   ##|#######|##|
 *         +##+ ##++##| ###++  ##|   ##|##+==##|##|
 *          +####++ ##|#######++######++##|  ##|#######+
 *           +===+  +=++======+ +=====+ +=+  +=++======+
 *
 *         VISUALIZATION ENGINE -- GRAPHVIZ DOT GENERATORS
 *
 * ============================================================
 *
 * HOW THIS WORKS (for the teacher/student):
 * ------------------------------------------------------------------------------------
 * Each viz_XXX() function:
 *   1. Opens a .dot file for writing
 *   2. Writes Graphviz DOT language describing the DS state
 *   3. Calls viz_run_dot() which runs "dot" command to
 *      convert .dot -> .png AND .dot -> .svg
 *
 * DOT Language Basics:
 *   digraph G { ... }       -> directed graph
 *   A -> B                  -> arrow from A to B
 *   node [color=red]        -> style for all nodes
 *   A [label="text"]        -> custom label for node A
 *   A -> B [label="chain"]  -> label on an edge/arrow
 *
 * COLOR SCHEME (consistent across all diagrams):
 *   GREEN  (#2ecc71) -> Free slot / Entry event / Empty bucket
 *   RED    (#e74c3c) -> Occupied slot / Exit event
 *   BLUE   (#3498db) -> Waiting queue node
 *   ORANGE (#e67e22) -> Stack log entry
 *   PURPLE (#9b59b6) -> Hash bucket header
 *   YELLOW (#f1c40f) -> Hash chained node (collision)
 *   GREY   (#95a5a6) -> NULL pointer / inactive
 *   TEAL   (#1abc9c) -> Highlighted / active step in flow
 *
 * ============================================================ */

/*
 * viz_run_dot()
 * -------------------------
 * Takes a .dot file and runs Graphviz twice:
 *   dot -Tpng input.dot -o output.png
 *   dot -Tsvg input.dot -o output.svg
 *
 * Uses system() -- standard C way to run shell commands.
 * On Windows this runs inside cmd.exe automatically.
 *
 * If Graphviz is not installed, system() will fail silently.
 * The .dot file is still written, so Graphviz can be run later.
 */
void viz_run_dot(const char* dot_file, const char* png_file, const char* svg_file) {
    char cmd[512];

    /* Build and run PNG command */
    snprintf(cmd, sizeof(cmd), "dot -Tpng \"%s\" -o \"%s\" 2>nul", dot_file, png_file);
    system(cmd);

    /* Build and run SVG command */
    snprintf(cmd, sizeof(cmd), "dot -Tsvg \"%s\" -o \"%s\" 2>nul", dot_file, svg_file);
    system(cmd);
}

/*
 * viz_open_image()
 * ---------------------------------
 * Opens a PNG file using Windows "start" command.
 * This pops up the default image viewer.
 * Only called after parking lot diagram (main overview).
 * We don't auto-open all 5 to avoid flooding screen.
 */
void viz_open_image(const char* filename) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "start \"\" \"%s\"", filename);
    system(cmd);
}

/*
 * viz_init_dirs()
 * --------------------------------
 * Creates output subdirectories if they don't already exist.
 * Uses _mkdir() (Windows). Silently ignores EEXIST errors.
 *
 *   dot/   -> all .dot source files
 *   png/   -> all rendered .png images
 *   svg/   -> all rendered .svg images
 */
void viz_init_dirs(void) {
    _mkdir("dot");
    _mkdir("png");
    _mkdir("svg");
    _mkdir(DATA_DIR);
}

/*
 * viz_generate_all()
 * -------------------------------------
 * Master function -- called after every user operation.
 * Regenerates ALL 5 diagrams in sequence.
 * Also updates the flow diagram to highlight current step.
 *
 * Output folder layout:
 *   dot/01_parking_lot.dot  png/01_parking_lot.png  svg/01_parking_lot.svg
 *   dot/02_queue.dot        png/02_queue.png        svg/02_queue.svg
 *   dot/03_stack.dot        png/03_stack.png        svg/03_stack.svg
 *   dot/04_hashmap.dot      png/04_hashmap.png      svg/04_hashmap.svg
 *   dot/05_flow.dot         png/05_flow.png         svg/05_flow.svg
 */
void viz_generate_all(void) {
    viz_init_dirs();
    printf("\n  [VIZ] Generating diagrams...\n");
    viz_parking_lot();           /* 1. Slot grid                 */
    viz_queue();                 /* 2. Waiting list linked list  */
    viz_stack();                 /* 3. Log stack (LIFO)          */
    viz_hashmap();               /* 4. Hash map with chains      */
    viz_flow(g_flow_step);       /* 5. Code flow diagram         */
    printf("  [VIZ] Done! Check dot/ png/ svg/ folders.\n");
    printf("  [VIZ] Files: 01_parking_lot | 02_queue | 03_stack | 04_hashmap | 05_flow\n");
}

/* ============================================================
 *  DIAGRAM 1: PARKING LOT GRID
 * ============================================================
 *
 * Generates a visual grid of all parking slots.
 *
 * HOW TO READ:
 *   +--------------------+        +--------------------------------------------+
 *   | Slot 1   |        | Slot 6               |
 *   | BIKE     |        | CAR                  |
 *   |  FREE    |        | OCCUPIED             |
 *   | (green)  |        | MH12AB1234           |
 *   +--------------------+        | Rahul  (red)         |
 *                       +--------------------------------------------+
 *
 * Layout: Two rows -- top row = BIKE slots, bottom = CAR slots.
 * This matches the real physical layout of a parking lot.
 */
void viz_parking_lot(void) {
    FILE* fp = fopen("dot/01_parking_lot.dot", "w");
    if (!fp) { printf("  [VIZ ERROR] Cannot write dot/01_parking_lot.dot\n"); return; }

    fprintf(fp, "digraph ParkingLot {\n");
    fprintf(fp, "    graph [label=\"PARKING LOT STATE\\n"
                "(GREEN=Free | RED=Occupied)\","
                " fontsize=18, fontname=\"Helvetica\","
                " bgcolor=\"#f8f9fa\", pad=0.5];\n");
    fprintf(fp, "    node  [fontname=\"Helvetica\", fontsize=12];\n");
    fprintf(fp, "    edge  [style=invis];\n"); /* No visible arrows -- record layout */
    fprintf(fp, "    rankdir=LR;\n");

    /*
     * We use Graphviz "record" shape to make each slot look
     * like a structured box with multiple labeled fields.
     * Format: label="{line1 | line2 | line3}"
     */
    for (int i = 0; i < MAX_SLOTS; i++) {
        ParkingSlot* s = &lot[i];
        const char* type_str = (s->slot_type == TYPE_BIKE) ? "BIKE" : "CAR";

        if (s->status == SLOT_FREE) {
            /* GREEN box for free slot */
            fprintf(fp,
                "    Slot%d [shape=record, style=filled,"
                " fillcolor=\"#2ecc71\", fontcolor=\"white\","
                " label=\"{Slot %d | %s | FREE}\"];\n",
                s->slot_id, s->slot_id, type_str);
        } else {
            /* RED box for occupied slot -- show plate and owner */
            fprintf(fp,
                "    Slot%d [shape=record, style=filled,"
                " fillcolor=\"#e74c3c\", fontcolor=\"white\","
                " label=\"{Slot %d | %s | OCCUPIED | %s | %s}\"];\n",
                s->slot_id, s->slot_id, type_str,
                s->vehicle.plate, s->vehicle.owner);
        }
    }

    /*
     * Force BIKE slots (1-5) on top row, CAR slots (6-10) on bottom.
     * Graphviz "rank=same" puts nodes on the same horizontal level.
     */
    fprintf(fp, "    { rank=same; Slot1; Slot2; Slot3; Slot4; Slot5; }\n");
    fprintf(fp, "    { rank=same; Slot6; Slot7; Slot8; Slot9; Slot10; }\n");

    /* Invisible edges to enforce left-to-right ordering within each row */
    fprintf(fp, "    Slot1->Slot2->Slot3->Slot4->Slot5 [style=invis];\n");
    fprintf(fp, "    Slot6->Slot7->Slot8->Slot9->Slot10 [style=invis];\n");

    /* Invisible connector between rows so layout stays clean */
    fprintf(fp, "    Slot1->Slot6 [style=invis];\n");

    /* Legend */
    fprintf(fp, "    Legend [shape=plaintext, label=\""
                "Legend:\\nGREEN = Free Slot\\nRED = Occupied Slot\"];\n");

    fprintf(fp, "}\n");
    fclose(fp);

    viz_run_dot("dot/01_parking_lot.dot", "png/01_parking_lot.png", "svg/01_parking_lot.svg");
    printf("  [VIZ] png/01_parking_lot.png + svg/01_parking_lot.svg updated.\n");
}

/* ============================================================
 *  DIAGRAM 2: QUEUE (WAITING LIST) -- LINKED LIST
 * ============================================================
 *
 * Shows the FIFO waiting queue as a linked-list chain.
 *
 * HOW TO READ:
 *
 *  FRONT                                        REAR
 *    |                                            |
 *    v                                            v
 *  +---------------------+    +---------------------+    +---------------------+
 *  | MH12AB1234|----->>| MH14XY567 |----->>| DL1CA0001 |----->> NULL
 *  | Rahul     |    | Priya     |    | Amit      |
 *  | CAR       |    | BIKE      |    | CAR       |
 *  +---------------------+    +---------------------+    +---------------------+
 *    Position 1       Position 2       Position 3
 *
 * Arrows show the "next" pointer of each WaitingNode struct.
 * FRONT = will be served first (FIFO).
 */
void viz_queue(void) {
    FILE* fp = fopen("dot/02_queue.dot", "w");
    if (!fp) { printf("  [VIZ ERROR] Cannot write dot/02_queue.dot\n"); return; }

    fprintf(fp, "digraph Queue {\n");
    fprintf(fp, "    graph [label=\"WAITING QUEUE (FIFO -- First Come First Served)\\n"
                "Vehicles waiting for a free slot\","
                " fontsize=16, fontname=\"Helvetica\", bgcolor=\"#f8f9fa\", pad=0.5];\n");
    fprintf(fp, "    node  [fontname=\"Helvetica\", fontsize=12];\n");
    fprintf(fp, "    rankdir=LR;\n"); /* Left to right -- natural queue direction */

    if (queue_is_empty()) {
        /*
         * Empty queue: show a single explanatory node.
         * This helps the teacher see the empty state clearly.
         */
        fprintf(fp, "    Empty [shape=rectangle, style=filled,"
                " fillcolor=\"#bdc3c7\", fontcolor=\"white\","
                " label=\"Queue is EMPTY\\nNo vehicles waiting\"];\n");
        fprintf(fp, "    FRONT [shape=ellipse, style=filled, fillcolor=\"#3498db\","
                " fontcolor=white, label=\"FRONT\\n(NULL)\"];\n");
        fprintf(fp, "    REAR  [shape=ellipse, style=filled, fillcolor=\"#9b59b6\","
                " fontcolor=white, label=\"REAR\\n(NULL)\"];\n");
        fprintf(fp, "    FRONT -> Empty [label=\"points to\"];\n");
        fprintf(fp, "    REAR  -> Empty [label=\"points to\"];\n");
    } else {
        /* Draw FRONT and REAR labels */
        fprintf(fp, "    FRONT [shape=ellipse, style=filled, fillcolor=\"#3498db\","
                " fontcolor=white, label=\"FRONT\\n(dequeue here)\"];\n");
        fprintf(fp, "    REAR  [shape=ellipse, style=filled, fillcolor=\"#9b59b6\","
                " fontcolor=white, label=\"REAR\\n(enqueue here)\"];\n");

        WaitingNode* curr = waiting_queue.front;
        int pos = 1;
        char prev_id[32] = "";

        while (curr) {
            char node_id[32];
            snprintf(node_id, sizeof(node_id), "W%d", pos);

            const char* type_str = (curr->vehicle.type == TYPE_BIKE) ? "BIKE" : "CAR";

            /*
             * Each waiting node is a record-shaped box showing:
             *   - Position in queue
             *   - Plate number
             *   - Owner name
             *   - Vehicle type
             */
            fprintf(fp,
                "    %s [shape=record, style=filled, fillcolor=\"#3498db\","
                " fontcolor=\"white\","
                " label=\"{Pos %d | %s | %s | %s}\"];\n",
                node_id, pos,
                curr->vehicle.plate,
                curr->vehicle.owner,
                type_str);

            if (pos == 1) {
                /* Connect FRONT pointer to first node */
                fprintf(fp, "    FRONT -> %s [label=\"->\"];\n", node_id);
            } else {
                /* Arrow from previous node to this one (the "next" pointer) */
                fprintf(fp, "    %s -> %s [label=\"next\"];\n", prev_id, node_id);
            }

            strncpy(prev_id, node_id, sizeof(prev_id)-1);
            curr = curr->next;
            pos++;
        }

        /* Last node -> NULL */
        fprintf(fp, "    NULL_Q [shape=rectangle, style=filled,"
                " fillcolor=\"#95a5a6\", fontcolor=white, label=\"NULL\"];\n");
        fprintf(fp, "    %s -> NULL_Q [label=\"next\"];\n", prev_id);

        /* REAR points to last real node */
        fprintf(fp, "    REAR -> %s [label=\"->\", style=dashed, color=\"#9b59b6\"];\n", prev_id);
    }

    /* Position counter label */
    fprintf(fp, "    Count [shape=plaintext,"
            " label=\"Total Waiting: %d / %d\"];\n",
            waiting_queue.count, QUEUE_CAPACITY);

    fprintf(fp, "}\n");
    fclose(fp);

    viz_run_dot("dot/02_queue.dot", "png/02_queue.png", "svg/02_queue.svg");
    printf("  [VIZ] png/02_queue.png + svg/02_queue.svg updated.\n");
}

/* ============================================================
 *  DIAGRAM 3: STACK (LOG HISTORY) -- LIFO
 * ============================================================
 *
 * Shows the event log stack from TOP (most recent) to BOTTOM.
 *
 * HOW TO READ:
 *
 *         <- TOP (most recent event)
 *   +-----------------------------------------------------------------+
 *   |  EXIT | Slot 6 | MH12AB1234    |  <- log_stack.entries[top]
 *   +-----------------------------------------------------------------+
 *   |  ENTRY | Slot 6 | MH12AB1234   |
 *   +-----------------------------------------------------------------+
 *   |  ENTRY | Slot 1 | MH14XY5678   |
 *   +-----------------------------------------------------------------+
 *         <- BOTTOM (oldest event)
 *
 * New events are PUSHED on top.
 * Reading the log shows most recent first (top-to-bottom).
 */
void viz_stack(void) {
    FILE* fp = fopen("dot/03_stack.dot", "w");
    if (!fp) { printf("  [VIZ ERROR] Cannot write dot/03_stack.dot\n"); return; }

    fprintf(fp, "digraph Stack {\n");
    fprintf(fp, "    graph [label=\"LOG STACK (LIFO -- Last In First Out)\\n"
                "Entry/Exit history -- most recent at TOP\","
                " fontsize=16, fontname=\"Helvetica\", bgcolor=\"#f8f9fa\", pad=0.5];\n");
    fprintf(fp, "    node  [fontname=\"Helvetica\", fontsize=11];\n");
    fprintf(fp, "    rankdir=TB;\n"); /* Top-to-Bottom -- stack grows downward visually */

    if (stack_is_empty()) {
        fprintf(fp, "    Empty [shape=rectangle, style=filled,"
                " fillcolor=\"#bdc3c7\", fontcolor=white,"
                " label=\"Stack is EMPTY\\nNo events logged yet\"];\n");
        fprintf(fp, "    TOP [shape=ellipse, style=filled, fillcolor=\"#e67e22\","
                " fontcolor=white, label=\"TOP = -1\\n(empty)\"];\n");
        fprintf(fp, "    TOP -> Empty [style=dashed];\n");
    } else {
        /* TOP pointer label */
        fprintf(fp, "    TOP [shape=ellipse, style=filled, fillcolor=\"#e67e22\","
                " fontcolor=white, label=\"TOP (index=%d)\\n<- PUSH/POP here\"];\n",
                log_stack.top);

        char prev_id[32] = "TOP";

        /*
         * Iterate from top to bottom of the stack.
         * top is the index of most recent entry.
         * We go downward: top, top-1, top-2, ...
         */
        for (int i = log_stack.top; i >= 0; i--) {
            LogEntry* e = &log_stack.entries[i];
            char node_id[32];
            snprintf(node_id, sizeof(node_id), "S%d", i);

            const char* event_str = (e->event_type == EVENT_ENTRY) ? "ENTRY" : "EXIT";
            const char* type_str  = (e->vehicle_type == TYPE_BIKE) ? "BIKE" : "CAR";

            /* Entry events = GREEN, Exit events = RED */
            const char* color = (e->event_type == EVENT_ENTRY) ? "#2ecc71" : "#e74c3c";

            /*
             * Stack frame box format:
             *   Event type | Slot | Plate | Type | Fee | Time
             */
            fprintf(fp,
                "    %s [shape=record, style=filled, fillcolor=\"%s\","
                " fontcolor=white,"
                " label=\"{%s [index=%d] | Slot %d | %s | %s | Fee: Rs.%.0f | %s}\"];\n",
                node_id, color,
                event_str, i,
                e->slot_id, e->plate, type_str,
                e->fee_charged,
                format_time(e->timestamp));

            /* Arrow from previous node downward */
            if (i == log_stack.top) {
                fprintf(fp, "    TOP -> %s [label=\"top points here\"];\n", node_id);
            } else {
                fprintf(fp, "    %s -> %s [label=\"below\"];\n", prev_id, node_id);
            }

            strncpy(prev_id, node_id, sizeof(prev_id)-1);
        }

        /* Bottom of stack marker */
        fprintf(fp, "    BOTTOM [shape=rectangle, style=filled,"
                " fillcolor=\"#2c3e50\", fontcolor=white,"
                " label=\"BOTTOM (index=0)\"];\n");
        fprintf(fp, "    %s -> BOTTOM [label=\"oldest\", style=dashed];\n", prev_id);
    }

    fprintf(fp, "    Cap [shape=plaintext,"
            " label=\"Capacity: %d / %d entries used\"];\n",
            log_stack.top + 1, STACK_CAPACITY);

    fprintf(fp, "}\n");
    fclose(fp);

    viz_run_dot("dot/03_stack.dot", "png/03_stack.png", "svg/03_stack.svg");
    printf("  [VIZ] png/03_stack.png + svg/03_stack.svg updated.\n");
}

/* ============================================================
 *  DIAGRAM 4: HASH MAP (PLATE -> SLOT LOOKUP TABLE)
 * ============================================================
 *
 * Shows all 20 hash buckets and their chained linked lists.
 *
 * HOW TO READ:
 *
 *  Bucket 0 -> [NULL]
 *  Bucket 1 -> [MH12AB1234 -> Slot 6] -> [MH99ZZ0001 -> Slot 3] -> NULL
 *  Bucket 2 -> [NULL]
 *  Bucket 3 -> [MH14XY5678 -> Slot 1] -> NULL
 *  ...
 *
 *  Two plates can hash to the same bucket = COLLISION.
 *  They are stored as a chain (linked list) in that bucket.
 *  The diagram makes this collision clearly visible!
 *
 * WHY THIS MATTERS:
 *  When we search for "MH12AB1234":
 *    1. hash("MH12AB1234") = 1  -> go to bucket 1
 *    2. Walk the chain -> found! return slot_id = 6
 *    O(1) average time -- much faster than scanning all slots.
 */
void viz_hashmap(void) {
    FILE* fp = fopen("dot/04_hashmap.dot", "w");
    if (!fp) { printf("  [VIZ ERROR] Cannot write dot/04_hashmap.dot\n"); return; }

    fprintf(fp, "digraph HashMap {\n");
    fprintf(fp, "    graph [label=\"HASH MAP -- License Plate -> Slot ID\\n"
                "(djb2 hash function | %d buckets | Chaining for collisions)\","
                " fontsize=16, fontname=\"Helvetica\","
                " bgcolor=\"#f8f9fa\", pad=0.5];\n", HASH_SIZE);
    fprintf(fp, "    node  [fontname=\"Helvetica\", fontsize=11];\n");
    fprintf(fp, "    rankdir=LR;\n");

    /* Draw all HASH_SIZE bucket labels in a vertical column */
    fprintf(fp, "    subgraph cluster_buckets {\n");
    fprintf(fp, "        label=\"Hash Buckets\";\n");
    fprintf(fp, "        style=filled; fillcolor=\"#ecf0f1\";\n");

    for (int i = 0; i < HASH_SIZE; i++) {
        /*
         * Each bucket is a PURPLE node showing its index.
         * If occupied: darker. If empty: lighter.
         */
        const char* bcolor = vehicle_map.buckets[i] ? "#9b59b6" : "#bdc3c7";
        fprintf(fp,
            "        B%d [shape=record, style=filled, fillcolor=\"%s\","
            " fontcolor=white, label=\"bucket[%d]\"];\n",
            i, bcolor, i);
    }
    /* Make buckets appear in a vertical column */
    for (int i = 0; i < HASH_SIZE - 1; i++)
        fprintf(fp, "        B%d -> B%d [style=invis];\n", i, i+1);
    fprintf(fp, "    }\n");

    /* Draw chained nodes for each non-empty bucket */
    for (int i = 0; i < HASH_SIZE; i++) {
        HashNode* curr = vehicle_map.buckets[i];
        if (!curr) {
            /* Empty bucket -> NULL node */
            fprintf(fp,
                "    NULL%d [shape=rectangle, style=filled,"
                " fillcolor=\"#95a5a6\", fontcolor=white, label=\"NULL\"];\n", i);
            fprintf(fp, "    B%d -> NULL%d;\n", i, i);
            continue;
        }

        /* Draw each node in this bucket's chain */
        char prev_id[64];
        snprintf(prev_id, sizeof(prev_id), "B%d", i);
        int node_num = 0;

        while (curr) {
            char node_id[64];
            snprintf(node_id, sizeof(node_id), "H%d_%d", i, node_num);

            /*
             * YELLOW for first node, darker for subsequent ones
             * (helps spot collisions -- multiple yellows in same bucket).
             */
            const char* ncolor = (node_num == 0) ? "#f39c12" : "#e67e22";

            fprintf(fp,
                "    %s [shape=record, style=filled, fillcolor=\"%s\","
                " fontcolor=white,"
                " label=\"{%s | -> Slot %d}\"];\n",
                node_id, ncolor, curr->plate, curr->slot_id);

            fprintf(fp, "    %s -> %s", prev_id, node_id);
            if (node_num == 0)
                fprintf(fp, " [label=\"hash=%d\"]", i);
            fprintf(fp, ";\n");

            strncpy(prev_id, node_id, sizeof(prev_id)-1);
            curr = curr->next;
            node_num++;
        }

        /* Chain ends with NULL */
        char null_id[32];
        snprintf(null_id, sizeof(null_id), "NULLH%d", i);
        fprintf(fp,
            "    %s [shape=rectangle, style=filled,"
            " fillcolor=\"#95a5a6\", fontcolor=white, label=\"NULL\"];\n", null_id);
        fprintf(fp, "    %s -> %s [label=\"next\"];\n", prev_id, null_id);
    }

    /* Collision warning note */
    fprintf(fp, "    ColNote [shape=plaintext,"
            " label=\"ORANGE nodes = collision in same bucket\\n"
            "Walk the chain to find the right plate\"];\n");

    fprintf(fp, "}\n");
    fclose(fp);

    viz_run_dot("dot/04_hashmap.dot", "png/04_hashmap.png", "svg/04_hashmap.svg");
    printf("  [VIZ] png/04_hashmap.png + svg/04_hashmap.svg updated.\n");
}

/* ============================================================
 *  DIAGRAM 5: CODE FLOW DIAGRAM (START -> CURRENT STATE)
 * ============================================================
 *
 * Shows the complete execution flow of the entire program,
 * from startup to the current operation being performed.
 * The CURRENT step is highlighted in TEAL.
 *
 * HOW TO READ:
 *   Each box = one stage of the program.
 *   Arrows = program moves from one stage to next.
 *   TEAL box = where the program is RIGHT NOW.
 *   Diamond = decision point (yes/no branch).
 *
 * This helps a student/teacher see:
 *   "The code ran init -> showed menu -> user chose PARK ->
 *    found a free slot -> parked -> updated DS -> generated diagrams"
 */
void viz_flow(int current_step) {
    FILE* fp = fopen("dot/05_flow.dot", "w");
    if (!fp) { printf("  [VIZ ERROR] Cannot write dot/05_flow.dot\n"); return; }

    fprintf(fp, "digraph Flow {\n");
    fprintf(fp, "    graph [label=\"PROGRAM EXECUTION FLOW\\n"
                "TEAL = current step | Follow arrows to trace execution\","
                " fontsize=16, fontname=\"Helvetica\","
                " bgcolor=\"#f8f9fa\", pad=0.8];\n");
    fprintf(fp, "    node  [fontname=\"Helvetica\", fontsize=12, style=filled];\n");
    fprintf(fp, "    rankdir=TB;\n");

    /*
     * Helper macro pattern: for each node, pick color based on
     * whether it is the current step (TEAL) or not (grey/default).
     */
#define NODE_COLOR(step_id) \
    (current_step == (step_id) ? "#1abc9c\" fontcolor=\"white" : "#ecf0f1\" fontcolor=\"#2c3e50")

    /* ---- Step 0: Program Start ---- */
    fprintf(fp, "    Start [shape=oval, fillcolor=\"%s\","
            " label=\"PROGRAM START\\nmain() called\"];\n",
            NODE_COLOR(0));

    /* ---- Step 1: Init ---- */
    fprintf(fp, "    Init [shape=rectangle, fillcolor=\"%s\","
            " label=\"SYSTEM INIT\\ninit_slots() -- Array of 10 slots\\n"
            "init_hashmap() -- 20 empty buckets\\n"
            "init_stack()  -- top = -1\\n"
            "init_queue()  -- front = rear = NULL\"];\n",
            NODE_COLOR(1));

    /* ---- Step 2: Menu ---- */
    fprintf(fp, "    Menu [shape=rectangle, fillcolor=\"%s\","
            " label=\"SHOW MENU\\nWait for user input (1-6 or 0)\"];\n",
            NODE_COLOR(2));

    /* ---- Decision Diamond ---- */
    fprintf(fp, "    Decision [shape=diamond, fillcolor=\"#f39c12\","
            " fontcolor=white,"
            " label=\"User Choice?\"];\n");

    /* ---- Step 3: Park ---- */
    fprintf(fp, "    Park [shape=rectangle, fillcolor=\"%s\","
            " label=\"PARK VEHICLE\\n"
            "1. Read plate + owner + type\\n"
            "2. Check duplicate -> hashmap_search()\\n"
            "3. find_free_slot() -> scan array\\n"
            "   If slot found: mark OCCUPIED\\n"
            "   hashmap_insert(plate, slot_id)\\n"
            "   stack_push(ENTRY log)\\n"
            "   If no slot: queue_enqueue()\"];\n",
            NODE_COLOR(3));

    /* ---- Step 4: Remove ---- */
    fprintf(fp, "    Remove [shape=rectangle, fillcolor=\"%s\","
            " label=\"REMOVE VEHICLE\\n"
            "1. Read plate\\n"
            "2. hashmap_search() -> O(1) find slot\\n"
            "3. calculate_fee() -> duration * rate\\n"
            "4. Mark slot FREE\\n"
            "5. hashmap_delete(plate)\\n"
            "6. stack_push(EXIT log)\\n"
            "7. Check queue -> auto-assign\"];\n",
            NODE_COLOR(4));

    /* ---- Step 5: Search ---- */
    fprintf(fp, "    Search [shape=rectangle, fillcolor=\"%s\","
            " label=\"SEARCH VEHICLE\\n"
            "1. Read plate\\n"
            "2. hashmap_search() -> O(1) lookup\\n"
            "3. If not found -> scan queue\\n"
            "4. Show details + accrued fee\"];\n",
            NODE_COLOR(5));

    /* ---- Step 6: View Lot ---- */
    fprintf(fp, "    ViewLot [shape=rectangle, fillcolor=\"%s\","
            " label=\"VIEW PARKING LOT\\n"
            "Scan array[0..9]\\n"
            "Print slot_id, type, status,\\n"
            "plate, owner for each slot\"];\n",
            NODE_COLOR(6));

    /* ---- Step 7: View Queue ---- */
    fprintf(fp, "    ViewQueue [shape=rectangle, fillcolor=\"%s\","
            " label=\"VIEW WAITING LIST\\n"
            "Traverse queue linked list\\n"
            "front -> ... -> rear\\n"
            "Print position + vehicle info\"];\n",
            NODE_COLOR(7));

    /* ---- Step 8: View Log ---- */
    fprintf(fp, "    ViewLog [shape=rectangle, fillcolor=\"%s\","
            " label=\"VIEW LOG\\n"
            "Read stack top-to-bottom\\n"
            "entries[top] -> entries[0]\\n"
            "Most recent event shown first\"];\n",
            NODE_COLOR(8));

    /* ---- Step 9: Exit ---- */
    fprintf(fp, "    Exit [shape=oval, fillcolor=\"%s\","
            " label=\"EXIT\\nFree queue nodes (malloc)\\n"
            "Free hashmap nodes (malloc)\\n"
            "return 0\"];\n",
            NODE_COLOR(9));

    /* ---- Viz sub-step (always shown, not a user step) ---- */
    fprintf(fp, "    Viz [shape=parallelogram, style=filled,"
            " fillcolor=\"#8e44ad\", fontcolor=white,"
            " label=\"GENERATE DIAGRAMS\\n"
            "viz_parking_lot()\\n"
            "viz_queue()\\n"
            "viz_stack()\\n"
            "viz_hashmap()\\n"
            "viz_flow()\"];\n");

    /* ---- EDGES (arrows showing flow) ---- */
    fprintf(fp, "    Start    -> Init     [label=\"program begins\"];\n");
    fprintf(fp, "    Init     -> Menu     [label=\"all DS ready\"];\n");
    fprintf(fp, "    Menu     -> Decision [label=\"reads choice\"];\n");

    fprintf(fp, "    Decision -> Park      [label=\"choice=1\"];\n");
    fprintf(fp, "    Decision -> Remove    [label=\"choice=2\"];\n");
    fprintf(fp, "    Decision -> Search    [label=\"choice=3\"];\n");
    fprintf(fp, "    Decision -> ViewLot   [label=\"choice=4\"];\n");
    fprintf(fp, "    Decision -> ViewQueue [label=\"choice=5\"];\n");
    fprintf(fp, "    Decision -> ViewLog   [label=\"choice=6\"];\n");
    fprintf(fp, "    Decision -> Exit      [label=\"choice=0\"];\n");

    /* Each operation leads to diagram generation, then back to menu */
    fprintf(fp, "    Park      -> Viz [label=\"DS updated\"];\n");
    fprintf(fp, "    Remove    -> Viz [label=\"DS updated\"];\n");
    fprintf(fp, "    Search    -> Viz [label=\"DS read\"];\n");
    fprintf(fp, "    ViewLot   -> Viz [label=\"DS read\"];\n");
    fprintf(fp, "    ViewQueue -> Viz [label=\"DS read\"];\n");
    fprintf(fp, "    ViewLog   -> Viz [label=\"DS read\"];\n");

    fprintf(fp, "    Viz -> Menu [label=\"loop back\", style=dashed, color=\"#8e44ad\"];\n");
    fprintf(fp, "    Exit -> Exit [label=\"program ends\", style=invis];\n");

    fprintf(fp, "}\n");
    fclose(fp);

    viz_run_dot("dot/05_flow.dot", "png/05_flow.png", "svg/05_flow.svg");
    printf("  [VIZ] png/05_flow.png + svg/05_flow.svg updated. (TEAL = current step)\n");

#undef NODE_COLOR
}

/*
 * ============================================================
 * END OF FILE
 * ============================================================
 *
 * FILES GENERATED ON EACH OPERATION:
 * ------------------------------------------------------------------------
 *  dot/01_parking_lot.dot  png/01_parking_lot.png  svg/01_parking_lot.svg  -> Slot grid
 *  dot/02_queue.dot        png/02_queue.png        svg/02_queue.svg        -> Waiting list
 *  dot/03_stack.dot        png/03_stack.png        svg/03_stack.svg        -> Log stack LIFO
 *  dot/04_hashmap.dot      png/04_hashmap.png      svg/04_hashmap.svg      -> Hash map
 *  dot/05_flow.dot         png/05_flow.png         svg/05_flow.svg         -> Full code flow
 *
 * OPEN svg/ files in any browser for best quality.
 * OPEN png/ files in any image viewer.
 * Numbers (01-05) show the recommended reading order.
 *
 * ============================================================
 */