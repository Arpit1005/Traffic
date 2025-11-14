/*
 * Banker's Algorithm - Deadlock Prevention System
 *
 * Implements the Banker's algorithm for safe resource allocation in traffic intersection.
 * Prevents deadlocks by checking resource allocation safety before granting access.
 *
 * Resource Model: Intersection quadrants (4 quadrants for 4-way intersection)
 * Lanes: 4 traffic approaches (North, South, East, West)
 *
 * Safety Conditions:
 * - Request <= Need (lane hasn't exceeded max claims)
 * - Request <= Available (sufficient free resources)
 * - Allocation results in safe state (all lanes can complete)
 */

#ifndef BANKERS_ALGORITHM_H
#define BANKERS_ALGORITHM_H

#include <pthread.h>
#include <stdbool.h>
#include "lane_process.h"

#define NUM_QUADRANTS 4
#ifndef NUM_LANES
#define NUM_LANES 4
#endif
#define LANE_NORTH 0
#define LANE_SOUTH 1
#define LANE_EAST 2
#define LANE_WEST 3

#define QUADRANT_NE 0
#define QUADRANT_NW 1
#define QUADRANT_SW 2
#define QUADRANT_SE 3

typedef struct {
    int available[NUM_QUADRANTS];
    int maximum[NUM_LANES][NUM_QUADRANTS];
    int allocation[NUM_LANES][NUM_QUADRANTS];
    int need[NUM_LANES][NUM_QUADRANTS];
    pthread_mutex_t resource_lock;
    bool safe_state;
    int deadlock_preventions;
} BankersState;

void init_bankers_state(BankersState* state);
void destroy_bankers_state(BankersState* state);
BankersState* get_global_bankers_state();
bool request_resources(BankersState* state, int lane_id, int request[NUM_QUADRANTS]);
bool is_safe_state(BankersState* state);
bool safety_algorithm(BankersState* state, bool finish[NUM_LANES]);
void allocate_resources(BankersState* state, int lane_id, int allocation[NUM_QUADRANTS]);
void deallocate_resources(BankersState* state, int lane_id);
void update_available_resources(BankersState* state, int available[NUM_QUADRANTS]);
void calculate_needed_quadrants(LaneProcess* lane, int need[NUM_QUADRANTS]);
void calculate_maximum_quadrants(int lane_id, int maximum[NUM_QUADRANTS]);
bool are_quadrants_available(BankersState* state, int request[NUM_QUADRANTS]);
bool can_lane_finish(BankersState* state, int lane_id);
bool check_resource_request(BankersState* state, int lane_id, int request[NUM_QUADRANTS]);
bool is_deadlock_possible(BankersState* state);
void calculate_straight_movement_quadrants(int lane_id, int quadrants[NUM_QUADRANTS]);
void calculate_left_turn_quadrants(int lane_id, int quadrants[NUM_QUADRANTS]);
void calculate_right_turn_quadrants(int lane_id, int quadrants[NUM_QUADRANTS]);
void calculate_u_turn_quadrants(int lane_id, int quadrants[NUM_QUADRANTS]);
void print_bankers_state(BankersState* state);
void print_lane_allocation(BankersState* state, int lane_id);
void print_available_quadrants(BankersState* state);
int get_total_available_quadrants(BankersState* state);
int get_total_allocated_quadrants(BankersState* state);
int get_deadlock_prevention_count(BankersState* state);
float get_resource_utilization(BankersState* state);
void increment_deadlock_preventions(BankersState* state);
void reset_bankers_state();

#endif
