# TrafficGuru: Visual Architecture & Flow Diagrams

## 1. System Component Hierarchy

```
┌────────────────────────────────────────────────────────────┐
│                    TrafficGuruSystem                       │
├────────────────────────────────────────────────────────────┤
│                                                            │
│  ┌──────────────────────────────────────────────────────┐  │
│  │  Lane Processes                                      │  │
│  │  ┌────────────┬────────────┬────────────┬────────────┐ │
│  │  │   North    │   South    │    East    │    West    │ │
│  │  │  Lane (0)  │  Lane (1)  │  Lane (2)  │  Lane (3)  │ │
│  │  │  ┌─────┐   │  ┌─────┐   │  ┌─────┐   │  ┌─────┐   │ │
│  │  │  │Queue│   │  │Queue│   │  │Queue│   │  │Queue│   │ │
│  │  │  └─────┘   │  └─────┘   │  └─────┘   │  └─────┘   │ │
│  │  │  State: R  │  State: W  │  State: R  │  State: W  │ │
│  │  └────────────┴────────────┴────────────┴────────────┘ │
│  └──────────────────────────────────────────────────────┘  │
│                        │                                    │
│                        ▼                                    │
│  ┌──────────────────────────────────────────────────────┐  │
│  │              Central Scheduler                       │  │
│  │                                                      │  │
│  │     Algorithm: ┌──────────────────────────────┐    │  │
│  │                │  SJF                         │    │  │
│  │                │  Multilevel Feedback         │    │  │
│  │                │  Priority Round Robin        │    │  │
│  │                └──────────────────────────────┘    │  │
│  │                                                      │  │
│  │     Selects: North (GREEN) → execute               │  │
│  │     Time Quantum: 3-6 seconds (algorithm dependent)│  │
│  │                                                      │  │
│  └──────────────────────────────────────────────────────┘  │
│                        │                                    │
│                        ▼                                    │
│  ┌──────────────────────────────────────────────────────┐  │
│  │        Synchronization Layer                         │  │
│  │                                                      │  │
│  │  ┌─ Intersection Mutex (1 holder at a time)        │  │
│  │  │  ┌─ Condition Variables [North, South, E, W]   │  │
│  │  │  │  ┌─ Priority Inheritance (boost priority)   │  │
│  │  │  │  │  ┌─ Deadlock Detection (monitor loops)  │  │
│  │  │  │  │  │                                       │  │
│  │  └─ ┴─ ┴─ ┴─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─     │  │
│  └──────────────────────────────────────────────────────┘  │
│                        │                                    │
│                        ▼                                    │
│  ┌──────────────────────────────────────────────────────┐  │
│  │     Banker's Algorithm (Deadlock Prevention)         │  │
│  │                                                      │  │
│  │  Resources: 4 Quadrants (NE, NW, SW, SE)           │  │
│  │                                                      │  │
│  │  Available:   [1, 1, 1, 1]                         │  │
│  │  Allocated:   [0, 0, 0, 0] (per lane)             │  │
│  │  Need:        [2, 2, 2, 2] (per lane max)         │  │
│  │                                                      │  │
│  │  ┌──────────────────────────────────────────────┐  │  │
│  │  │ Safety Check:                               │  │  │
│  │  │ ┌─ Is request valid? (req ≤ need)         │  │  │
│  │  │ ├─ Are resources available? (req ≤ avail) │  │  │
│  │  │ └─ Would resulting state be safe?         │  │  │
│  │  │    (run safety algorithm)                 │  │  │
│  │  └──────────────────────────────────────────────┘  │  │
│  │                                                      │  │
│  │  Result: APPROVE ✓  or  DENY ✗                      │  │
│  │                                                      │  │
│  └──────────────────────────────────────────────────────┘  │
│                        │                                    │
│                        ▼                                    │
│  ┌──────────────────────────────────────────────────────┐  │
│  │       Emergency Vehicle System                       │  │
│  │                                                      │  │
│  │  Detection → Priority Override → Preempt Signal  │  │
│  │                                                      │  │
│  │  Types: Ambulance, Fire Truck, Police             │  │
│  │  Action: Boost lane to priority 0 (highest)       │  │
│  │                                                      │  │
│  └──────────────────────────────────────────────────────┘  │
│                        │                                    │
│                        ▼                                    │
│  ┌──────────────────────────────────────────────────────┐  │
│  │    Performance Metrics & Visualization              │  │
│  │                                                      │  │
│  │  Metrics:                                           │  │
│  │    • Throughput: 165 vehicles/min                   │  │
│  │    • Avg Wait: 14.5 sec                            │  │
│  │    • Fairness: 0.91                                │  │
│  │    • Context Switches: 47                          │  │
│  │    • Deadlock Preventions: 2                       │  │
│  │                                                      │  │
│  │  Display (ncurses):                                 │  │
│  │    • Real-time lane status                          │  │
│  │    • Queue visualization                            │  │
│  │    • Gantt chart (execution timeline)               │  │
│  │    • Signal history                                 │  │
│  │                                                      │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                            │
└────────────────────────────────────────────────────────────┘
```

