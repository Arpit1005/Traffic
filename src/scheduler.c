#define _XOPEN_SOURCE 600
#include "../include/scheduler.h"
#include "../include/lane_process.h"
#include "../include/trafficguru.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

// Algorithm names for display
static const char* algorithm_names[] = {
    "Shortest Job First",
    "Multilevel Feedback Queue",
    "Priority Round Robin"
};

// Initialize scheduler
void init_scheduler(Scheduler* scheduler, SchedulingAlgorithm algorithm) {
    if (!scheduler) {
        return;
    }

    scheduler->algorithm = algorithm;
    scheduler->ready_queue = create_queue(20); // Support up to 20 lanes in ready state
    scheduler->time_quantum = DEFAULT_TIME_QUANTUM;
    scheduler->context_switch_time = CONTEXT_SWITCH_TIME;
    scheduler->current_lane = -1; // No lane currently selected
    scheduler->execution_history = NULL;
    scheduler->history_size = 1000; // Keep last 1000 execution records
    scheduler->history_index = 0;
    scheduler->total_context_switches = 0;
    scheduler->last_schedule_time = time(NULL);
    scheduler->scheduler_running = false;

    // Allocate execution history buffer
    scheduler->execution_history = (ExecutionRecord*)malloc(
        scheduler->history_size * sizeof(ExecutionRecord));
    if (!scheduler->execution_history) {
        scheduler->history_size = 0;
    }

    // Initialize synchronization primitives
    pthread_mutex_init(&scheduler->scheduler_lock, NULL);
    pthread_cond_init(&scheduler->scheduler_cond, NULL);
}

// Destroy scheduler and cleanup resources
void destroy_scheduler(Scheduler* scheduler) {
    if (!scheduler) {
        return;
    }

    if (scheduler->ready_queue) {
        destroy_queue(scheduler->ready_queue);
        scheduler->ready_queue = NULL;
    }

    if (scheduler->execution_history) {
        free(scheduler->execution_history);
        scheduler->execution_history = NULL;
    }

    pthread_mutex_destroy(&scheduler->scheduler_lock);
    pthread_cond_destroy(&scheduler->scheduler_cond);
}

// Start scheduler
void start_scheduler(Scheduler* scheduler) {
    if (!scheduler) {
        return;
    }

    pthread_mutex_lock(&scheduler->scheduler_lock);
    scheduler->scheduler_running = true;
    scheduler->last_schedule_time = time(NULL);
    pthread_cond_signal(&scheduler->scheduler_cond);
    pthread_mutex_unlock(&scheduler->scheduler_lock);
}

// Stop scheduler
void stop_scheduler(Scheduler* scheduler) {
    if (!scheduler) {
        return;
    }

    pthread_mutex_lock(&scheduler->scheduler_lock);
    scheduler->scheduler_running = false;
    pthread_cond_signal(&scheduler->scheduler_cond);
    pthread_mutex_unlock(&scheduler->scheduler_lock);
}

// Main scheduling function - delegates to specific algorithm
int schedule_next_lane(Scheduler* scheduler, LaneProcess lanes[4]) {
    if (!scheduler || !lanes) {
        return -1;
    }

    pthread_mutex_lock(&scheduler->scheduler_lock);

    int next_lane = -1;

    switch (scheduler->algorithm) {
        case SJF:
            next_lane = schedule_next_lane_sjf(scheduler, lanes);
            break;
        case MULTILEVEL_FEEDBACK:
            next_lane = schedule_next_lane_multilevel(scheduler, lanes);
            break;
        case PRIORITY_ROUND_ROBIN:
            next_lane = schedule_next_lane_priority_rr(scheduler, lanes);
            break;
        default:
            next_lane = schedule_next_lane_sjf(scheduler, lanes);
            break;
    }

    // Perform context switch if needed
    if (next_lane != scheduler->current_lane && next_lane != -1) {
        context_switch(scheduler,
                      scheduler->current_lane >= 0 ? &lanes[scheduler->current_lane] : NULL,
                      &lanes[next_lane]);
        scheduler->current_lane = next_lane;
    }

    scheduler->last_schedule_time = time(NULL);
    pthread_mutex_unlock(&scheduler->scheduler_lock);

    return next_lane;
}

