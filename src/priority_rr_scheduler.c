#include "../include/scheduler.h"
#include "../include/lane_process.h"
#include "../include/emergency_system.h"
#include "../include/trafficguru.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Priority levels for traffic scheduling
typedef enum {
    PRIORITY_EMERGENCY = 1,    // Emergency vehicles
    PRIORITY_NORMAL = 2,       // Normal traffic lanes with > 3 vehicles
    PRIORITY_LOW = 3           // Normal traffic lanes with ≤ 3 vehicles
} TrafficPriority;

// Lane tracking for Round Robin
typedef struct {
    int lane_id;
    TrafficPriority priority;
    time_t last_service_time;
    int service_count;
    bool in_ready_queue;
} LaneRRInfo;

// Global Round Robin tracking
static LaneRRInfo lane_rr_info[4];
static bool rr_initialized = false;
static int current_round_robin_index = 0;

// Round Robin time quantum
#define RR_TIME_QUANTUM 3  // seconds for all priority levels

// Initialize Round Robin tracking
void init_round_robin_tracking() {
    if (rr_initialized) {
        return;
    }

    time_t current_time = time(NULL);

    for (int i = 0; i < 4; i++) {
        lane_rr_info[i].lane_id = i;
        lane_rr_info[i].priority = PRIORITY_NORMAL;
        lane_rr_info[i].last_service_time = current_time;
        lane_rr_info[i].service_count = 0;
        lane_rr_info[i].in_ready_queue = false;
    }

    current_round_robin_index = 0;
    rr_initialized = true;
}

// Update lane priority based on conditions
void update_lane_rr_priority(LaneProcess* lane) {
    if (!lane || !rr_initialized) {
        return;
    }

    LaneRRInfo* rr_info = &lane_rr_info[lane->lane_id];
    int queue_length = get_lane_queue_length(lane);

    // Check for emergency vehicles (highest priority)
    // In a real implementation, this would check with emergency system
    if (lane->priority == 1) { // Emergency priority set externally
        rr_info->priority = PRIORITY_EMERGENCY;
    } else if (queue_length > 3) {
        rr_info->priority = PRIORITY_NORMAL;
    } else {
        rr_info->priority = PRIORITY_LOW;
    }
}

// Get next lane in Round Robin order for specific priority
int get_next_rr_lane(LaneProcess lanes[4], TrafficPriority priority) {
    if (!rr_initialized) {
        return -1;
    }

    int start_index = current_round_robin_index;
    int lanes_checked = 0;

    while (lanes_checked < 4) {
        int lane_id = start_index % 4;

        if (is_lane_ready(&lanes[lane_id]) &&
            !is_lane_blocked(&lanes[lane_id]) &&
            lane_rr_info[lane_id].priority == priority) {

            current_round_robin_index = (lane_id + 1) % 4;
            return lane_id;
        }

        start_index++;
        lanes_checked++;
    }

    return -1; // No lane found with this priority
}

// Priority Round Robin scheduling algorithm
int schedule_next_lane_priority_rr(Scheduler* scheduler, LaneProcess lanes[4]) {
    if (!scheduler || !lanes) {
        return -1;
    }

    // Initialize Round Robin tracking if not done yet
    if (!rr_initialized) {
        init_round_robin_tracking();
    }

    // Update all lane priorities
    for (int i = 0; i < 4; i++) {
        update_lane_rr_priority(&lanes[i]);
    }

    // Set standard time quantum for Priority Round Robin
    scheduler->time_quantum = RR_TIME_QUANTUM;

    // Process priorities in order: Emergency -> Normal -> Low

    // Priority 1: Emergency vehicles
    int emergency_lane = get_next_rr_lane(lanes, PRIORITY_EMERGENCY);
    if (emergency_lane != -1) {
        return emergency_lane;
    }

    // Priority 2: Normal traffic lanes
    int normal_lane = get_next_rr_lane(lanes, PRIORITY_NORMAL);
    if (normal_lane != -1) {
        return normal_lane;
    }

    // Priority 3: Low traffic lanes
    int low_lane = get_next_rr_lane(lanes, PRIORITY_LOW);
    if (low_lane != -1) {
        return low_lane;
    }

    // No ready lanes found
    return -1;
}

