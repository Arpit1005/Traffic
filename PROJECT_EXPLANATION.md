# TrafficGuru: Complete Project Explanation

## Table of Contents
1. [Project Overview](#project-overview)
2. [Theoretical Foundations](#theoretical-foundations)
3. [System Architecture](#system-architecture)
4. [Core Components](#core-components)
5. [Implementation Details](#implementation-details)
6. [Scheduling Algorithms](#scheduling-algorithms)
7. [Deadlock Prevention](#deadlock-prevention)
8. [Synchronization Mechanisms](#synchronization-mechanisms)
9. [Performance Metrics](#performance-metrics)
10. [Build & Execution](#build--execution)

---

## Project Overview

**TrafficGuru** is an intelligent traffic intersection management system that applies **Operating Systems (OS) concepts** to optimize vehicle flow at a 4-way traffic intersection. It's a sophisticated simulator that bridges real-world traffic engineering with computer science principles.

### Key Innovation
Rather than treating traffic as a simple queue management problem, TrafficGuru models:
- **Vehicles as OS processes**
- **Lanes as process queues**
- **Traffic lights as mutual exclusion locks**
- **Intersection quadrants as shared resources**
- **Vehicle crossings as critical sections**

This allows the system to apply proven OS techniques:
- Multiple scheduling algorithms (SJF, Multilevel Feedback, Priority RR)
- Deadlock prevention (Banker's Algorithm)
- Synchronization primitives (mutexes, condition variables)
- Emergency preemption (priority inheritance)

---

## Theoretical Foundations

### 1. Operating Systems Scheduling Theory

Traffic signal scheduling is analogous to **CPU scheduling**:

| Aspect | OS Context | Traffic Context |
|--------|-----------|-----------------|
| **Processes** | Running programs | Vehicle lanes |
| **Time Quantum** | CPU time slice | Green light duration |
| **Context Switch** | Switch between processes | Change active lane |
| **Ready Queue** | Waiting processes | Lanes with vehicles |
| **Scheduling Goal** | Minimize wait time, maximize throughput | Minimize vehicle wait, maximize intersection utilization |

### 2. Scheduling Metrics

**Throughput**: Number of vehicles crossing per unit time
```
Throughput = Total Vehicles Processed / Total Time
```

**Average Waiting Time**: Mean time vehicles wait for green light
```
Avg Wait = Σ(Vehicle Wait Times) / Total Vehicles
```

**Fairness Index (Jain's Index)**: Ensures no lane is starved
```
Fairness = (Σ xi)² / (n × Σ xi²)
Range: 1/n (unfair) to 1 (perfectly fair)
```

**Context Switch Overhead**: Cost of changing which lane has green light
```
Overhead = Number of Switches × Time per Switch
```

### 3. Deadlock Theory

In traffic, a **deadlock** occurs when:
```
Lane A needs Quadrant 1 (held by Lane B)
Lane B needs Quadrant 2 (held by Lane A)
→ Circular wait → No progress
```

**Four Necessary and Sufficient Conditions for Deadlock**:
1. **Mutual Exclusion**: Only one lane can cross at a time
2. **Hold and Wait**: Lane holds resources while waiting for others
3. **No Preemption**: Resources can't be taken forcibly
4. **Circular Wait**: Cycle of lanes waiting for each other

To prevent deadlock, break at least one condition.

### 4. Resource Allocation Problem

**Banker's Algorithm** (Dijkstra, 1965):
- Prevents deadlock by only allowing **safe allocations**
- A state is **safe** if there exists a sequence where all processes can complete
- Before allocating: check if resulting state is still safe
- If unsafe: deny request and wait

---

## System Architecture

```
┌─────────────────────────────────────────┐
│         TrafficGuruSystem               │
│  ┌───────────────────────────────────┐  │
│  │   Lane Processes (4 lanes)        │  │
│  │   ┌──────┬──────┬──────┬──────┐   │  │
│  │   │North │South │East  │West  │   │  │
│  │   └──────┴──────┴──────┴──────┘   │  │
│  └───────────────────────────────────┘  │
│                   ↓                      │
│  ┌───────────────────────────────────┐  │
│  │    Central Scheduler              │  │
│  │  ┌─────────────────────────────┐  │  │
│  │  │ Select Algorithm:           │  │  │
│  │  │ • SJF                       │  │  │
│  │  │ • Multilevel Feedback       │  │  │
│  │  │ • Priority Round Robin      │  │  │
│  │  └─────────────────────────────┘  │  │
│  └───────────────────────────────────┘  │
│                   ↓                      │
│  ┌───────────────────────────────────┐  │
│  │  Synchronization Layer            │  │
│  │  ┌─────────────────────────────┐  │  │
│  │  │ Intersection Mutex          │  │  │
│  │  │ Condition Variables         │  │  │
│  │  │ Deadlock Detection          │  │  │
│  │  └─────────────────────────────┘  │  │
│  └───────────────────────────────────┘  │
│                   ↓                      │
│  ┌───────────────────────────────────┐  │
│  │  Banker's Algorithm               │  │
│  │  ┌─────────────────────────────┐  │  │
│  │  │ Quadrant Allocation         │  │  │
│  │  │ Safety Checks               │  │  │
│  │  │ Deadlock Prevention         │  │  │
│  │  └─────────────────────────────┘  │  │
│  └───────────────────────────────────┘  │
│                   ↓                      │
│  ┌───────────────────────────────────┐  │
│  │  Emergency System                 │  │
│  │  ┌─────────────────────────────┐  │  │
│  │  │ Priority Preemption         │  │  │
│  │  │ Vehicle Type Detection      │  │  │
│  │  │ Response Time Tracking      │  │  │
│  │  └─────────────────────────────┘  │  │
│  └───────────────────────────────────┘  │
│                   ↓                      │
│  ┌───────────────────────────────────┐  │
│  │  Performance Metrics & Viz        │  │
│  │  ┌─────────────────────────────┐  │  │
│  │  │ Real-time ncurses UI        │  │  │
│  │  │ Gantt Charts                │  │  │
│  │  │ Performance Dashboard       │  │  │
│  │  └─────────────────────────────┘  │  │
│  └───────────────────────────────────┘  │
└─────────────────────────────────────────┘
```

---

## Core Components

### 1. Lane Process (`lane_process.h/c`)

**Purpose**: Represents a single traffic lane (North, South, East, West)

**Key Data Structure**:
```c
typedef struct {
    int lane_id;                    // Unique identifier (0-3)
    Queue* queue;                   // Vehicle queue
    LaneState state;                // WAITING, READY, RUNNING, BLOCKED
    int priority;                   // For priority scheduling
    int waiting_time;               // Total wait time
    pthread_mutex_t queue_lock;     // Thread-safe queue access
    pthread_cond_t queue_cond;      // Signal when green light activated
    int total_vehicles_served;      // Throughput metric
    int requested_quadrants;        // Resources needed
    int allocated_quadrants;        // Resources held
} LaneProcess;
```

**Lane States**:
- `WAITING`: No vehicles, lane inactive
- `READY`: Vehicles present, waiting for green light
- `RUNNING`: Active, vehicles crossing intersection
- `BLOCKED`: Blocked by Banker's algorithm for safety

**Key Operations**:
```c
add_vehicle_to_lane()           // Queue new vehicle
remove_vehicle_from_lane()      // Process vehicle through intersection
get_lane_queue_length()         // Current vehicles waiting
update_lane_metrics()           // Update statistics
request_intersection_quadrants() // Resource request
release_intersection_quadrants() // Resource release
```

### 2. Queue (`queue.h/c`)

**Purpose**: FIFO queue for vehicles in a lane

**Implementation**: Circular array for O(1) enqueue/dequeue

**Key Features**:
- Dynamic resizing when full
- Overflow tracking
- Utilization statistics
- Statistics: enqueue count, dequeue count, overflow count

```c
typedef struct {
    int* vehicles;          // Array of vehicle IDs
    int front, rear, size;  // Pointers and size tracking
    int capacity;           // Current capacity
    int enqueue_count;      // Total enqueues
    int dequeue_count;      // Total dequeues
    int overflow_count;     // Times capacity exceeded
} Queue;
```

### 3. Scheduler (`scheduler.h/c`)

**Purpose**: Central traffic signal controller

**Key Data Structure**:
```c
typedef struct {
    SchedulingAlgorithm algorithm;  // Current algorithm
    Queue* ready_queue;             // Lanes ready for green
    int time_quantum;               // Green light duration (seconds)
    int context_switch_time;        // Time to change signals
    int current_lane;               // Lane with green light
    ExecutionRecord* execution_history; // Historical data
    bool scheduler_running;         // Status
    pthread_mutex_t scheduler_lock; // Thread safety
} Scheduler;
```

**Core Function**:
```c
int schedule_next_lane(Scheduler* scheduler, LaneProcess lanes[4])
// Returns: which lane (0-3) gets green light next
// Calls algorithm-specific scheduler (SJF, Multilevel, or Priority RR)
```

### 4. Synchronization (`synchronization.h/c`)

**Purpose**: Manage intersection access atomically

**Key Structure**:
```c
typedef struct {
    pthread_mutex_t intersection_lock;   // Main intersection lock
    pthread_cond_t condition_vars[4];   // One per lane
    int current_lane;                   // Who has green
    int active_quadrants;               // Resources in use
} IntersectionMutex;
```

**Critical Sections**:
- Only one lane's vehicles can cross at a time
- Vehicles from same lane cross atomically (no interleaving)
- Other lanes wait on condition variables

**Deadlock Detection**:
```c
bool detect_deadlock(LaneProcess lanes[4])
// Checks for circular wait in resource dependencies
bool is_circular_wait_detected(LaneProcess lanes[4])
// Validates deadlock conditions
```

### 5. Banker's Algorithm (`bankers_algorithm.h/c`)

**Purpose**: Prevent traffic gridlock via safe resource allocation

**Resource Model**:
- **Resources**: 4 intersection quadrants (NE, NW, SW, SE)
- **Processes**: 4 lanes (North, South, East, West)
- **Allocation**: Before lane crosses, check safety

**State Tracking**:
```c
typedef struct {
    int available[NUM_QUADRANTS];           // Free quadrants
    int maximum[NUM_LANES][NUM_QUADRANTS];  // Max claim per lane
    int allocation[NUM_LANES][NUM_QUADRANTS]; // Currently held
    int need[NUM_LANES][NUM_QUADRANTS];     // Still needed
} BankersState;
```

**Safety Algorithm** (simplified):
```
while (unfinished lanes exist) {
    find lane L where need[L] <= available {
        // Lane can complete with current resources
        mark L as finished
        available += allocation[L]  // Release resources
    }
    if (no such lane exists) {
        UNSAFE STATE → Deny allocation
    }
}
if (all lanes finished) {
    SAFE STATE → Allow allocation
}
```

### 6. Emergency System (`emergency_system.h/c`)

**Purpose**: Priority handling of emergency vehicles

**Emergency Types**:
```c
enum {
    EMERGENCY_AMBULANCE,    // Priority: 1 (highest)
    EMERGENCY_FIRE_TRUCK,   // Priority: 2
    EMERGENCY_POLICE        // Priority: 3
}
```

**Operation**:
1. Detect emergency vehicle entering lane
2. Set lane priority to highest
3. Scheduler grants immediate green light
4. Clear intersection quadrants for emergency
5. Return to normal scheduling after passage

### 7. Performance Metrics (`performance_metrics.h`)

**Tracked Metrics**:
- **Throughput**: Vehicles/minute processed
- **Average Wait Time**: Mean waiting time
- **Fairness Index**: Jain's fairness measure
- **Context Switches**: Number of light changes
- **Emergency Response Time**: Time to clear emergency
- **Deadlock Preventions**: Unsafe states blocked

---

## Implementation Details

### Execution Flow

**1. Initialization Phase** (`init_traffic_guru_system()`)
```c
1. Allocate TrafficGuruSystem structure
2. Initialize 4 lane processes
3. Create scheduler with selected algorithm
4. Initialize intersection mutex
5. Initialize Banker's algorithm state
6. Start ncurses visualization
7. Initialize performance metrics
```

**2. Simulation Start** (`start_traffic_simulation()`)
```c
1. Start scheduler thread
2. Create simulation main loop thread
3. Create vehicle generator thread
4. Set simulation_running = true
```

**3. Main Simulation Loop** (`simulation_main_loop()`)
```
while (simulation_running && keep_running) {
    if (not paused) {
        update_simulation_state()          // Update metrics, check deadlocks
        process_traffic_events()           // Schedule & execute
    }
    usleep(300ms)                          // Control speed
}
```

**4. Traffic Event Processing**
```c
process_traffic_events() {
    next_lane = schedule_next_lane()       // Pick lane by algorithm
    if (next_lane != -1) {
        execute_lane_time_slice(next_lane) // Give it green light
    }
}
```

**5. Lane Execution**
```c
execute_lane_time_slice(lane) {
    acquire_intersection(lane)              // Get mutex
    
    // Process BATCH_EXIT_SIZE vehicles
    for (int i = 0; i < 3; i++) {
        if (!is_queue_empty(lane)) {
            remove_vehicle_from_lane(lane)  // Process through intersection
            sleep(VEHICLE_CROSS_TIME)       // Simulate crossing
        }
    }
    
    release_intersection(lane)              // Release mutex
}
```

### Multi-threading Model

**Thread Types**:

1. **Main Thread**
   - Handles ncurses UI
   - User input processing
   - Display updates
   - Signal handling

2. **Simulation Thread**
   - Main loop: schedule and execute lanes
   - Deadlock detection
   - Metric updates

3. **Vehicle Generator Thread**
   - Generates random vehicles
   - Random lane selection
   - Random emergency generation
   - Sleeps between generations

**Synchronization Points**:
```
Main Thread
    ↓ (display)
Simulation Thread ←→ Vehicle Generator Thread
    ↓ (resource access)
Global State Lock (metrics, emergencies)
    ↓
Lane Locks (queue operations)
    ↓
Intersection Mutex (critical section)
    ↓
Banker's Algorithm (safe allocation check)
```

### Critical Sections

**Intersection Access** (Most Protected):
```c
// Only ONE lane can have active vehicles at a time
acquire_intersection(lane);         // Lock
{
    for (int i = 0; i < BATCH_EXIT_SIZE; i++) {
        process_vehicle(lane);      // Safe
    }
}
release_intersection(lane);         // Unlock
```

**Queue Modifications**:
```c
pthread_mutex_lock(&lane->queue_lock);
{
    if (!queue_full) {
        enqueue(lane->queue, vehicle_id);
        lane->state = READY;
    }
}
pthread_mutex_unlock(&lane->queue_lock);
```

**Metrics Update**:
```c
pthread_mutex_lock(&global_state_lock);
{
    update_time_based_metrics();
    update_emergency_progress();
}
pthread_mutex_unlock(&global_state_lock);
```

---

## Scheduling Algorithms

### 1. Shortest Job First (SJF)

**Theory**:
- Prioritize lanes with **shortest estimated service time**
- Minimizes average waiting time (optimal non-preemptive)
- Susceptible to **starvation** (long jobs always delayed)

**Implementation**:
```c
int schedule_next_lane_sjf(Scheduler* scheduler, LaneProcess lanes[4]) {
    int best_lane = -1;
    int shortest_time = INT_MAX;
    
    for (int i = 0; i < NUM_LANES; i++) {
        if (lanes[i].state == READY || lanes[i].state == RUNNING) {
            // Estimate: queue_length × vehicle_cross_time
            int estimated_time = lanes[i].queue_length * VEHICLE_CROSS_TIME;
            
            if (estimated_time < shortest_time) {
                shortest_time = estimated_time;
                best_lane = i;
            }
        }
    }
    
    return best_lane;
}
```

**Advantages**:
- ✓ Minimizes average wait time
- ✓ Simple to implement

**Disadvantages**:
- ✗ Starvation: long lanes wait indefinitely
- ✗ Unpredictable (depends on queue size estimate)
- ✗ No fairness guarantee

---

### 2. Multilevel Feedback Queue

**Theory**:
- Multiple **priority levels** with different time quantums
- Lanes move between levels based on **behavior**
- **Aging**: long-waiting lanes promoted to prevent starvation
- Adapts to lane characteristics dynamically

**Priority Levels**:
```
HIGH   (Priority 1): New lanes or short jobs
       → Time Quantum: 2 seconds
       
MEDIUM (Priority 2): Promoted from high
       → Time Quantum: 4 seconds
       
LOW    (Priority 3): Further demoted
       → Time Quantum: 6 seconds
```

**Promotion/Demotion Rules**:
1. **New lane**: Start at HIGH priority
2. **Uses full quantum**: Move DOWN one level
3. **Doesn't use full quantum**: Stay same level
4. **Waiting too long**: Move UP one level (aging)

**Implementation Concept**:
```
Lane characteristics determine priority:
- Queue length < 3:     HIGH (short job)
- Queue length 3-8:     MEDIUM
- Queue length > 8:     LOW (long job)

Age-based boost:
- Waiting > 30 seconds:  Promote one level

Time quantum by level:
- HIGH:   2 second green light
- MEDIUM: 4 second green light
- LOW:    6 second green light
```

**Advantages**:
- ✓ Prevents starvation (via aging)
- ✓ Adapts to lane behavior
- ✓ Good fairness and response time
- ✓ Flexible tuning

**Disadvantages**:
- ✗ More complex implementation
- ✗ Tuning parameters affect behavior

---

### 3. Priority Round Robin

**Theory**:
- Each lane has **static priority** (1-3)
- **Round robin** within same priority
- **Time quantum**: 3 seconds for all
- **Emergency preemption**: Emergency vehicles get priority 0 (highest)

**Priority Assignment**:
```
Priority 0: Emergency vehicles
Priority 1: High-demand lanes (queue > 3)
Priority 2: Normal lanes
Priority 3: Low-traffic lanes (queue ≤ 3)
```

**Scheduling Decision**:
```c
// Always check highest priority first
for (priority = 0; priority < 4; priority++) {
    for (each lane at this priority in round-robin order) {
        if (lane has vehicles) {
            return lane;
        }
    }
}
return -1;  // No lanes ready
```

**Advantages**:
- ✓ Predictable response time
- ✓ Emergency preemption
- ✓ Simple and fair
- ✓ Bounded wait time

**Disadvantages**:
- ✗ Fixed priority may not adapt
- ✗ Context switches if queue becomes empty

---

### Algorithm Comparison

| Metric | SJF | Multilevel | Priority RR |
|--------|-----|-----------|------------|
| **Avg Wait Time** | Best | Good | Fair |
| **Fairness** | Poor | Excellent | Good |
| **Starvation** | Yes | No | No |
| **Latency** | Unpredictable | Bounded | Bounded |
| **Complexity** | Low | Medium | Medium |
| **Emergency** | No | No | Yes |
| **Adaptability** | No | Yes | Limited |

---

## Deadlock Prevention

### Traffic Gridlock: When It Happens

```
        North Lane
            ↓↓↓ (wants SE quadrant)
   ┌─────────────────┐
   │     NE    NW    │
   │  ┌────┬────┐    │
E→ │  │    │    │ ←W  │
   │  │    │    │    │
   │  ├────┼────┤    │
   │  │    │    │    │
   │  └────┴────┘    │
   │     SE    SW    │
   └─────────────────┘
            ↑↑↑ (wants NE quadrant)
        South Lane

DEADLOCK:
- North needs SE (held by West)
- South needs NE (held by East)
- East needs SW (held by South)
- West needs NW (held by North)
→ Circular wait → No vehicle can proceed
```

### Banker's Algorithm Implementation

**Goal**: Ensure system never enters unsafe state

**Safety Checking Algorithm**:
```
Given:
- available[q] = free quadrants
- allocated[lane][q] = held quadrants per lane
- need[lane][q] = still-needed quadrants per lane

Safety Algorithm:
1. Mark all lanes as unfinished
2. find_safe_sequence:
   For each unfinished lane L:
      if (need[L][q] <= available[q]) for all q:
         // Lane can finish with current resources
         finished[L] = true
         available += allocated[L]
         
   if (any lane finished in step 2):
      go back to step 2
   else if (all lanes finished):
      SAFE STATE
   else:
      UNSAFE STATE
```

**Before Allocation Decision**:
```c
bool request_resources(lane, request) {
    // Check 1: Is request valid?
    if (request > need[lane]) reject;
    
    // Check 2: Are resources available?
    if (request > available) deny;
    
    // Check 3: Would allocation be safe?
    {
        // Tentatively allocate
        available -= request;
        allocation[lane] += request;
        need[lane] -= request;
        
        // Run safety check
        if (is_safe_state()) {
            return true;  // APPROVE allocation
        } else {
            // Undo tentative allocation
            available += request;
            allocation[lane] -= request;
            need[lane] += request;
            return false;  // DENY allocation
        }
    }
}
```

**Quadrant Allocation by Movement Type**:

```
Lane Straight  Left Turn  Right Turn  U-Turn
┌──────────────────────────────────────────┐
│ N    → SE   NE+SE    NW       NW+SW    │
│ S    → NW   NW+SW    NE       NE+SE    │
│ E    → NW   NW+NE    SW       SW+SE    │
│ W    → SE   SE+SW    NE       NE+NW    │
└──────────────────────────────────────────┘
```

---

## Synchronization Mechanisms

### Mutex: Mutual Exclusion

**Purpose**: Ensure only one lane crosses at a time

```c
// Acquire for exclusive access
pthread_mutex_lock(&intersection_lock);
{
    // Critical section
    execute_lane_time_slice(lane);
}
pthread_mutex_unlock(&intersection_lock);
```

**Properties**:
- **Atomic**: Acquire is indivisible
- **Exclusive**: Only one owner at a time
- **Blocking**: Waiters sleep until available

### Condition Variables: Lane Signaling

**Purpose**: Lanes sleep when they don't have green light

```c
// Lane waiting for green light
pthread_cond_wait(&condition_var[lane], &queue_lock);
// Woken when scheduler signals it's turn

// Scheduler signals ready lane
pthread_cond_signal(&condition_var[next_lane]);
```

**Why Needed**:
- Without it: lanes would busy-wait, wasting CPU
- With it: lanes sleep, woken by scheduler

### Priority Inheritance

**Problem**: Priority Inversion
```
High-priority lane blocked by low-priority lane
→ Violates real-time guarantees

Solution: Temporarily boost low-priority lane
until it releases the resource
```

### Deadlock Detection

**Real-time monitoring** in simulation:
```c
// Check every 100 iterations
detect_and_resolve_advanced_deadlock(lanes);

// Detects: circular wait + resource contention
// Resolves: preempt lower-priority lanes
```

---

## Performance Metrics

### 1. Throughput
```
= Total Vehicles Served / Total Time
Unit: vehicles/minute

High throughput = intersection efficiently using green time
Low throughput = congestion or inefficient scheduling
```

**Measurement**:
```c
int processed_count = 0;
for each lane:
    processed_count += lane->total_vehicles_served;
    
throughput = (processed_count / elapsed_time_minutes)
```

### 2. Average Waiting Time
```
= Σ(Each vehicle's wait) / Total Vehicles
Unit: seconds

Lower is better (vehicles want short waits)
```

**Real-time Tracking**:
```c
lane->total_waiting_time += lane->waiting_time;
avg_wait = lane->total_waiting_time / vehicles_served;
```

### 3. Fairness Index (Jain's Index)
```
J = (Σ throughput_i)² / (n × Σ throughput_i²)

Range: [1/n, 1]
1/n = worst (only 1 lane moves)
1.0 = perfect (all equal)
```

**Example** with 4 lanes:
```
Throughputs: [5, 5, 5, 5] → J = 1.0 (perfect)
Throughputs: [8, 8, 8, 0] → J = 0.75 (acceptable)
Throughputs: [20, 0, 0, 0] → J = 0.25 (unfair)
```

### 4. Context Switch Overhead
```
= Number of Switches × Time Per Switch
= N_switches × context_switch_time

High overhead = frequent light changes (inefficient)
Low overhead = fewer changes (better batching)
```

### 5. Emergency Response Time
```
= Time from emergency detection to green light
Unit: milliseconds

Critical for life-safety (ambulances, fire trucks)
Target: < 5 seconds typically
```

---

## Build & Execution

### Prerequisites
- GCC compiler with C99 support
- ncurses library (terminal UI)
- POSIX threads (pthread)
- Make utility

### Building

**Ubuntu/Debian**:
```bash
sudo apt-get install build-essential libncurses5-dev

cd Traffic
make clean
make
```

**Windows (with GCC/MinGW)**:
```bash
# Ensure GCC and make installed
cd Traffic
make clean
make
```

### Running

**Default settings**:
```bash
./bin/trafficguru
# 200-second simulation, SJF algorithm, 1-3 second arrival rate
```

**Custom parameters**:
```bash
./bin/trafficguru -d 120 -g multilevel -a 1 -A 5
# 120 seconds, Multilevel Feedback, arrival rate 1-5 sec

./bin/trafficguru -g priority --debug
# Priority RR algorithm, debug mode enabled

./bin/trafficguru --benchmark
# 60-second performance benchmark
```

### Interactive Controls

| Key | Function |
|-----|----------|
| **1** | Switch to SJF |
| **2** | Switch to Multilevel Feedback |
| **3** | Switch to Priority Round Robin |
| **SPACE** | Pause/Resume |
| **e** | Trigger emergency vehicle |
| **r** | Reset simulation |
| **h** | Show help |
| **q** | Quit |

### Output

**Terminal UI Displays**:
- Real-time traffic state (all 4 lanes)
- Current scheduling algorithm
- Performance metrics (throughput, wait time, fairness)
- Vehicle queue lengths
- Signal history (recent light changes)
- Emergency status

**Performance Summary** (after exit):
```
=== PERFORMANCE SUMMARY ===
Algorithm Used: Multilevel Feedback Queue
Duration: 120 seconds
Total Vehicles Generated: 342
Total Vehicles Processed: 338
Throughput: 169 vehicles/minute

Avg Wait Time: 14.3 seconds
Fairness Index: 0.92
Context Switches: 45
Deadlocks Prevented: 3
Emergency Vehicles: 2 (Avg Response: 2.1 seconds)
===========================
```

---

## Code Structure Summary

```
src/
├── main.c                      # Entry point, simulation control
├── lane_process.c              # Lane state management
├── queue.c                     # FIFO queue for vehicles
├── scheduler.c                 # Main scheduling framework
├── sjf_scheduler.c             # SJF algorithm implementation
├── multilevel_scheduler.c      # Multilevel Feedback
├── priority_rr_scheduler.c     # Priority Round Robin
├── synchronization.c           # Mutex and condition variables
├── bankers_algorithm.c         # Deadlock prevention
├── traffic_mutex.c             # Enhanced synchronization
├── emergency_system.c          # Emergency vehicle handling
├── performance_metrics.c       # Tracking and statistics
└── visualization.c             # ncurses UI and display

include/
├── *.h                         # Header files (interfaces)
```

---

## Key Insights

1. **OS Concepts Applied**: The project successfully models traffic as a process scheduling problem with resource constraints.

2. **Multiple Solutions**: Three different algorithms offer different trade-offs (latency, fairness, complexity).

3. **Deadlock Prevention**: Banker's Algorithm prevents gridlock proactively, not reactively.

4. **Real-time Constraints**: Emergency preemption handles life-critical scenarios.

5. **Performance Trade-offs**:
   - **SJF**: Best throughput but starves long lanes
   - **Multilevel**: Balanced fairness and throughput
   - **Priority RR**: Predictable latency, handles emergencies

6. **Metrics-Driven**: Comprehensive metrics help evaluate which algorithm works best for different traffic patterns.

---

## Further Reading

**Recommended Theory**:
- Dijkstra, "The Banker's Algorithm" (1965)
- Silberschatz & Galvin, "Operating Systems Concepts" (Chapter: Scheduling, Deadlock)
- Real-time scheduling theory

**Real-World Application**:
- Adaptive Traffic Signal Control (ATSC)
- Connected Vehicle Technology (V2X)
- Smart City Traffic Management

---

