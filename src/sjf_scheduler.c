#include "../include/scheduler.h"
#include "../include/lane_process.h"
#include <stdlib.h>
#include <limits.h>
#include <float.h>

// Shortest Job First scheduling algorithm
int schedule_next_lane_sjf(Scheduler* scheduler, LaneProcess lanes[4]) {
    if (!scheduler || !lanes) {
        return -1;
    }

    int best_lane = -1;
    int min_estimated_time = INT_MAX;
    time_t earliest_arrival = time(NULL);

    // Find lane with shortest estimated processing time
    for (int i = 0; i < 4; i++) {
        if (is_lane_ready(&lanes[i]) && !is_lane_blocked(&lanes[i])) {
            int queue_length = get_lane_queue_length(&lanes[i]);
            int estimated_time = queue_length * VEHICLE_CROSS_TIME;

            // Select lane with minimum estimated time
            if (estimated_time < min_estimated_time) {
                min_estimated_time = estimated_time;
                best_lane = i;
                earliest_arrival = lanes[i].last_arrival_time;
            } else if (estimated_time == min_estimated_time) {
                // Tie breaker: use FIFO order (earliest arrival)
                if (lanes[i].last_arrival_time < earliest_arrival) {
                    best_lane = i;
                    earliest_arrival = lanes[i].last_arrival_time;
                }
            }
        }
    }

    return best_lane;
}

// SJF variant: Preemptive Shortest Remaining Time First
int schedule_next_lane_srtf(Scheduler* scheduler, LaneProcess lanes[4]) {
    if (!scheduler || !lanes) {
        return -1;
    }

    int best_lane = -1;
    int min_remaining_time = INT_MAX;

    // Find lane with shortest remaining time
    for (int i = 0; i < 4; i++) {
        if (is_lane_ready(&lanes[i]) && !is_lane_blocked(&lanes[i])) {
            int queue_length = get_lane_queue_length(&lanes[i]);
            int remaining_time = queue_length * VEHICLE_CROSS_TIME;

            if (remaining_time < min_remaining_time) {
                min_remaining_time = remaining_time;
                best_lane = i;
            }
        }
    }

    return best_lane;
}

// SJF with aging to prevent starvation
int schedule_next_lane_sjf_with_aging(Scheduler* scheduler, LaneProcess lanes[4]) {
    if (!scheduler || !lanes) {
        return -1;
    }

    int best_lane = -1;
    float min_priority_score = FLT_MAX;

    for (int i = 0; i < 4; i++) {
        if (is_lane_ready(&lanes[i]) && !is_lane_blocked(&lanes[i])) {
            int queue_length = get_lane_queue_length(&lanes[i]);
            int waiting_time = lanes[i].waiting_time;

            // Calculate priority score: estimated_time - aging_factor
            float estimated_time = queue_length * VEHICLE_CROSS_TIME;
            float aging_factor = waiting_time * 0.1f; // 10% aging bonus
            float priority_score = estimated_time - aging_factor;

            if (priority_score < min_priority_score) {
                min_priority_score = priority_score;
                best_lane = i;
            }
        }
    }

    return best_lane;
}

// Enhanced SJF with multiple factors
int schedule_next_lane_enhanced_sjf(Scheduler* scheduler, LaneProcess lanes[4]) {
    if (!scheduler || !lanes) {
        return -1;
    }

    int best_lane = -1;
    float min_weighted_score = FLT_MAX;

    for (int i = 0; i < 4; i++) {
        if (is_lane_ready(&lanes[i]) && !is_lane_blocked(&lanes[i])) {
            int queue_length = get_lane_queue_length(&lanes[i]);
            int waiting_time = lanes[i].waiting_time;
            float avg_wait = get_lane_average_wait_time(&lanes[i]);

            // Calculate weighted score considering multiple factors
            float processing_time = queue_length * VEHICLE_CROSS_TIME;
            float waiting_bonus = waiting_time * 0.2f; // 20% waiting bonus
            float fairness_penalty = avg_wait * 0.1f;  // 10% fairness penalty

            float weighted_score = processing_time - waiting_bonus + fairness_penalty;

            if (weighted_score < min_weighted_score) {
                min_weighted_score = weighted_score;
                best_lane = i;
            }
        }
    }

    return best_lane;
}

// SJF with burst time prediction
int schedule_next_lane_predictive_sjf(Scheduler* scheduler, LaneProcess lanes[4]) {
    if (!scheduler || !lanes) {
        return -1;
    }

    int best_lane = -1;
    float min_predicted_time = FLT_MAX;

    for (int i = 0; i < 4; i++) {
        if (is_lane_ready(&lanes[i]) && !is_lane_blocked(&lanes[i])) {
            int queue_length = get_lane_queue_length(&lanes[i]);
            int throughput = get_lane_throughput(&lanes[i]);

            // Predict processing time based on historical throughput
            float avg_service_time = throughput > 0 ? (60.0f / throughput) : VEHICLE_CROSS_TIME;
            float predicted_time = queue_length * avg_service_time;

            if (predicted_time < min_predicted_time) {
                min_predicted_time = predicted_time;
                best_lane = i;
            }
        }
    }

    return best_lane;
}