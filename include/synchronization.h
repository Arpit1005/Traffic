/*
 * Synchronization - Intersection Mutex and Condition Variables
 *
 * Manages thread-safe intersection access using mutexes and condition variables.
 * Implements lane signaling, lock acquisition/release, and deadlock detection.
 *
 * Key Functions:
 * - acquire_intersection: Blocking intersection lock acquisition
 * - try_acquire_intersection: Non-blocking lock attempt
 * - release_intersection: Release intersection access and signal waiting lanes
 * - Priority inversion and deadlock handling
 * - Real-time lock holder and acquisition time tracking
 */

#ifndef SYNCHRONIZATION_H
#define SYNCHRONIZATION_H

#include <pthread.h>
#include <stdbool.h>
#include "lane_process.h"

typedef struct {
    pthread_mutex_t intersection_lock;
    pthread_cond_t condition_vars[4];
    int current_lane;
    pthread_t lock_holder;
    time_t lock_acquisition_time;
    bool intersection_available;
    int active_quadrants;
} IntersectionMutex;

void init_intersection_mutex(IntersectionMutex* intersection);
void destroy_intersection_mutex(IntersectionMutex* intersection);

bool acquire_intersection(LaneProcess* lane);
bool try_acquire_intersection(LaneProcess* lane);
void release_intersection(LaneProcess* lane);
bool is_intersection_available(LaneProcess* lane);

void wait_for_lane_signal(LaneProcess* lane);
void signal_lane(LaneProcess* lane);
void signal_all_lanes();

void handle_priority_inversion(LaneProcess* high_priority_lane, LaneProcess* low_priority_lane);
void boost_lane_priority(LaneProcess* lane, int new_priority);
void restore_lane_priority(LaneProcess* lane, int original_priority);

bool detect_deadlock(LaneProcess lanes[4]);
void resolve_deadlock(LaneProcess lanes[4]);
bool is_circular_wait_detected(LaneProcess lanes[4]);

int get_current_lane();
pthread_t get_lock_holder();
time_t get_lock_acquisition_time();
int get_active_quadrants();

void print_intersection_state();
bool validate_intersection_state();
void reset_intersection_state();
IntersectionMutex* get_global_intersection();

#endif