// Execute time slice for selected lane
void execute_lane_time_slice(Scheduler* scheduler, LaneProcess* lane, int time_quantum) {
    if (!scheduler || !lane) {
        return;
    }

    time_t start_time = time(NULL);
    int initial_vehicles = get_lane_queue_length(lane);

    // Set lane to running state
    update_lane_state(lane, RUNNING);

    // Execute for specified time quantum
    usleep(time_quantum * 1000000 / 10); // Simulate time quantum

    time_t end_time = time(NULL);
    int vehicles_processed = initial_vehicles - get_lane_queue_length(lane);

    // Record execution
    record_execution(scheduler, lane->lane_id, start_time, end_time, vehicles_processed);

    // Set lane back to ready if vehicles remain, waiting if empty
    if (get_lane_queue_length(lane) > 0) {
        update_lane_state(lane, READY);
    } else {
        update_lane_state(lane, WAITING);
    }
}

// Perform context switch between lanes
void context_switch(Scheduler* scheduler, LaneProcess* from_lane, LaneProcess* to_lane) {
    if (!scheduler) {
        return;
    }

    // Stop previous lane if exists
    if (from_lane) {
        update_lane_state(from_lane, READY);
    }

    // Start new lane if exists
    if (to_lane) {
        update_lane_state(to_lane, RUNNING);
    }

    scheduler->total_context_switches++;

    // Simulate context switch overhead
    usleep(scheduler->context_switch_time * 1000);
}

// Set scheduling algorithm
void set_scheduling_algorithm(Scheduler* scheduler, SchedulingAlgorithm algorithm) {
    if (!scheduler) {
        return;
    }

    pthread_mutex_lock(&scheduler->scheduler_lock);
    scheduler->algorithm = algorithm;
    scheduler->current_lane = -1; // Reset current lane on algorithm change
    pthread_mutex_unlock(&scheduler->scheduler_lock);
}

// Get current scheduling algorithm
SchedulingAlgorithm get_scheduling_algorithm(Scheduler* scheduler) {
    return scheduler ? scheduler->algorithm : SJF;
}

// Get algorithm name
const char* get_algorithm_name(SchedulingAlgorithm algorithm) {
    if (algorithm >= 0 && algorithm < 3) {
        return algorithm_names[algorithm];
    }
    return "Unknown";
}

// Record execution in history
void record_execution(Scheduler* scheduler, int lane_id, time_t start_time,
                      time_t end_time, int vehicles_processed) {
    if (!scheduler || !scheduler->execution_history) {
        return;
    }

    pthread_mutex_lock(&scheduler->scheduler_lock);

    ExecutionRecord* record = &scheduler->execution_history[scheduler->history_index];
    record->lane_id = lane_id;
    record->start_time = start_time;
    record->end_time = end_time;
    record->duration = (int)(end_time - start_time);
    record->vehicles_processed = vehicles_processed;

    scheduler->history_index = (scheduler->history_index + 1) % scheduler->history_size;

    pthread_mutex_unlock(&scheduler->scheduler_lock);
}

// Print execution history
void print_execution_history(Scheduler* scheduler) {
    if (!scheduler || !scheduler->execution_history) {
        printf("No execution history available.\n");
        return;
    }

    printf("\n=== EXECUTION HISTORY ===\n");
    printf("Lane | Start Time | Duration | Vehicles\n");
    printf("-----|------------|----------|----------\n");

    int count = (scheduler->history_index < scheduler->history_size) ?
                scheduler->history_index : scheduler->history_size;

    int start_index = (scheduler->history_index < scheduler->history_size) ? 0 : scheduler->history_index;

    for (int i = 0; i < count; i++) {
        int index = (start_index + i) % scheduler->history_size;
        ExecutionRecord* record = &scheduler->execution_history[index];

        printf("%4d | %10ld | %8ds | %8d\n",
               record->lane_id,
               record->start_time,
               record->duration,
               record->vehicles_processed);
    }
    printf("\n");
}