---

## 2. Simulation Main Loop Flow

```
┌─────────────────────────────────┐
│    Simulation Start             │
│  start_traffic_simulation()     │
└────────┬────────────────────────┘
         │
         ▼
┌─────────────────────────────────────────────────┐
│ Create Threads:                                 │
│ 1. simulation_main_loop()                       │
│ 2. vehicle_generator_loop()                     │
│ 3. (Main thread: UI & input)                    │
└────────┬────────────────────────────────────────┘
         │
         ├──────────────────┬──────────────────────────┐
         ▼                  ▼                          ▼
    ┌─────────────┐   ┌──────────────┐   ┌──────────────────────┐
    │ Simulation  │   │   Vehicle    │   │   Main Thread (UI)   │
    │   Thread    │   │  Generator   │   │   & User Input       │
    │             │   │              │   │                      │
    │ Loop:       │   │ Loop:        │   │ Loop:                │
    │ 1. Update   │   │ 1. Sleep     │   │ 1. Display UI        │
    │ 2. Process  │   │ 2. Generate  │   │ 2. Get input         │
    │ 3. Sleep    │   │    vehicle   │   │ 3. Handle hotkeys    │
    │             │   │ 3. Sleep     │   │ 4. Update display    │
    │ Every 300ms │   │    Random    │   │                      │
    │             │   │    time      │   │                      │
    └─────────────┘   └──────────────┘   └──────────────────────┘
         │                  │                      │
         ▼                  ▼                      ▼
    ┌─────────────────────────────────────────────────┐
    │     Global State (Protected by Locks)           │
    │                                                  │
    │  Lane Queues, Metrics, Emergency Status...     │
    └─────────────────────────────────────────────────┘
```

---

## 3. Traffic Event Processing Cycle

```
START OF SIMULATION ITERATION
            │
            ▼
    ┌───────────────────┐
    │ update_simulation │
    │   _state()        │
    └─────────┬─────────┘
              │
              ├─→ Update time-based metrics
              ├─→ Update emergency progress
              └─→ Check deadlocks every 100 cycles
              
              ▼
    ┌───────────────────────┐
    │ process_traffic_      │
    │ events()              │
    └─────────┬─────────────┘
              │
              ├─→ Call schedule_next_lane()
              │   (Consult current algorithm)
              │
              ▼
    ┌──────────────────────────────────────┐
    │ Execute Lane Time Slice              │
    │                                      │
    │ 1. acquire_intersection()            │
    │    (lock intersection for lane)      │
    │                                      │
    │ 2. for i=0 to BATCH_EXIT_SIZE-1:    │
    │      if lane not empty:              │
    │        remove_vehicle_from_lane()    │
    │        sleep(VEHICLE_CROSS_TIME)    │
    │                                      │
    │ 3. release_intersection()            │
    │    (unlock, signal next ready lane)  │
    │                                      │
    └────────┬─────────────────────────────┘
             │
             ▼
    ┌──────────────────────────┐
    │ usleep(300ms)            │
    │ (Control simulation      │
    │  speed, allow UI update) │
    └────────┬─────────────────┘
             │
             ├─→ Duration not exceeded? LOOP
             └─→ Duration exceeded? EXIT
```

