/*
 * Traffic Mutex Implementation - Hybrid Deadlock Prevention
 *
 * Implements enhanced intersection access control combining:
 * - POSIX mutexes for basic synchronization
 * - Banker's algorithm for resource safety
 * - Hybrid strategy for optimal deadlock prevention
 *
 * Compilation: Include traffic_mutex.h, synchronization.h, bankers_algorithm.h
 */

#define _XOPEN_SOURCE 600
#include "../include/synchronization.h"
#include "../include/bankers_algorithm.h"
#include "../include/traffic_mutex.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

typedef struct {
    IntersectionMutex* intersection;
    BankersState* bankers;
    bool enhanced_mode;
    int allocation_strategy;
} EnhancedTrafficMutex;

static EnhancedTrafficMutex g_traffic_mutex = {0};
static bool traffic_mutex_initialized = false;

// Initialize enhanced traffic mutex system
void init_traffic_mutex_system() {
    if (traffic_mutex_initialized) {
        return;
    }

    g_traffic_mutex.intersection = get_global_intersection();
    g_traffic_mutex.bankers = get_global_bankers_state();
    g_traffic_mutex.enhanced_mode = true;
    g_traffic_mutex.allocation_strategy = 2; // Hybrid mode

    traffic_mutex_initialized = true;
}

// Enhanced lock acquisition with Banker's algorithm integration
bool acquire_intersection_with_bankers(LaneProcess* lane) {
    if (!lane) {
        return false;
    }

    if (!traffic_mutex_initialized) {
        init_traffic_mutex_system();
    }

    EnhancedTrafficMutex* tm = &g_traffic_mutex;

    // Calculate resources needed for this lane
    int needed_quadrants[NUM_QUADRANTS] = {0};
    calculate_needed_quadrants(lane, needed_quadrants);

    // Set requested quadrants in lane structure
    int requested_mask = 0;
    for (int quad = 0; quad < NUM_QUADRANTS; quad++) {
        if (needed_quadrants[quad] > 0) {
            requested_mask |= (1 << quad);
        }
    }
    lane->requested_quadrants = requested_mask;

    // Different allocation strategies
    bool acquired = false;

    switch (tm->allocation_strategy) {
        case 0: // Simple FIFO
            acquired = acquire_intersection(lane);
            break;

        case 1: // Banker's algorithm only
            if (request_resources(tm->bankers, lane->lane_id, needed_quadrants)) {
                acquired = acquire_intersection(lane);
                if (acquired) {
                    lane->allocated_quadrants = requested_mask;
                } else {
                    // Rollback bankers allocation if intersection acquisition failed
                    deallocate_resources(tm->bankers, lane->lane_id);
                }
            }
            break;

        case 2: // Hybrid approach
            acquired = acquire_intersection_hybrid(lane, needed_quadrants);
            break;

        default:
            acquired = acquire_intersection(lane);
            break;
    }

    return acquired;
}

// Hybrid allocation strategy combining Banker's algorithm with traditional locking
bool acquire_intersection_hybrid(LaneProcess* lane, int needed_quadrants[NUM_QUADRANTS]) {
    EnhancedTrafficMutex* tm = &g_traffic_mutex;

    // First try Banker's algorithm
    if (request_resources(tm->bankers, lane->lane_id, needed_quadrants)) {
        // Banker's says it's safe, try to acquire intersection
        if (acquire_intersection(lane)) {
            lane->allocated_quadrants = lane->requested_quadrants;
            return true;
        } else {
            // Intersection busy, rollback bankers allocation
            deallocate_resources(tm->bankers, lane->lane_id);
            return false;
        }
    } else {
        // Banker's algorithm says unsafe, but check if we can still proceed
        // for time-critical situations (emergency vehicles, etc.)
        if (lane->priority == 1) { // Emergency vehicle
            printf("Emergency override: allowing lane %d despite unsafe state\n", lane->lane_id);
            return acquire_intersection(lane);
        }

        // Check if system is in safe state overall
        if (is_safe_state(tm->bankers)) {
            printf("System safe, proceeding with traditional allocation for lane %d\n", lane->lane_id);
            return acquire_intersection(lane);
        }

        printf("Allocation denied for lane %d: unsafe state detected\n", lane->lane_id);
        return false;
    }
}

