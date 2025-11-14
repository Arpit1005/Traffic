/*
 * Synchronization Implementation - Intersection Lock Management
 *
 * Thread-safe intersection access control using POSIX mutexes and condition variables.
 * Provides lane signaling, blocking/non-blocking lock acquisition, and deadlock detection.
 *
 * Compilation: Include synchronization.h, lane_process.h
 */

#define _XOPEN_SOURCE 600
#include "../include/synchronization.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <assert.h>

static IntersectionMutex g_intersection = {0};
static bool intersection_initialized = false;

// Initialize intersection mutex
void init_intersection_mutex(IntersectionMutex* intersection) {
    if (!intersection) {
        return;
    }

    pthread_mutex_init(&intersection->intersection_lock, NULL);

    // Initialize condition variables for each lane
    for (int i = 0; i < 4; i++) {
        pthread_cond_init(&intersection->condition_vars[i], NULL);
    }

    intersection->current_lane = -1; // No lane currently occupying intersection
    intersection->lock_holder = 0;
    intersection->lock_acquisition_time = 0;
    intersection->intersection_available = true;
    intersection->active_quadrants = 0;

    // Use global instance if none provided
    if (intersection == &g_intersection) {
        intersection_initialized = true;
    }
}

// Destroy intersection mutex
void destroy_intersection_mutex(IntersectionMutex* intersection) {
    if (!intersection) {
        return;
    }

    pthread_mutex_destroy(&intersection->intersection_lock);

    for (int i = 0; i < 4; i++) {
        pthread_cond_destroy(&intersection->condition_vars[i]);
    }

    if (intersection == &g_intersection) {
        intersection_initialized = false;
    }
}

// Get global intersection instance
IntersectionMutex* get_global_intersection() {
    if (!intersection_initialized) {
        init_intersection_mutex(&g_intersection);
    }
    return &g_intersection;
}

// Acquire intersection access for a lane
bool acquire_intersection(LaneProcess* lane) {
    if (!lane) {
        return false;
    }

    IntersectionMutex* intersection = get_global_intersection();
    pthread_mutex_lock(&intersection->intersection_lock);

    // Wait if intersection is not available or another lane has it
    while (!intersection->intersection_available ||
           (intersection->current_lane != -1 && intersection->current_lane != lane->lane_id)) {

        // Wait on this lane's condition variable
        pthread_cond_wait(&intersection->condition_vars[lane->lane_id],
                         &intersection->intersection_lock);
    }

    // Grant intersection access to this lane
    intersection->intersection_available = false;
    intersection->current_lane = lane->lane_id;
    intersection->lock_holder = pthread_self();
    intersection->lock_acquisition_time = time(NULL);
    intersection->active_quadrants = lane->requested_quadrants;

    pthread_mutex_unlock(&intersection->intersection_lock);
    return true;
}

// Try to acquire intersection access (non-blocking)
bool try_acquire_intersection(LaneProcess* lane) {
    if (!lane) {
        return false;
    }

    IntersectionMutex* intersection = get_global_intersection();
    bool acquired = false;

    if (pthread_mutex_trylock(&intersection->intersection_lock) == 0) {
        // Check if intersection is available
        if (intersection->intersection_available ||
            (intersection->current_lane == lane->lane_id)) {

            // Grant access
            intersection->intersection_available = false;
            intersection->current_lane = lane->lane_id;
            intersection->lock_holder = pthread_self();
            intersection->lock_acquisition_time = time(NULL);
            intersection->active_quadrants = lane->requested_quadrants;
            acquired = true;
        }

        pthread_mutex_unlock(&intersection->intersection_lock);
    }

    return acquired;
}

// Release intersection access
void release_intersection(LaneProcess* lane) {
    if (!lane) {
        return;
    }

    IntersectionMutex* intersection = get_global_intersection();
    pthread_mutex_lock(&intersection->intersection_lock);

    // Verify this lane holds the intersection lock
    if (intersection->current_lane == lane->lane_id) {
        // Release intersection
        intersection->intersection_available = true;
        intersection->current_lane = -1;
        intersection->lock_holder = 0;
        intersection->lock_acquisition_time = 0;
        intersection->active_quadrants = 0;

        // Signal all waiting lanes
        for (int i = 0; i < 4; i++) {
            pthread_cond_signal(&intersection->condition_vars[i]);
        }
    }

    pthread_mutex_unlock(&intersection->intersection_lock);
}