---

## 4. Scheduling Algorithm Comparison

### SJF (Shortest Job First)

```
Lanes: [Q_len=2, Q_len=5, Q_len=1, Q_len=4]
Time:  [  3     10     1       8  ]

SJF Decision:
Find lane with minimum time: Lane 2 (1 second)
→ Give Lane 2 green light
→ After execution, recalculate

Sequence: 2 → 0 → 3 → 1 → 2 → ...

Gantt Chart:
┌───┬─────────┬───────────┬───────┐
│ 2 │    0    │     3     │   1   │
└───┴─────────┴───────────┴───────┘
0   1         6          16      24
```

**Characteristics**:
- ✓ Optimal average wait time (for non-preemptive)
- ✗ Starvation risk for long lanes
- ✗ Unpredictable delays

---

### Multilevel Feedback Queue

```
Lanes with Priorities:
Lane 0: Q_len=2  → MEDIUM priority (used full quantum last time)
Lane 1: Q_len=8  → LOW priority (long queue)
Lane 2: Q_len=1  → HIGH priority (short queue)
Lane 3: waiting 60s → promoted to HIGH (aging)

Scheduling:
1. Check HIGH priority: [Lane 2, Lane 3]
2. Round-robin: Lane 2 gets green
3. Allocate: 2 second quantum (HIGH level)

After execution:
- Lane 2: used 2s → stays MEDIUM (next iteration)
- Others: wait times increase

Gantt Chart with Priority Levels:
Time: 0     2     4      6      8    10  ...
      ┌─H─┬─M──┬─H─┬──M────┬─H────┤
      │ 2 │  3 │ 2 │   0   │  1   │
      └───┴────┴───┴───────┴──────┤
```

**Characteristics**:
- ✓ Prevents starvation (via aging)
- ✓ Adapts to lane characteristics
- ✓ Good fairness
- ✗ More complex parameters

---

### Priority Round Robin

```
Lane Priorities (Static):
Lane 0: Priority 2 (Normal)
Lane 1: Priority 3 (Low traffic)
Lane 2: Priority 2 (Normal)
Lane 3: Priority 1 (High traffic, Q>3)
EMERGENCY: Priority 0 (if present)

Priority Queue:
Priority 0: [Emergency vehicles]
Priority 1: [Lane 3]
Priority 2: [Lane 0, Lane 2] (round-robin)
Priority 3: [Lane 1]

Scheduling Decision:
for pri = 0 to 3:
    if lane at pri has vehicles:
        return next lane at pri (round-robin)

Gantt Chart:
Time: 0  3  6  9 12 15 18 21 24
      ┌──┬──┬──┬──┬──┬──┬──┬──┐
      │3 │0 │2 │3 │0 │2 │3 │1│
      └──┴──┴──┴──┴──┴──┴──┴──┘
      (3-sec quantum for each)
```

**Characteristics**:
- ✓ Predictable response time
- ✓ Emergency preemption
- ✓ Simple and fair
- ✗ May switch frequently

---

## 5. Banker's Algorithm Safety Check

