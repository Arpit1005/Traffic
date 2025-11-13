#include "../include/scheduler.h"
#include "../include/lane_process.h"
#include "../include/trafficguru.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

// Priority levels for multilevel feedback queue
typedef enum {
    PRIORITY_HIGH = 0,
    PRIORITY_MEDIUM = 1,
    PRIORITY_LOW = 2,
    NUM_PRIORITY_LEVELS = 3
} PriorityLevel;

// Lane priority tracking
typedef struct {
    int lane_id;
    PriorityLevel current_priority;
    int consecutive_runs;
    time_t last_promotion;
    time_t last_demotion;
    int time_in_current_level;
} LanePriorityInfo;

// Global priority tracking for lanes
static LanePriorityInfo lane_priorities[4];
static bool priorities_initialized = false;
static pthread_mutex_t priority_lock = PTHREAD_MUTEX_INITIALIZER;  // Thread safety for priority updates

// Priority thresholds
#define PROMOTION_THRESHOLD 10    // Waiting time in seconds for promotion
#define DEMOTION_THRESHOLD 5       // Consecutive runs for demotion (increased for better traffic flow
#define AGING_THRESHOLD 15       // Maximum wait time before forced promotion
// Time quantum per priority level
static const int time_quanta[NUM_PRIORITY_LEVELS] = {2, 4, 6}; // seconds

// Initialize priority tracking
void init_lane_priorities() {
    if (priorities_initialized) {
        return;
    }

    time_t current_time = time(NULL);

    for (int i = 0; i < 4; i++) {
        lane_priorities[i].lane_id = i;
        lane_priorities[i].current_priority = PRIORITY_MEDIUM; // Start at medium
        lane_priorities[i].consecutive_runs = 0;
        lane_priorities[i].last_promotion = current_time;
        lane_priorities[i].last_demotion = current_time;
        lane_priorities[i].time_in_current_level = 0;
    }

    priorities_initialized = true;
}

// Update lane priorities based on behavior
void update_lane_priority(LaneProcess* lane) {
        pthread_mutex_lock(&priority_lock);  // Protect priority updates
    if (!lane || !priorities_initialized) {
        pthread_mutex_unlock(&priority_lock);
            return;}

    LanePriorityInfo* priority_info = &lane_priorities[lane->lane_id];
    time_t current_time = time(NULL);

    // Update time in current priority level
    priority_info->time_in_current_level = current_time - priority_info->last_promotion;

    // Check for promotion (lane has waited too long)
    if (lane->waiting_time > PROMOTION_THRESHOLD &&
        priority_info->current_priority > PRIORITY_HIGH) {

        priority_info->current_priority--;
        priority_info->last_promotion = current_time;
        priority_info->consecutive_runs = 0;
        lane->priority = priority_info->current_priority + 1; // Convert to 1-based
    }

    // Check for aging (prevent starvation)
    if (priority_info->time_in_current_level > AGING_THRESHOLD &&
        priority_info->current_priority > PRIORITY_HIGH) {

        priority_info->current_priority = PRIORITY_HIGH;
        priority_info->last_promotion = current_time;
        priority_info->consecutive_runs = 0;
        lane->priority = 1;
    }

    // Update consecutive runs counter
    if (lane->state == RUNNING) {
        priority_info->consecutive_runs++;

        // Check for demotion (lane has run too long)
        if (priority_info->consecutive_runs > DEMOTION_THRESHOLD &&
            priority_info->current_priority < PRIORITY_LOW) {

            priority_info->current_priority++;
            priority_info->last_demotion = current_time;
            priority_info->consecutive_runs = 0;
            lane->priority = priority_info->current_priority + 1;
        }
    } else {
        // Reset consecutive runs when lane is not running
        priority_info->consecutive_runs = 0;
    }
    pthread_mutex_unlock(&priority_lock);  // Release lock before returning
}

// Multilevel Feedback Queue scheduling algorithm
int schedule_next_lane_multilevel(Scheduler* scheduler, LaneProcess lanes[4]) {
    if (!scheduler || !lanes) {
        return -1;
    }

    // Initialize priorities if not done yet
    if (!priorities_initialized) {
        init_lane_priorities();
    }

    // Update all lane priorities
    for (int i = 0; i < 4; i++) {
        update_lane_priority(&lanes[i]);
    }

    // Check each priority level from highest to lowest
    for (PriorityLevel level = PRIORITY_HIGH; level < NUM_PRIORITY_LEVELS; level++) {
        int best_lane_in_level = -1;
        int max_wait_time = -1;

        // Find best lane in this priority level
        for (int i = 0; i < 4; i++) {
            if (is_lane_ready(&lanes[i]) &&
                !is_lane_blocked(&lanes[i]) &&
                lane_priorities[i].current_priority == level) {

                // Within same priority level, use Round Robin based on waiting time
                if (lanes[i].waiting_time > max_wait_time) {
                    max_wait_time = lanes[i].waiting_time;
                    best_lane_in_level = i;
                }
            }
        }

        // If we found a lane in this level, return it
        if (best_lane_in_level != -1) {
            // Set appropriate time quantum for this priority level
            scheduler->time_quantum = time_quanta[level];
            return best_lane_in_level;
        }
    }

    // No ready lanes found
    return -1;
}