// Enhanced release with Banker's algorithm cleanup
void release_intersection_with_bankers(LaneProcess* lane) {
    if (!lane) {
        return;
    }

    if (!traffic_mutex_initialized) {
        init_traffic_mutex_system();
    }

    EnhancedTrafficMutex* tm = &g_traffic_mutex;

    // Release intersection first
    release_intersection(lane);

    // Clean up Banker's algorithm allocation
    if (tm->allocation_strategy == 1 || tm->allocation_strategy == 2) {
        deallocate_resources(tm->bankers, lane->lane_id);
    }

    // Clear lane allocation
    lane->allocated_quadrants = 0;
    lane->requested_quadrants = 0;
}

// Advanced deadlock detection and resolution
bool detect_and_resolve_advanced_deadlock(LaneProcess lanes[4]) {
    if (!lanes) {
        return false;
    }

    if (!traffic_mutex_initialized) {
        init_traffic_mutex_system();
    }

    EnhancedTrafficMutex* tm = &g_traffic_mutex;
    bool deadlock_detected = false;

    // Check for deadlock using multiple methods
    bool traditional_deadlock = detect_deadlock(lanes);
    bool circular_wait = is_circular_wait_detected(lanes);
    bool bankers_unsafe = !is_safe_state(tm->bankers);

    if (traditional_deadlock || circular_wait || bankers_unsafe) {
        deadlock_detected = true;
        printf("Deadlock detected - Traditional: %s, Circular Wait: %s, Banker's Unsafe: %s\n",
               traditional_deadlock ? "Yes" : "No",
               circular_wait ? "Yes" : "No",
               bankers_unsafe ? "Yes" : "No");

        // Attempt resolution
        resolve_advanced_deadlock(lanes);
    }

    return deadlock_detected;
}

// Advanced deadlock resolution strategy
void resolve_advanced_deadlock(LaneProcess lanes[4]) {
    if (!lanes) {
        return;
    }

    EnhancedTrafficMutex* tm = &g_traffic_mutex;

    // Strategy 1: Check for emergency vehicles and prioritize them
    for (int i = 0; i < 4; i++) {
        if (lanes[i].priority == 1 && lanes[i].state == BLOCKED) {
            printf("Emergency deadlock resolution: prioritizing lane %d\n", i);
            lanes[i].state = READY;
            signal_lane(&lanes[i]);
            return;
        }
    }

    // Strategy 2: Use Banker's algorithm to find safe sequence
    bool finish_sequence[NUM_LANES];
    if (safety_algorithm(tm->bankers, finish_sequence)) {
        printf("Found safe sequence using Banker's algorithm\n");
        // Find first lane in safe sequence that's blocked
        for (int i = 0; i < NUM_LANES; i++) {
            int lane_idx = -1;
            for (int j = 0; j < NUM_LANES; j++) {
                if (finish_sequence[j]) {
                    lane_idx = j;
                    break;
                }
            }

            if (lane_idx != -1 && lanes[lane_idx].state == BLOCKED) {
                printf("Unblocking lane %d as part of safe sequence\n", lane_idx);
                lanes[lane_idx].state = READY;
                signal_lane(&lanes[lane_idx]);
                return;
            }
        }
    }

    // Strategy 3: Traditional victim selection (lowest priority)
    resolve_deadlock(lanes);

    // Strategy 4: If still deadlocked, force system reset
    if (detect_deadlock(lanes)) {
        printf("Critical deadlock detected, performing system reset\n");
        reset_intersection_state();
        reset_bankers_state();

        // Mark all lanes as ready
        for (int i = 0; i < 4; i++) {
            lanes[i].state = READY;
            signal_lane(&lanes[i]);
        }
    }
}

// Resource allocation with timeout and retry
bool acquire_intersection_with_timeout(LaneProcess* lane, int timeout_seconds) {
    if (!lane) {
        return false;
    }

    time_t start_time = time(NULL);
    time_t current_time = start_time;

    while ((current_time - start_time) < timeout_seconds) {
        if (acquire_intersection_with_bankers(lane)) {
            return true;
        }

        // Wait before retrying
        usleep(100000); // 100ms
        current_time = time(NULL);
    }

    printf("Timeout: Failed to acquire intersection for lane %d after %d seconds\n",
           lane->lane_id, timeout_seconds);
    return false;
}