```
REQUEST: Lane North wants [1, 1, 0, 0] (NE + NW quadrants)

Current State:
                NE  NW  SW  SE
available:    [ 1,  1,  1,  1]
allocated[N]: [ 0,  0,  0,  0]
need[N]:      [ 2,  2,  0,  0]

Step 1: Check if request ≤ need
        [1,1,0,0] ≤ [2,2,0,0] ✓ YES

Step 2: Check if request ≤ available
        [1,1,0,0] ≤ [1,1,1,1] ✓ YES

Step 3: Tentatively allocate
        available  = [0, 0, 1, 1]
        allocated[N] = [1, 1, 0, 0]
        need[N]    = [1, 1, 0, 0]

Step 4: Run Safety Algorithm
        Check if all lanes can complete:
        
        Lane N: need [1,1,0,0] ≤ avail [0,0,1,1]? NO
        Lane S: need [2,2,0,0] ≤ avail [0,0,1,1]? NO
        Lane E: need [0,2,2,0] ≤ avail [0,0,1,1]? NO
        Lane W: need [0,0,2,2] ≤ avail [0,0,1,1]? YES
        
        If W finishes: available += allocated[W]
        Now: available = [1,1,2,2] (all quadrants free)
        
        Retry others: All can complete now
        
        Safe sequence found: W → N → S → E ✓

Result: REQUEST ✓ APPROVED
        Allow Lane North to cross

Alternative (UNSAFE case):
Step 3: Tentatively allocate (same as above)
Step 4: No safe sequence exists
Result: REQUEST ✗ DENIED
        Don't allocate, Lane waits
```

---

## 6. Emergency Vehicle Preemption

```
Timeline:
0 sec: Normal operation
       [SJF algorithm running]
       Lane 0: 5 vehicles
       Lane 1: 2 vehicles
       Lane 2: 1 vehicle
       Lane 3: 3 vehicles

5 sec: EMERGENCY AMBULANCE enters Lane 2
       
       Immediate Actions:
       ├─→ Detect emergency vehicle
       ├─→ Set Lane 2 priority to 0 (highest)
       ├─→ Interrupt current execution if needed
       ├─→ Grant green light to Lane 2 ASAP
       └─→ Start emergency timer
       
       Lane 2 gets green light (normal would be Lane 0)

7 sec: Lane 2 crosses emergency vehicle (3 seconds)
       
       After Emergency:
       ├─→ Clear emergency flag
       ├─→ Restore Lane 2 priority
       ├─→ Resume normal scheduling
       └─→ Log response time (2 seconds = excellent)

9 sec: Resume normal operation
       Update emergency statistics:
       - Total emergencies: +1
       - Total response time: 2 seconds
       - Average response: 2 seconds

Display Update:
┌──────────────────────────────────────┐
│ Emergency Status: AMBULANCE CLEARED  │
│ Response Time: 2.0 seconds ✓         │
│ Total Emergencies: 47                │
│ Avg Response: 1.8 seconds            │
└──────────────────────────────────────┘
```

---

## 7. Multi-threaded Synchronization Points