// Get time quantum for a specific lane
int get_time_quantum_for_lane(int lane_id) {
    if (!priorities_initialized || lane_id < 0 || lane_id >= 4) {
        return DEFAULT_TIME_QUANTUM;
    }

    PriorityLevel priority = lane_priorities[lane_id].current_priority;
    return time_quanta[priority];
}

// Promote lane to higher priority
void promote_lane(int lane_id) {
    if (!priorities_initialized || lane_id < 0 || lane_id >= 4) {
        return;
    }

    LanePriorityInfo* priority_info = &lane_priorities[lane_id];

    if (priority_info->current_priority > PRIORITY_HIGH) {
        priority_info->current_priority--;
        priority_info->last_promotion = time(NULL);
        priority_info->consecutive_runs = 0;
    }
}

// Demote lane to lower priority
void demote_lane(int lane_id) {
    if (!priorities_initialized || lane_id < 0 || lane_id >= 4) {
        return;
    }

    LanePriorityInfo* priority_info = &lane_priorities[lane_id];

    if (priority_info->current_priority < PRIORITY_LOW) {
        priority_info->current_priority++;
        priority_info->last_demotion = time(NULL);
        priority_info->consecutive_runs = 0;
    }
}

// Get current priority of a lane
PriorityLevel get_lane_priority(int lane_id) {
    if (!priorities_initialized || lane_id < 0 || lane_id >= 4) {
        return PRIORITY_MEDIUM;
    }

    return lane_priorities[lane_id].current_priority;
}

// Reset lane priorities to initial state
void reset_lane_priorities() {
    priorities_initialized = false;
    init_lane_priorities();
}

// Print priority information for debugging
void print_lane_priorities() {
    if (!priorities_initialized) {
        printf("Lane priorities not initialized.\n");
        return;
    }

    printf("\n=== LANE PRIORITIES ===\n");
    const char* priority_names[] = {"HIGH", "MEDIUM", "LOW"};

    for (int i = 0; i < 4; i++) {
        LanePriorityInfo* info = &lane_priorities[i];
        printf("Lane %d: Priority=%s, Consecutive Runs=%d, Time in Level=%ds\n",
               i, priority_names[info->current_priority],
               info->consecutive_runs, info->time_in_current_level);
    }
    printf("\n");
}

// Enhanced multilevel feedback with dynamic thresholds
int schedule_next_lane_adaptive_multilevel(Scheduler* scheduler, LaneProcess lanes[4]) {
    if (!scheduler || !lanes) {
        return -1;
    }

    // Initialize priorities if not done yet
    if (!priorities_initialized) {
        init_lane_priorities();
    }

    // Calculate system load and adjust thresholds dynamically
    int total_queue_length = 0;
    int ready_lanes = 0;

    for (int i = 0; i < 4; i++) {
        total_queue_length += get_lane_queue_length(&lanes[i]);
        if (is_lane_ready(&lanes[i])) {
            ready_lanes++;
        }
    }

    // Adjust thresholds based on system load
    float system_load = ready_lanes > 0 ? (float)total_queue_length / ready_lanes : 0;
    int adaptive_promotion_threshold = system_load > 5 ? PROMOTION_THRESHOLD / 2 : PROMOTION_THRESHOLD;
    int adaptive_demotion_threshold = system_load < 2 ? DEMOTION_THRESHOLD * 2 : DEMOTION_THRESHOLD;

    // Update priorities with adaptive thresholds
    for (int i = 0; i < 4; i++) {
        if (lanes[i].waiting_time > adaptive_promotion_threshold &&
            lane_priorities[i].current_priority > PRIORITY_HIGH) {
            promote_lane(i);
        }

        if (lane_priorities[i].consecutive_runs > adaptive_demotion_threshold &&
            lane_priorities[i].current_priority < PRIORITY_LOW) {
            demote_lane(i);
        }
    }

    // Use standard multilevel scheduling with updated priorities
    return schedule_next_lane_multilevel(scheduler, lanes);
}