// Handle emergency vehicle preemption
int preempt_for_emergency_rr(Scheduler* scheduler, LaneProcess lanes[4], int emergency_lane_id) {
    if (!scheduler || !lanes || emergency_lane_id < 0 || emergency_lane_id >= 4) {
        return -1;
    }

    // Set emergency lane to highest priority
    lane_rr_info[emergency_lane_id].priority = PRIORITY_EMERGENCY;
    lanes[emergency_lane_id].priority = 1;

    // Immediately select emergency lane
    scheduler->time_quantum = RR_TIME_QUANTUM;
    current_round_robin_index = emergency_lane_id;

    return emergency_lane_id;
}

// Clear emergency priority after vehicle passes
void clear_emergency_priority(int lane_id) {
    if (!rr_initialized || lane_id < 0 || lane_id >= 4) {
        return;
    }

    lane_rr_info[lane_id].priority = PRIORITY_NORMAL;
}

// Enhanced Priority Round Robin with fairness considerations
int schedule_next_lane_priority_rr_fair(Scheduler* scheduler, LaneProcess lanes[4]) {
    if (!scheduler || !lanes) {
        return -1;
    }

    if (!rr_initialized) {
        init_round_robin_tracking();
    }

    // Update lane priorities
    for (int i = 0; i < 4; i++) {
        update_lane_rr_priority(&lanes[i]);
    }

    time_t current_time = time(NULL);
    scheduler->time_quantum = RR_TIME_QUANTUM;

    // Check for lanes that haven't been served recently (fairness)
    for (int i = 0; i < 4; i++) {
        time_t time_since_service = current_time - lane_rr_info[i].last_service_time;

        // Boost priority if lane hasn't been served for too long
        if (time_since_service > 30 && lane_rr_info[i].priority == PRIORITY_LOW) {
            lane_rr_info[i].priority = PRIORITY_NORMAL;
        }
    }

    // Process with standard priority order
    return schedule_next_lane_priority_rr(scheduler, lanes);
}

// Update lane service information
void update_lane_service_info(int lane_id) {
    if (!rr_initialized || lane_id < 0 || lane_id >= 4) {
        return;
    }

    lane_rr_info[lane_id].last_service_time = time(NULL);
    lane_rr_info[lane_id].service_count++;
}

// Get lane service statistics
void get_lane_service_stats(int lane_id, time_t* last_service, int* service_count) {
    if (!rr_initialized || lane_id < 0 || lane_id >= 4) {
        if (last_service) *last_service = 0;
        if (service_count) *service_count = 0;
        return;
    }

    if (last_service) *last_service = lane_rr_info[lane_id].last_service_time;
    if (service_count) *service_count = lane_rr_info[lane_id].service_count;
}

// Reset Round Robin tracking
void reset_round_robin_tracking() {
    rr_initialized = false;
    current_round_robin_index = 0;
    init_round_robin_tracking();
}

// Print Round Robin information for debugging
void print_round_robin_info() {
    if (!rr_initialized) {
        printf("Round Robin tracking not initialized.\n");
        return;
    }

    printf("\n=== ROUND ROBIN INFO ===\n");
    const char* priority_names[] = {"", "EMERGENCY", "NORMAL", "LOW"};

    for (int i = 0; i < 4; i++) {
        LaneRRInfo* info = &lane_rr_info[i];
        time_t time_since_service = time(NULL) - info->last_service_time;

        printf("Lane %d: Priority=%s, Service Count=%d, Last Service=%lds ago\n",
               i, priority_names[info->priority],
               info->service_count, time_since_service);
    }
    printf("Current RR Index: %d\n\n", current_round_robin_index);
}

// Adaptive Priority Round Robin with dynamic time quantum
int schedule_next_lane_adaptive_priority_rr(Scheduler* scheduler, LaneProcess lanes[4]) {
    if (!scheduler || !lanes) {
        return -1;
    }

    if (!rr_initialized) {
        init_round_robin_tracking();
    }

    // Calculate system load
    int total_vehicles = 0;
    int ready_lanes = 0;

    for (int i = 0; i < 4; i++) {
        if (is_lane_ready(&lanes[i])) {
            total_vehicles += get_lane_queue_length(&lanes[i]);
            ready_lanes++;
        }
    }

    // Adjust time quantum based on system load
    if (ready_lanes > 0) {
        float avg_queue_length = (float)total_vehicles / ready_lanes;

        if (avg_queue_length > 8) {
            scheduler->time_quantum = 2; // Shorter quantum for heavy load
        } else if (avg_queue_length < 2) {
            scheduler->time_quantum = 4; // Longer quantum for light load
        } else {
            scheduler->time_quantum = RR_TIME_QUANTUM; // Default
        }
    }

    // Use standard priority scheduling with adaptive time quantum
    return schedule_next_lane_priority_rr(scheduler, lanes);
}