```
┌──────────────┐
│ Main Thread  │ ◄─── User input (q to quit, h for help, etc.)
│ (ncurses UI) │      Display updates
└───────┬──────┘
        │
        │ nodelay(TRUE) - Non-blocking input
        │
        ├──────────────────┬─────────────────────────┐
        │                  │                         │
        ▼                  ▼                         ▼
┌────────────────┐ ┌──────────────────┐ ┌─────────────────────┐
│  SIM Thread    │ │  Vehicle Gen     │ │  (Shared Resources) │
│                │ │  Thread          │ │                     │
│ Loop:          │ │                  │ │ - g_traffic_system  │
│ - Schedule     │ │ Loop:            │ │ - lanes[]           │
│ - Execute      │ │ - Sleep random   │ │ - global_state_lock │
│ - Check dead   │ │ - Generate veh   │ │ - metrics           │
│ - Update UI    │ │ - Add to queue   │ │ - emergencies       │
│                │ │                  │ │                     │
│ Hold: scheduler│ │ Hold: gen lock   │ │ Protected by:       │
│ mutex          │ │                  │ │ pthread_mutex_t     │
│                │ │                  │ │                     │
└────────────────┘ └──────────────────┘ └─────────────────────┘
        │                  │                         │
        │                  │                         │
        └──────────────────┴─────────────────────────┘
                           │
                           ▼
                ┌─────────────────────┐
                │ Lanes (Protected)   │
                ├─────────────────────┤
                │ Lane 0:             │
                │  - queue_lock       │
                │  - queue (FIFO)     │
                │  - state            │
                │  - priority         │
                ├─────────────────────┤
                │ ... (Lane 1, 2, 3)  │
                └─────────────────────┘
                           │
                           ▼
                ┌─────────────────────────────┐
                │ Intersection (Most Protected)│
                ├─────────────────────────────┤
                │ intersection_lock (mutex)   │
                │ Only 1 holder allowed       │
                │ condition_vars[4] (signal)  │
                │                             │
                │ Critical Section:           │
                │ execute_lane_time_slice()   │
                │ (Atomic from start to end)  │
                └─────────────────────────────┘
                           │
                           ▼
                ┌──────────────────────────┐
                │ Banker's Algorithm Check │
                ├──────────────────────────┤
                │ resource_lock (mutex)    │
                │ available[], allocated[]│
                │ need[], maximum[]       │
                │                          │
                │ Safety verification:    │
                │ Can lane complete safely?│
                └──────────────────────────┘
```

---

## 8. State Transition Diagram for Lanes

```
                   ┌────────────────┐
                   │   WAITING      │
                   │ (No vehicles)  │
                   └───────┬────────┘
                           │
                           │ add_vehicle_to_lane()
                           ▼
                   ┌────────────────┐
                   │    READY       │
                   │(Vehicles, no   │
                   │ green light)   │
                   └───────┬────────┘
                           │
                           │ schedule_next_lane() returns this lane
                           │ acquire_intersection() succeeds
                           ▼
                   ┌────────────────┐
                   │   RUNNING      │
                   │(Active, green  │
                   │ light on)      │
                   └───────┬────────┘
                           │
                           ├─ Exit due to:
                           │
                           ├─→ Queue empty + time quantum expired
                           │   → Back to WAITING
                           │
                           ├─→ Queue not empty + time quantum expired
                           │   → Back to READY (round-robin)
                           │
                           └─→ Banker's check fails (deadlock risk)
                               → Move to BLOCKED
                               
              Optional: ┌─────────────────────┐
                       │     BLOCKED         │
                       │ (Unsafe to proceed) │
                       │ (Banker's algorithm)│
                       └──────────┬──────────┘
                                  │
                                  │ Waiting for safe state
                                  ▼
                            (back to READY or WAITING)
```

---

## 9. Performance Metrics Collection Timeline

