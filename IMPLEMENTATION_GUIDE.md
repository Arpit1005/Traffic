# TrafficGuru: Implementation Deep Dive & Code Examples

## Table of Contents
1. [Data Structure Details](#data-structure-details)
2. [Key Algorithm Implementations](#key-algorithm-implementations)
3. [Thread Synchronization Examples](#thread-synchronization-examples)
4. [Error Handling & Deadlock Resolution](#error-handling--deadlock-resolution)
5. [Performance Optimization Techniques](#performance-optimization-techniques)
6. [Testing & Debugging](#testing--debugging)

---

## Data Structure Details

### 1. Lane Process Structure and Operations

**Header Definition** (`lane_process.h`):
```c
typedef enum {
    WAITING = 0,  // No vehicles, idle
    READY = 1,    // Vehicles waiting, no green light
    RUNNING = 2,  // Active, executing
    BLOCKED = 3   // Blocked by Banker's algorithm
} LaneState;

typedef struct {
    int lane_id;                    // 0=North, 1=South, 2=East, 3=West
    Queue* queue;                   // FIFO queue for vehicles
    int queue_length;               // Current queue size
    int max_queue_length;           // Capacity
    LaneState state;                // Current state
    int priority;                   // 0=High, 3=Low
    int waiting_time;               // Cumulative wait
    pthread_t thread_id;            // Lane's thread ID
    pthread_mutex_t queue_lock;     // Protects queue
    pthread_cond_t queue_cond;      // Signal when green
    time_t last_arrival_time;       // Last vehicle arrival
    time_t last_service_time;       // Last execution time
    int total_vehicles_served;      // Throughput counter
    int total_waiting_time;         // For averaging
    int requested_quadrants;        // Bitmask of needed resources
    int allocated_quadrants;        // Bitmask of held resources
} LaneProcess;
```

**Initialization** (Conceptual):
```c
void init_lane_process(LaneProcess* lane, int lane_id, int max_capacity) {
    lane->lane_id = lane_id;
    lane->queue = create_queue(max_capacity);
    lane->queue_length = 0;
    lane->max_queue_length = max_capacity;
    lane->state = WAITING;
    lane->priority = 2;  // Default medium priority
    lane->waiting_time = 0;
    lane->total_vehicles_served = 0;
    lane->total_waiting_time = 0;
    
    // Initialize synchronization primitives
    pthread_mutex_init(&lane->queue_lock, NULL);
    pthread_cond_init(&lane->queue_cond, NULL);
    
    // Initialize time tracking
    lane->last_arrival_time = time(NULL);
    lane->last_service_time = time(NULL);
}
```

**Adding Vehicle** (Thread-safe):
```c
void add_vehicle_to_lane(LaneProcess* lane, int vehicle_id) {
    if (!lane || !lane->queue) return;
    
    // Lock queue for exclusive access
    pthread_mutex_lock(&lane->queue_lock);
    {
        // Check capacity
        if (!is_full(lane->queue)) {
            // Add to queue
            enqueue(lane->queue, vehicle_id);
            lane->queue_length = get_size(lane->queue);
            
            // Update state if was WAITING
            if (lane->state == WAITING) {
                lane->state = READY;
            }
            
            // Record arrival
            lane->last_arrival_time = time(NULL);
        } else {
            // Queue full - potential overflow
            // In real system: reject vehicle or increase queue
        }
    }
    pthread_mutex_unlock(&lane->queue_lock);
}
```

**Processing Vehicle** (Critical Section):
```c
int remove_vehicle_from_lane(LaneProcess* lane) {
    if (!lane || !lane->queue) return -1;
    
    pthread_mutex_lock(&lane->queue_lock);
    {
        if (is_empty(lane->queue)) {
            // No vehicles to process
            pthread_mutex_unlock(&lane->queue_lock);
            return -1;
        }
        
        // Remove next vehicle from queue
        int vehicle_id = dequeue(lane->queue);
        lane->queue_length = get_size(lane->queue);
        
        // Update metrics
        lane->total_vehicles_served++;
        lane->last_service_time = time(NULL);
        
        // Check if queue now empty
        if (is_empty(lane->queue)) {
            lane->state = WAITING;
        }
        
        pthread_mutex_unlock(&lane->queue_lock);
        return vehicle_id;
    }
}
```

### 2. Queue (Circular Array) Implementation

**Structure**:
```c
typedef struct {
    int* vehicles;          // Dynamic array of vehicle IDs
    int front;              // Index of first element
    int rear;               // Index of last element + 1
    int size;               // Current number of elements
    int capacity;           // Maximum capacity
    int enqueue_count;      // Total enqueues for statistics
    int dequeue_count;      // Total dequeues
    int overflow_count;     // Times overflow occurred
} Queue;
```

**Enqueue (FIFO - Add to Rear)**:
```c
bool enqueue(Queue* queue, int vehicle_id) {
    if (!queue) return false;
    
    // Check if full
    if (queue->size >= queue->capacity) {
        queue->overflow_count++;
        
        // Option 1: Reject
        return false;
        
        // Option 2: Resize (if dynamic)
        // resize_queue(queue, queue->capacity * 2);
    }
    
    // Add to rear
    queue->vehicles[queue->rear] = vehicle_id;
    queue->rear = (queue->rear + 1) % queue->capacity;  // Circular wrap
    queue->size++;
    queue->enqueue_count++;
    
    return true;
}
```

**Dequeue (FIFO - Remove from Front)**:
```c
int dequeue(Queue* queue) {
    if (!queue || queue->size == 0) {
        return -1;  // Queue empty
    }
    
    // Get from front
    int vehicle_id = queue->vehicles[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;  // Circular wrap
    queue->size--;
    queue->dequeue_count++;
    
    return vehicle_id;
}
```

**Visualization** (Circular Buffer):
```
Capacity: 5
State 1 - Normal queue:
  [2, 3, 4, _, _]
   ↑        ↑
  front   rear
  size=3

State 2 - After wraparound:
  [5, 6, 1, 2, 3]
        ↑  ↑
      rear front
  size=3 (actually [1, 2, 3])

State 3 - Nearly full:
  [1, 2, 3, 4, 5]
           ↑  ↑
        rear front
  size=5 (actually [5, 1, 2, 3, 4])
```

---

## Key Algorithm Implementations

### 1. SJF Scheduler (Simplified Implementation)

**Core Logic**:
```c
int schedule_next_lane_sjf(Scheduler* scheduler, LaneProcess lanes[4]) {
    int best_lane = -1;
    int shortest_time = INT_MAX;
    
    // Loop through all lanes
    for (int i = 0; i < NUM_LANES; i++) {
        // Skip if lane has no vehicles or is blocked
        if (lanes[i].state == WAITING || lanes[i].state == BLOCKED) {
            continue;
        }
        
        // Calculate estimated service time
        // Estimate = queue_length × average_vehicle_processing_time
        int estimated_time = lanes[i].queue_length * VEHICLE_CROSS_TIME;
        
        // SJF metric: prefer shortest time
        if (estimated_time < shortest_time) {
            shortest_time = estimated_time;
            best_lane = i;
        }
    }
    
    return best_lane;  // Returns -1 if no lane ready
}
```

**Time Complexity**: O(4) = O(1) - constant time (4 lanes)

**Why It Works**:
- Minimizes average wait time (provably optimal for non-preemptive)
- Simple to implement and understand
- Low overhead (single pass through lanes)

**Starvation Problem**:
```
Scenario:
  Lane 0: 10 vehicles (estimated 30s)
  Lane 1: 1 vehicle  (estimated 3s)
  Lane 2: 2 vehicles (estimated 6s)
  Lane 3: 1 vehicle  (estimated 3s)

Scheduling sequence: 1 → 3 → 2 → 1 → 3 → 2 → ...

Result: Lane 0 never gets scheduled!
        (Vehicles keep arriving in shorter lanes)
        
STARVATION: Lane 0's vehicles wait indefinitely
```

---

### 2. Multilevel Feedback Queue (Priority-based)

**Conceptual Implementation**:
```c
typedef struct {
    Queue* queues[3];           // Three priority levels
    int time_quantum[3] = {2, 4, 6};  // Different quantums
    LaneProcess* last_scheduled;       // For round-robin
} MultilevelScheduler;

int schedule_next_lane_multilevel(Scheduler* scheduler, LaneProcess lanes[4]) {
    // Priority levels: 0=HIGH (2s quantum), 1=MEDIUM (4s), 2=LOW (6s)
    
    // Step 1: Determine priority based on queue length
    for (int i = 0; i < NUM_LANES; i++) {
        if (lanes[i].queue_length <= 2) {
            lanes[i].priority = 0;  // HIGH
        } else if (lanes[i].queue_length <= 5) {
            lanes[i].priority = 1;  // MEDIUM
        } else {
            lanes[i].priority = 2;  // LOW
        }
    }
    
    // Step 2: Check aging (boost priority if waiting too long)
    time_t now = time(NULL);
    for (int i = 0; i < NUM_LANES; i++) {
        int wait_time = now - lanes[i].last_service_time;
        if (wait_time > 30) {  // Waited > 30 seconds
            if (lanes[i].priority > 0) {
                lanes[i].priority--;  // Promote by one level
            }
        }
    }
    
    // Step 3: Schedule using round-robin within priority
    for (int pri = 0; pri < 3; pri++) {
        for (int i = 0; i < NUM_LANES; i++) {
            if (lanes[i].priority == pri && lanes[i].state != WAITING) {
                // Found a lane at this priority
                scheduler->time_quantum = time_quantum[pri];
                return i;
            }
        }
    }
    
    return -1;  // No lane ready
}
```

**Prevents Starvation**:
```
With same scenario as SJF:
  Lane 0: 10 vehicles (priority 2=LOW, quantum 6s)
  Lane 1: 1 vehicle  (priority 0=HIGH, quantum 2s)
  Lane 2: 2 vehicles (priority 1=MED, quantum 4s)
  Lane 3: 1 vehicle  (priority 0=HIGH, quantum 2s)

Scheduling sequence:
  1 (2s) → 3 (2s) → 2 (4s) → 1 (2s) → 3 (2s) → ...
  
After 30s+ wait, Lane 0 gets promoted:
  0 (6s) → ... (continues regularly)
  
RESULT: Lane 0 eventually runs (no starvation)
```

---

### 3. Banker's Algorithm (Deadlock Prevention)

**Safety Check Implementation**:
```c
bool is_safe_state(BankersState* state) {
    if (!state) return false;
    
    bool finish[NUM_LANES] = {false};  // Track completed lanes
    int available[NUM_QUADRANTS];
    
    // Copy current available resources
    for (int q = 0; q < NUM_QUADRANTS; q++) {
        available[q] = state->available[q];
    }
    
    // Safety algorithm: find safe sequence
    int count = 0;
    while (count < NUM_LANES) {
        bool found = false;
        
        // Find lane that can complete with available resources
        for (int lane = 0; lane < NUM_LANES; lane++) {
            if (finish[lane]) continue;  // Already finished
            
            // Check if lane's need ≤ available
            bool can_finish = true;
            for (int q = 0; q < NUM_QUADRANTS; q++) {
                if (state->need[lane][q] > available[q]) {
                    can_finish = false;
                    break;
                }
            }
            
            if (can_finish) {
                // Lane can complete - mark as finished
                finish[lane] = true;
                
                // Release its resources
                for (int q = 0; q < NUM_QUADRANTS; q++) {
                    available[q] += state->allocation[lane][q];
                }
                
                found = true;
                count++;
                break;
            }
        }
        
        if (!found) {
            // No lane can finish - UNSAFE STATE
            return false;
        }
    }
    
    // All lanes can finish - SAFE STATE
    return true;
}
```

**Request Handling** (Core Banker's Algorithm):
```c
bool request_resources(BankersState* state, int lane_id, 
                       int request[NUM_QUADRANTS]) {
    pthread_mutex_lock(&state->resource_lock);
    {
        // Check 1: Request ≤ Need
        for (int q = 0; q < NUM_QUADRANTS; q++) {
            if (request[q] > state->need[lane_id][q]) {
                pthread_mutex_unlock(&state->resource_lock);
                return false;  // Exceed maximum claim
            }
        }
        
        // Check 2: Request ≤ Available
        for (int q = 0; q < NUM_QUADRANTS; q++) {
            if (request[q] > state->available[q]) {
                pthread_mutex_unlock(&state->resource_lock);
                return false;  // Insufficient resources
            }
        }
        
        // Check 3: Safety (tentatively allocate & verify safe)
        // Save current state for rollback
        int saved_available[NUM_QUADRANTS];
        int saved_allocation[NUM_QUADRANTS];
        
        for (int q = 0; q < NUM_QUADRANTS; q++) {
            saved_available[q] = state->available[q];
            saved_allocation[q] = state->allocation[lane_id][q];
        }
        
        // Tentatively allocate
        for (int q = 0; q < NUM_QUADRANTS; q++) {
            state->available[q] -= request[q];
            state->allocation[lane_id][q] += request[q];
            state->need[lane_id][q] -= request[q];
        }
        
        // Check if result is safe
        if (is_safe_state(state)) {
            // APPROVE - keep allocation
            state->deadlock_preventions++;
            pthread_mutex_unlock(&state->resource_lock);
            return true;
        } else {
            // DENY - rollback allocation
            for (int q = 0; q < NUM_QUADRANTS; q++) {
                state->available[q] = saved_available[q];
                state->allocation[lane_id][q] = saved_allocation[q];
                state->need[lane_id][q] += request[q];
            }
            
            pthread_mutex_unlock(&state->resource_lock);
            return false;  // Unsafe state - deny request
        }
    }
}
```

---

## Thread Synchronization Examples

### 1. Lane Mutual Exclusion

**Scenario**: Multiple lanes trying to cross simultaneously

```c
// Lane Thread Execution
void execute_lane_time_slice(LaneProcess* lane, int quantum) {
    // CRITICAL: Only one lane crosses at a time
    acquire_intersection(&g_traffic_system->intersection, lane);
    
    // Now in critical section - exclusive intersection access
    {
        // Process BATCH_EXIT_SIZE vehicles
        for (int i = 0; i < BATCH_EXIT_SIZE; i++) {
            if (is_empty(lane->queue)) break;
            
            // Remove and process vehicle
            int vehicle_id = remove_vehicle_from_lane(lane);
            if (vehicle_id != -1) {
                // Simulate crossing (real system: actual vehicle movement)
                usleep(VEHICLE_CROSS_TIME * 1000000);
                
                // Update metrics
                lane->total_vehicles_served++;
            }
        }
    }
    // EXIT critical section
    release_intersection(&g_traffic_system->intersection, lane);
}

// Actual implementation using mutex
void acquire_intersection(IntersectionMutex* inter, LaneProcess* lane) {
    // Block until exclusive access obtained
    pthread_mutex_lock(&inter->intersection_lock);
    {
        // Now have exclusive access
        inter->current_lane = lane->lane_id;
        inter->lock_holder = pthread_self();
        inter->lock_acquisition_time = time(NULL);
        inter->intersection_available = false;
    }
}

void release_intersection(IntersectionMutex* inter, LaneProcess* lane) {
    pthread_mutex_unlock(&inter->intersection_lock);
    
    // Signal next waiting lane (condition variable)
    // Find highest priority waiting lane and signal it
    for (int priority = 0; priority <= 3; priority++) {
        for (int i = 0; i < NUM_LANES; i++) {
            if (g_traffic_system->lanes[i].priority == priority &&
                g_traffic_system->lanes[i].state == READY) {
                pthread_cond_signal(&inter->condition_vars[i]);
                break;
            }
        }
    }
}
```

**Timeline Visualization**:
```
Time  Lane 0        Lane 1        Lane 2        Lane 3
────────────────────────────────────────────────────────────
0ms   waiting       waiting       waiting       waiting
10ms  acquire       waiting       waiting       waiting
20ms  [executing]   blocked       waiting       waiting
30ms  [executing]   blocked       waiting       waiting
40ms  release       acquire       waiting       waiting
50ms  waiting       [executing]   blocked       waiting
60ms  waiting       [executing]   blocked       waiting
70ms  waiting       release       acquire       blocked
80ms  waiting       waiting       [executing]   blocked
...   (pattern continues)

Key Points:
- Only one lane executing at a time
- Blocked on mutex_lock() if intersection busy
- Released upon mutex_unlock()
```

### 2. Condition Variable Signaling

**Pattern**: Lane waits for green light signal

```c
// Lane wants green light
void lane_wait_for_signal(LaneProcess* lane) {
    pthread_mutex_lock(&lane->queue_lock);
    {
        // While not scheduled for green
        while (lane->state != RUNNING) {
            // Wait (releases lock, sleeps)
            // Woken by scheduler signal
            pthread_cond_wait(&lane->queue_cond, &lane->queue_lock);
        }
        
        // Got green light - execute
        execute_lane_time_slice(lane, scheduler->time_quantum);
        
        // Back to READY/WAITING
        lane->state = READY;
    }
    pthread_mutex_unlock(&lane->queue_lock);
}

// Scheduler signals lane
void scheduler_signal_lane(int lane_id) {
    LaneProcess* lane = &g_traffic_system->lanes[lane_id];
    
    pthread_mutex_lock(&lane->queue_lock);
    {
        if (lane->state == READY && !is_empty(lane->queue)) {
            lane->state = RUNNING;
            
            // Signal waiting thread
            pthread_cond_signal(&lane->queue_cond);
        }
    }
    pthread_mutex_unlock(&lane->queue_lock);
}
```

**Sequence Diagram**:
```
Scheduler Thread             Lane 0 Thread           Lane 1 Thread
────────────────────────────────────────────────────────────────────

[Schedule next]
                            Call: wait_for_signal()
                            Lock queue_lock
                            Cond_wait()
                            (SLEEPS, releases lock)

Decision: Lane 0
Call: signal_lane(0)
Lock lane 0 queue
Set state = RUNNING
Cond_signal()
                            (WOKEN UP)
                            Check: state = RUNNING ✓
                            Execute
                            (critical section)
                            Set state = READY

                            Signal lane 1?
Call: signal_lane(1)
Lock lane 1 queue
Set state = RUNNING
Cond_signal()
                                        (WOKEN UP)
                                        Execute
```

---

## Error Handling & Deadlock Resolution

### 1. Deadlock Detection (Circular Wait)

```c
bool detect_and_resolve_advanced_deadlock(LaneProcess lanes[4]) {
    // Strategy: Find circular resource dependencies
    
    // Build dependency graph
    int depends_on[NUM_LANES];  // -1 = no dependency
    for (int i = 0; i < NUM_LANES; i++) {
        depends_on[i] = -1;
        
        // Check what resources lane i needs that others hold
        for (int j = 0; j < NUM_LANES; j++) {
            if (i == j) continue;
            
            // If lane i needs resource held by lane j
            if ((lanes[i].requested_quadrants & lanes[j].allocated_quadrants) &&
                lanes[j].state == RUNNING) {
                depends_on[i] = j;
                break;
            }
        }
    }
    
    // Detect cycle using DFS
    for (int start = 0; start < NUM_LANES; start++) {
        int current = start;
        bool visited[NUM_LANES] = {false};
        
        while (current != -1) {
            if (visited[current]) {
                // Cycle found! Deadlock detected
                if (current == start) {
                    // Resolve: Preempt lower priority lane
                    resolve_circular_deadlock(lanes, start, current);
                    return true;
                }
            }
            
            visited[current] = true;
            current = depends_on[current];
        }
    }
    
    return false;  // No deadlock
}

void resolve_circular_deadlock(LaneProcess lanes[4], int start, int cycle_node) {
    // Find lowest priority lane in cycle and preempt it
    
    int min_priority_lane = -1;
    int min_priority = 0;
    
    int current = cycle_node;
    while (true) {
        if (lanes[current].priority > min_priority) {
            min_priority = lanes[current].priority;
            min_priority_lane = current;
        }
        
        // Next in cycle...
        int found_next = -1;
        for (int i = 0; i < NUM_LANES; i++) {
            if ((lanes[current].requested_quadrants & 
                 lanes[i].allocated_quadrants) &&
                lanes[i].state == RUNNING) {
                found_next = i;
                break;
            }
        }
        
        if (found_next == -1 || found_next == cycle_node) break;
        current = found_next;
    }
    
    // Preempt: force lowest priority lane to release resources
    if (min_priority_lane != -1) {
        // Force release
        release_intersection_quadrants(&lanes[min_priority_lane]);
        lanes[min_priority_lane].state = BLOCKED;
        
        printf("Deadlock resolved: Lane %d preempted\n", min_priority_lane);
    }
}
```

### 2. Queue Overflow Handling

```c
bool add_vehicle_with_overflow_handling(LaneProcess* lane, int vehicle_id) {
    pthread_mutex_lock(&lane->queue_lock);
    {
        if (is_full(lane->queue)) {
            // Option 1: Expand queue (if not at max)
            if (lane->queue->capacity < MAX_QUEUE_CAPACITY * 2) {
                resize_queue(lane->queue, lane->queue->capacity * 1.5);
                printf("Lane %d queue expanded to %d\n", 
                       lane->lane_id, lane->queue->capacity);
            }
            
            // Option 2: Reject vehicle (if still full)
            if (is_full(lane->queue)) {
                printf("WARNING: Lane %d queue full, vehicle %d rejected\n", 
                       lane->lane_id, vehicle_id);
                pthread_mutex_unlock(&lane->queue_lock);
                return false;
            }
        }
        
        // Enqueue
        if (enqueue(lane->queue, vehicle_id)) {
            lane->queue_length++;
            if (lane->state == WAITING) {
                lane->state = READY;
            }
            pthread_mutex_unlock(&lane->queue_lock);
            return true;
        }
    }
    pthread_mutex_unlock(&lane->queue_lock);
    return false;
}
```

---

## Performance Optimization Techniques

### 1. Batch Processing Optimization

**Instead of**: One vehicle at a time (high context switches)
```c
// Inefficient (causes many context switches)
while (!is_empty(lane->queue)) {
    process_one_vehicle(lane);  // Single vehicle
    // Light changes to next lane
}
```

**Use**: Process multiple vehicles in batch
```c
#define BATCH_EXIT_SIZE 3

void execute_lane_time_slice(LaneProcess* lane, int time_quantum) {
    // Process up to 3 vehicles per time slice
    for (int i = 0; i < BATCH_EXIT_SIZE; i++) {
        if (is_empty(lane->queue)) break;
        
        int vehicle_id = remove_vehicle_from_lane(lane);
        usleep(VEHICLE_CROSS_TIME * 1000000);
        lane->total_vehicles_served++;
    }
}
```

**Benefits**:
```
Without batching:     1 vehicle per time slice × 200 vehicles = 200 switches
With batching (3x):   3 vehicles per time slice × 200 vehicles ≈ 67 switches
Result:               3× fewer context switches = 3× less overhead
```

### 2. Lock-free Read Operations

```c
// Before: Acquire lock for read (unnecessary)
int get_lane_queue_length(LaneProcess* lane) {
    pthread_mutex_lock(&lane->queue_lock);
    int len = lane->queue_length;
    pthread_mutex_unlock(&lane->queue_lock);
    return len;
}

// After: Atomic read (no lock for simple integer)
// On x86_64, integer read is atomic
int get_lane_queue_length(LaneProcess* lane) {
    return lane->queue_length;  // Safe to read without lock
}
```

### 3. Scheduler Optimization (Quick Path)

```c
int schedule_next_lane_optimized(Scheduler* scheduler, LaneProcess lanes[4]) {
    // Fast path: current lane still has vehicles
    if (scheduler->current_lane != -1) {
        LaneProcess* current = &lanes[scheduler->current_lane];
        
        if (!is_empty(current->queue) && 
            current->state != BLOCKED) {
            // Continue current lane (no context switch)
            return scheduler->current_lane;
        }
    }
    
    // Slow path: must choose new lane (lock required)
    pthread_mutex_lock(&scheduler->scheduler_lock);
    {
        int next = schedule_next_lane_sjf(scheduler, lanes);
        pthread_mutex_unlock(&scheduler->scheduler_lock);
        return next;
    }
}
```

---

## Testing & Debugging

### 1. Unit Test: Lane Operations

```c
void test_lane_operations() {
    // Create test lane
    LaneProcess lane;
    init_lane_process(&lane, 0, 10);
    
    // Test 1: Add vehicle
    assert(lane.queue_length == 0);
    assert(lane.state == WAITING);
    
    add_vehicle_to_lane(&lane, 1);
    assert(lane.queue_length == 1);
    assert(lane.state == READY);
    
    // Test 2: Remove vehicle
    int vehicle = remove_vehicle_from_lane(&lane);
    assert(vehicle == 1);
    assert(lane.queue_length == 0);
    assert(lane.state == WAITING);
    
    // Test 3: Multiple vehicles
    for (int i = 0; i < 5; i++) {
        add_vehicle_to_lane(&lane, i);
    }
    assert(lane.queue_length == 5);
    
    // Test 4: Queue capacity
    for (int i = 5; i < 10; i++) {
        add_vehicle_to_lane(&lane, i);
    }
    assert(lane.queue_length == 10);  // Full
    
    // Try to add beyond capacity
    add_vehicle_to_lane(&lane, 100);  // Should be rejected
    assert(lane.queue_length == 10);  // No change
    
    printf("✓ Lane operations test passed\n");
    destroy_lane_process(&lane);
}
```

### 2. Integration Test: Scheduler Decision

```c
void test_scheduler_decisions() {
    // Setup: Create system with specific queue lengths
    TrafficGuruSystem system;
    init_traffic_guru_system();
    
    // Set lane queue lengths
    g_traffic_system->lanes[0].queue_length = 5;
    g_traffic_system->lanes[1].queue_length = 2;
    g_traffic_system->lanes[2].queue_length = 1;
    g_traffic_system->lanes[3].queue_length = 3;
    
    // Set all to READY
    for (int i = 0; i < NUM_LANES; i++) {
        g_traffic_system->lanes[i].state = READY;
    }
    
    // Test SJF: Should choose Lane 2 (smallest: 1 vehicle)
    set_scheduling_algorithm(&g_traffic_system->scheduler, SJF);
    int result = schedule_next_lane(&g_traffic_system->scheduler,
                                   g_traffic_system->lanes);
    assert(result == 2);
    printf("✓ SJF chose lane with shortest queue\n");
    
    // Test Priority RR: Set priorities
    for (int i = 0; i < NUM_LANES; i++) {
        g_traffic_system->lanes[i].priority = i;  // 0=high, 3=low
    }
    set_scheduling_algorithm(&g_traffic_system->scheduler, PRIORITY_ROUND_ROBIN);
    result = schedule_next_lane(&g_traffic_system->scheduler,
                               g_traffic_system->lanes);
    assert(result == 0);  // Highest priority
    printf("✓ Priority RR chose highest priority lane\n");
    
    destroy_traffic_guru_system();
}
```

### 3. Stress Test: Concurrent Operations

```c
void stress_test_concurrent_access() {
    init_traffic_guru_system();
    
    int num_threads = 8;
    pthread_t threads[num_threads];
    
    // Spawn multiple threads adding/removing vehicles
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, stress_thread_func, NULL);
    }
    
    // Wait for completion
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("✓ Stress test completed - no deadlocks/corruption\n");
    destroy_traffic_guru_system();
}

void* stress_thread_func(void* arg) {
    // Each thread: add 100 vehicles, remove 100 vehicles
    for (int i = 0; i < 100; i++) {
        int lane_id = rand() % NUM_LANES;
        add_vehicle_to_lane(&g_traffic_system->lanes[lane_id], rand());
        
        if (rand() % 2 == 0) {
            remove_vehicle_from_lane(&g_traffic_system->lanes[lane_id]);
        }
    }
    
    return NULL;
}
```

### 4. Performance Profiling

```c
void profile_scheduling_performance() {
    // Measure algorithm performance
    
    clock_t start, end;
    double cpu_time;
    
    // SJF Performance
    start = clock();
    for (int i = 0; i < 10000; i++) {
        schedule_next_lane_sjf(&g_traffic_system->scheduler,
                              g_traffic_system->lanes);
    }
    end = clock();
    cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
    printf("SJF 10000 iterations: %.2f ms (%.4f ms/iteration)\n", 
           cpu_time, cpu_time / 10000);
    
    // Multilevel Performance
    start = clock();
    for (int i = 0; i < 10000; i++) {
        schedule_next_lane_multilevel(&g_traffic_system->scheduler,
                                     g_traffic_system->lanes);
    }
    end = clock();
    cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
    printf("Multilevel 10000 iterations: %.2f ms (%.4f ms/iteration)\n", 
           cpu_time, cpu_time / 10000);
    
    // Priority RR Performance
    start = clock();
    for (int i = 0; i < 10000; i++) {
        schedule_next_lane_priority_rr(&g_traffic_system->scheduler,
                                      g_traffic_system->lanes);
    }
    end = clock();
    cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
    printf("Priority RR 10000 iterations: %.2f ms (%.4f ms/iteration)\n", 
           cpu_time, cpu_time / 10000);
}
```

---

This implementation guide shows the practical coding patterns used throughout TrafficGuru,
including optimization techniques, error handling, and testing strategies.