// Check if intersection is available for a specific lane
bool is_intersection_available(LaneProcess* lane) {
    if (!lane) {
        return false;
    }

    IntersectionMutex* intersection = get_global_intersection();
    pthread_mutex_lock(&intersection->intersection_lock);

    bool available = intersection->intersection_available ||
                    intersection->current_lane == lane->lane_id;

    pthread_mutex_unlock(&intersection->intersection_lock);
    return available;
}

// Wait for lane signal (conditional wait)
void wait_for_lane_signal(LaneProcess* lane) {
    if (!lane) {
        return;
    }

    IntersectionMutex* intersection = get_global_intersection();
    pthread_mutex_lock(&intersection->intersection_lock);
    pthread_cond_wait(&intersection->condition_vars[lane->lane_id],
                     &intersection->intersection_lock);
    pthread_mutex_unlock(&intersection->intersection_lock);
}

// Signal a specific lane
void signal_lane(LaneProcess* lane) {
    if (!lane) {
        return;
    }

    IntersectionMutex* intersection = get_global_intersection();
    pthread_mutex_lock(&intersection->intersection_lock);
    pthread_cond_signal(&intersection->condition_vars[lane->lane_id]);
    pthread_mutex_unlock(&intersection->intersection_lock);
}

// Signal all waiting lanes
void signal_all_lanes() {
    IntersectionMutex* intersection = get_global_intersection();
    pthread_mutex_lock(&intersection->intersection_lock);

    for (int i = 0; i < 4; i++) {
        pthread_cond_signal(&intersection->condition_vars[i]);
    }

    pthread_mutex_unlock(&intersection->intersection_lock);
}

// Handle priority inversion
void handle_priority_inversion(LaneProcess* high_priority_lane, LaneProcess* low_priority_lane) {
    if (!high_priority_lane || !low_priority_lane) {
        return;
    }

    IntersectionMutex* intersection = get_global_intersection();
    pthread_mutex_lock(&intersection->intersection_lock);

    // If low priority lane holds intersection and high priority lane is waiting
    if (intersection->current_lane == low_priority_lane->lane_id &&
        high_priority_lane->priority < low_priority_lane->priority) {

        // Temporarily boost low priority lane's priority
        int original_priority = low_priority_lane->priority;
        low_priority_lane->priority = high_priority_lane->priority;

        // Give low priority lane a chance to complete quickly
        pthread_cond_signal(&intersection->condition_vars[low_priority_lane->lane_id]);

        pthread_mutex_unlock(&intersection->intersection_lock);

        // Wait a bit for low priority lane to complete
        usleep(100000); // 100ms

        // Restore original priority
        low_priority_lane->priority = original_priority;
    } else {
        pthread_mutex_unlock(&intersection->intersection_lock);
    }
}

// Boost lane priority temporarily
void boost_lane_priority(LaneProcess* lane, int new_priority) {
    if (!lane) {
        return;
    }

    lane->priority = new_priority;
}

// Restore lane priority
void restore_lane_priority(LaneProcess* lane, int original_priority) {
    if (!lane) {
        return;
    }

    lane->priority = original_priority;
}

// Detect deadlock (simplified circular wait detection)
bool detect_deadlock(LaneProcess lanes[4]) {
    if (!lanes) {
        return false;
    }

    // Simple deadlock detection: check if all lanes are blocked
    // In a more complex implementation, we'd build a wait graph
    int blocked_lanes = 0;

    for (int i = 0; i < 4; i++) {
        if (is_lane_blocked(&lanes[i]) || lanes[i].state == BLOCKED) {
            blocked_lanes++;
        }
    }

    // Consider deadlock if 3 or 4 lanes are blocked for extended period
    return blocked_lanes >= 3;
}

// Resolve deadlock by selecting a victim lane
void resolve_deadlock(LaneProcess lanes[4]) {
    if (!lanes) {
        return;
    }

    // Find lane with lowest priority as victim
    int victim_lane = -1;
    int lowest_priority = INT_MAX;

    for (int i = 0; i < 4; i++) {
        if (lanes[i].state == BLOCKED && lanes[i].priority < lowest_priority) {
            lowest_priority = lanes[i].priority;
            victim_lane = i;
        }
    }

    // Unblock victim lane
    if (victim_lane != -1) {
        update_lane_state(&lanes[victim_lane], READY);
        signal_lane(&lanes[victim_lane]);
    }
}