// Priority-based acquisition with preemption
bool acquire_intersection_with_preemption(LaneProcess* lane) {
    if (!lane) {
        return false;
    }

    if (!traffic_mutex_initialized) {
        init_traffic_mutex_system();
    }

    EnhancedTrafficMutex* tm = &g_traffic_mutex;
    IntersectionMutex* intersection = tm->intersection;

    pthread_mutex_lock(&intersection->intersection_lock);

    // Check if current lane holder has lower priority
    if (intersection->current_lane != -1 &&
        intersection->current_lane != lane->lane_id) {

        LaneProcess* current_lane = NULL; // This would need access to the global lanes array
        
        // --- FIX: Suppress unused variable warning ---
        (void)current_lane; 
        // --- END FIX ---
        
        // For now, assume we can check priority

        if (lane->priority < 2) { // High priority lane
            printf("Preempting current lane for high priority lane %d\n", lane->lane_id);

            // Force release of current lane
            intersection->intersection_available = true;
            intersection->current_lane = -1;
            intersection->lock_holder = 0;

            // Signal all lanes
            for (int i = 0; i < 4; i++) {
                pthread_cond_signal(&intersection->condition_vars[i]);
            }
        }
    }

    pthread_mutex_unlock(&intersection->intersection_lock);

    // Now try normal acquisition
    return acquire_intersection_with_bankers(lane);
}

// Performance monitoring for the mutex system
typedef struct {
    int total_acquisitions;
    int successful_acquisitions;
    int failed_acquisitions;
    int timeouts;
    int preemptive_acquisitions;
    float average_wait_time;
    time_t monitoring_start_time;
} MutexPerformanceStats;

static MutexPerformanceStats g_perf_stats = {0};

void init_mutex_performance_monitoring() {
    g_perf_stats.total_acquisitions = 0;
    g_perf_stats.successful_acquisitions = 0;
    g_perf_stats.failed_acquisitions = 0;
    g_perf_stats.timeouts = 0;
    g_perf_stats.preemptive_acquisitions = 0;
    g_perf_stats.average_wait_time = 0.0f;
    g_perf_stats.monitoring_start_time = time(NULL);
}

void record_mutex_acquisition(bool success, bool timeout, bool preemptive, float wait_time) {
    g_perf_stats.total_acquisitions++;

    if (success) {
        g_perf_stats.successful_acquisitions++;
        if (preemptive) {
            g_perf_stats.preemptive_acquisitions++;
        }
    } else {
        g_perf_stats.failed_acquisitions++;
        if (timeout) {
            g_perf_stats.timeouts++;
        }
    }

    // Update average wait time
    if (g_perf_stats.successful_acquisitions > 0) {
        g_perf_stats.average_wait_time =
            (g_perf_stats.average_wait_time * (g_perf_stats.successful_acquisitions - 1) + wait_time) /
            g_perf_stats.successful_acquisitions;
    }
}

void print_mutex_performance_stats() {
    time_t monitoring_duration = time(NULL) - g_perf_stats.monitoring_start_time;

    printf("\n=== MUTEX PERFORMANCE STATISTICS ===\n");
    printf("Monitoring Duration: %ld seconds\n", monitoring_duration);
    printf("Total Acquisitions: %d\n", g_perf_stats.total_acquisitions);
    printf("Successful: %d (%.1f%%)\n",
           g_perf_stats.successful_acquisitions,
           g_perf_stats.total_acquisitions > 0 ?
           (float)g_perf_stats.successful_acquisitions / g_perf_stats.total_acquisitions * 100 : 0);
    printf("Failed: %d (%.1f%%)\n",
           g_perf_stats.failed_acquisitions,
           g_perf_stats.total_acquisitions > 0 ?
           (float)g_perf_stats.failed_acquisitions / g_perf_stats.total_acquisitions * 100 : 0);
    printf("Timeouts: %d\n", g_perf_stats.timeouts);
    printf("Preemptive Acquisitions: %d\n", g_perf_stats.preemptive_acquisitions);
    printf("Average Wait Time: %.2f seconds\n", g_perf_stats.average_wait_time);
    printf("Acquisition Rate: %.2f per second\n",
           monitoring_duration > 0 ? (float)g_perf_stats.total_acquisitions / monitoring_duration : 0);
    printf("====================================\n\n");
}

// Configuration functions for the traffic mutex system
void set_allocation_strategy(int strategy) {
    if (traffic_mutex_initialized) {
        g_traffic_mutex.allocation_strategy = strategy;
    }
}

int get_allocation_strategy() {
    return traffic_mutex_initialized ? g_traffic_mutex.allocation_strategy : 0;
}

void set_enhanced_mode(bool enabled) {
    if (traffic_mutex_initialized) {
        g_traffic_mutex.enhanced_mode = enabled;
    }
}

bool is_enhanced_mode_enabled() {
    return traffic_mutex_initialized ? g_traffic_mutex.enhanced_mode : false;
}

// Utility function to reset the entire traffic mutex system
void reset_traffic_mutex_system() {
    if (traffic_mutex_initialized) {
        reset_intersection_state();
        reset_bankers_state();
        init_mutex_performance_monitoring();
    }
}