// Get execution history
ExecutionRecord* get_execution_history(Scheduler* scheduler, int* count) {
    if (!scheduler || !count) {
        return NULL;
    }

    *count = (scheduler->history_index < scheduler->history_size) ?
             scheduler->history_index : scheduler->history_size;

    return scheduler->execution_history;
}

// Calculate average waiting time
float calculate_average_wait_time(Scheduler* scheduler, LaneProcess lanes[4]) {
    if (!scheduler || !lanes) {
        return 0.0f;
    }

    float total_wait = 0.0f;
    int active_lanes = 0;

    for (int i = 0; i < 4; i++) {
        float lane_wait = get_lane_average_wait_time(&lanes[i]);
        if (lane_wait > 0) {
            total_wait += lane_wait;
            active_lanes++;
        }
    }

    return active_lanes > 0 ? (total_wait / active_lanes) : 0.0f;
}

// Calculate throughput
float calculate_throughput(Scheduler* scheduler, time_t time_period) {
    if (!scheduler || time_period <= 0) {
        return 0.0f;
    }

    int total_vehicles = 0;
    int count = 0;
    ExecutionRecord* history = get_execution_history(scheduler, &count);

    for (int i = 0; i < count; i++) {
        total_vehicles += history[i].vehicles_processed;
    }

    double minutes = time_period / 60.0;
    return minutes > 0 ? (total_vehicles / minutes) : 0.0f;
}

// Calculate fairness index (Jain's fairness index)
float calculate_fairness_index(Scheduler* scheduler, LaneProcess lanes[4]) {
    if (!scheduler || !lanes) {
        return 0.0f;
    }

    float wait_times[4];
    float sum_wait = 0.0f, sum_wait_sq = 0.0f;
    int active_lanes = 0;

    for (int i = 0; i < 4; i++) {
        wait_times[i] = get_lane_average_wait_time(&lanes[i]);
        if (wait_times[i] > 0) {
            sum_wait += wait_times[i];
            sum_wait_sq += wait_times[i] * wait_times[i];
            active_lanes++;
        }
    }

    if (sum_wait > 0 && active_lanes > 0) {
        return (sum_wait * sum_wait) / (active_lanes * sum_wait_sq);
    }

    return 1.0f; // Perfect fairness when no waiting
}

// Calculate context switch overhead
int calculate_context_switch_overhead(Scheduler* scheduler) {
    return scheduler ? scheduler->total_context_switches * scheduler->context_switch_time : 0;
}

// Add lane to ready queue
void add_lane_to_ready_queue(Scheduler* scheduler, LaneProcess* lane) {
    if (!scheduler || !lane || !scheduler->ready_queue) {
        return;
    }

    pthread_mutex_lock(&scheduler->scheduler_lock);
    enqueue(scheduler->ready_queue, lane->lane_id);
    pthread_mutex_unlock(&scheduler->scheduler_lock);
}

// Remove lane from ready queue
void remove_lane_from_ready_queue(Scheduler* scheduler, LaneProcess* lane) {
    if (!scheduler || !lane || !scheduler->ready_queue) {
        return;
    }

    // Note: This is a simplified implementation
    // In practice, we'd need to search for the lane in the queue
    pthread_mutex_lock(&scheduler->scheduler_lock);
    dequeue(scheduler->ready_queue); // Remove from front (simplified)
    pthread_mutex_unlock(&scheduler->scheduler_lock);
}

// Get ready queue size
int get_ready_queue_size(Scheduler* scheduler) {
    if (!scheduler || !scheduler->ready_queue) {
        return 0;
    }

    pthread_mutex_lock(&scheduler->scheduler_lock);
    int size = get_size(scheduler->ready_queue);
    pthread_mutex_unlock(&scheduler->scheduler_lock);

    return size;
}

// Check if ready queue is empty
bool is_ready_queue_empty(Scheduler* scheduler) {
    return get_ready_queue_size(scheduler) == 0;
}