// Check for circular wait condition
bool is_circular_wait_detected(LaneProcess lanes[4]) {
    if (!lanes) {
        return false;
    }

    // Simplified circular wait detection
    // In practice, this would analyze resource allocation graphs
    IntersectionMutex* intersection = get_global_intersection();
    pthread_mutex_lock(&intersection->intersection_lock);

    bool circular_wait = false;

    // Check if multiple lanes are waiting for intersection resources
    int waiting_lanes = 0;
    for (int i = 0; i < 4; i++) {
        if (lanes[i].state == READY && lanes[i].requested_quadrants > 0) {
            waiting_lanes++;
        }
    }

    // Circular wait likely if multiple lanes are waiting
    if (waiting_lanes >= 3) {
        circular_wait = true;
    }

    pthread_mutex_unlock(&intersection->intersection_lock);
    return circular_wait;
}

// Get current lane ID
int get_current_lane() {
    IntersectionMutex* intersection = get_global_intersection();
    pthread_mutex_lock(&intersection->intersection_lock);
    int current = intersection->current_lane;
    pthread_mutex_unlock(&intersection->intersection_lock);
    return current;
}

// Get lock holder thread ID
pthread_t get_lock_holder() {
    IntersectionMutex* intersection = get_global_intersection();
    pthread_mutex_lock(&intersection->intersection_lock);
    pthread_t holder = intersection->lock_holder;
    pthread_mutex_unlock(&intersection->intersection_lock);
    return holder;
}

// Get lock acquisition time
time_t get_lock_acquisition_time() {
    IntersectionMutex* intersection = get_global_intersection();
    pthread_mutex_lock(&intersection->intersection_lock);
    time_t acquisition_time = intersection->lock_acquisition_time;
    pthread_mutex_unlock(&intersection->intersection_lock);
    return acquisition_time;
}

// Get active quadrants
int get_active_quadrants() {
    IntersectionMutex* intersection = get_global_intersection();
    pthread_mutex_lock(&intersection->intersection_lock);
    int quadrants = intersection->active_quadrants;
    pthread_mutex_unlock(&intersection->intersection_lock);
    return quadrants;
}

// Print intersection state for debugging
void print_intersection_state() {
    IntersectionMutex* intersection = get_global_intersection();
    pthread_mutex_lock(&intersection->intersection_lock);

    printf("\n=== INTERSECTION STATE ===\n");
    printf("Available: %s\n", intersection->intersection_available ? "Yes" : "No");
    printf("Current Lane: %d\n", intersection->current_lane);
    printf("Lock Holder: %lu\n", (unsigned long)intersection->lock_holder);
    printf("Active Quadrants: %d\n", intersection->active_quadrants);

    if (intersection->lock_acquisition_time > 0) {
        time_t hold_time = time(NULL) - intersection->lock_acquisition_time;
        printf("Lock Held For: %ld seconds\n", hold_time);
    }

    printf("===========================\n\n");

    pthread_mutex_unlock(&intersection->intersection_lock);
}

// Validate intersection state consistency
bool validate_intersection_state() {
    IntersectionMutex* intersection = get_global_intersection();
    pthread_mutex_lock(&intersection->intersection_lock);

    bool valid = true;

    // Check if intersection is marked as unavailable but no current lane
    if (!intersection->intersection_available && intersection->current_lane == -1) {
        printf("ERROR: Intersection unavailable but no current lane set\n");
        valid = false;
    }

    // Check if intersection is available but current lane is set
    if (intersection->intersection_available && intersection->current_lane != -1) {
        printf("ERROR: Intersection available but current lane is set\n");
        valid = false;
    }

    pthread_mutex_unlock(&intersection->intersection_lock);
    return valid;
}

// Reset intersection state
void reset_intersection_state() {
    IntersectionMutex* intersection = get_global_intersection();
    pthread_mutex_lock(&intersection->intersection_lock);

    intersection->intersection_available = true;
    intersection->current_lane = -1;
    intersection->lock_holder = 0;
    intersection->lock_acquisition_time = 0;
    intersection->active_quadrants = 0;

    // Signal all waiting lanes
    for (int i = 0; i < 4; i++) {
        pthread_cond_signal(&intersection->condition_vars[i]);
    }

    pthread_mutex_unlock(&intersection->intersection_lock);
}