```
Simulation Start: t=0
                  │
                  ▼
    ┌──────────────────────────────────────┐
    │ Initialize counters:                 │
    │ - total_vehicles_generated = 0       │
    │ - total_vehicles_processed = 0       │
    │ - total_wait_time = 0                │
    │ - context_switches = 0               │
    │ - emergency_count = 0                │
    │ - deadlock_count = 0                 │
    └──────────────────────────────────────┘
                  │
    ┌─────────────┴─────────────────────────────────────┐
    │                                                   │
    ▼                                                   ▼
During Execution (Every Update Cycle)      End of Simulation
    │                                                   │
    ├─ Add vehicle: generated++                        │
    ├─ Remove vehicle: processed++                     │
    ├─ Update wait: total_wait += time                 │
    ├─ Context switch: switches++                      │
    ├─ Deadlock check: deadlocks++                     │
    └─ Emergency: emergencies++                        │
                                                       │
                  ┌─────────────────────────────────────┘
                  │
                  ▼
    ┌──────────────────────────────────────────────────┐
    │ Calculate Final Metrics:                         │
    │                                                  │
    │ 1. Throughput = processed / (duration / 60)      │
    │    Example: 340 / (120 / 60) = 170 vehicles/min │
    │                                                  │
    │ 2. Avg Wait = total_wait / processed             │
    │    Example: 5000 / 340 = 14.7 seconds           │
    │                                                  │
    │ 3. Fairness (Jain's): (Σ x)² / (n × Σ x²)      │
    │    where x = vehicles per lane                  │
    │    Example: (85+85+85+85)² / 4×(85²) = 1.0      │
    │                                                  │
    │ 4. Context Overhead = switches × 500ms           │
    │    Example: 45 × 0.5 = 22.5 seconds             │
    │                                                  │
    │ 5. Emergency Response = avg(response_times)     │
    │    Example: average 2.3 seconds                 │
    │                                                  │
    │ 6. Deadlock Prevention = count                   │
    │    Example: 3 unsafe states prevented            │
    └──────────────────────────────────────────────────┘
                  │
                  ▼
    ┌──────────────────────────────────────────────────┐
    │ Display Summary (after ncurses shutdown):        │
    │                                                  │
    │ === PERFORMANCE SUMMARY ===                     │
    │ Algorithm: Multilevel Feedback Queue             │
    │ Duration: 120 seconds                            │
    │ Generated: 340 vehicles                          │
    │ Processed: 340 vehicles                          │
    │ Throughput: 170 vehicles/minute                  │
    │ Average Wait: 14.7 seconds                       │
    │ Fairness Index: 1.00                             │
    │ Context Switches: 45 (22.5 sec overhead)         │
    │ Deadlocks Prevented: 3                           │
    │ Emergencies: 2 (Avg Response: 2.1 sec)           │
    │ ===========================                      │
    └──────────────────────────────────────────────────┘
```

---

## 10. Intersection Quadrant Resource Model

```
                        North
                          ↑
                      ┌───────┐
                      │ Lane  │
                      │   0   │
                      └───────┘
                          │
         ┌────────────────┼────────────────┐
         │                │                │
       West               │              East
         │                │                │
    ┌────────┐   ┌─────────────────┐   ┌────────┐
    │ Lane 3 │   │                 │   │ Lane 2 │
    └────────┘   │   INTERSECTION  │   └────────┘
         │       │                 │      │
    ┌────────┐   │ ┌───────────────┤   ┌────────┐
    │ Queue  │   │ │      NW │ NE  │   │ Queue  │
    └────────┘   │ │    ────┼───── │   └────────┘
         │       │ │      SW │ SE  │      │
    ┌────────┐   │ └───────────────┤   ┌────────┐
    │        │   │                 │   │        │
    └────────┘   └─────────────────┘   └────────┘
         │                │                │
       South               │
                          ▼
                      ┌───────┐
                      │ Lane  │
                      │   1   │
                      └───────┘

Quadrant Assignment for Each Lane:

Lane 0 (North):
  Straight → SE
  Left → NE + SE
  Right → NW
  U-turn → NW + SW

Lane 1 (South):
  Straight → NW
  Left → NW + SW
  Right → NE
  U-turn → NE + SE

Lane 2 (East):
  Straight → NW
  Left → NW + NE
  Right → SW
  U-turn → SW + SE

Lane 3 (West):
  Straight → SE
  Left → SE + SW
  Right → NE
  U-turn → NE + NW

Deadlock Example:
Lane 0 (North, going straight): Needs SE
Lane 1 (South, going straight): Needs NW
Lane 2 (East, going straight): Needs NW
Lane 3 (West, going straight): Needs SE

Scenario:
  Lane 0 holds SE, needs NW (held by Lane 1)
  Lane 1 holds NW, needs SE (held by Lane 0)
  → DEADLOCK (circular wait)

Solution (Banker's):
  Reject one of these requests
  Ensure safe allocation order:
  If Lane 2 & 3 finish: all quadrants free
  Then Lane 0 & 1 can proceed
```

---

This visualization guide complements the theoretical explanation in PROJECT_EXPLANATION.md
and helps understand the complex interactions between components.
