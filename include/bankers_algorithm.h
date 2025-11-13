#ifndef BANKERS_ALGORITHM_H
#define BANKERS_ALGORITHM_H

#include <pthread.h>
#include <stdbool.h>
#include "lane_process.h"

#define NUM_LANES 4
#define NUM_QUADRANTS 4

// Lane definitions (matching trafficguru.h)
#define LANE_NORTH 0
#define LANE_SOUTH 1
#define LANE_EAST 2
#define LANE_WEST 3

typedef struct {
    int available[NUM_QUADRANTS];          // Available intersection quadrants
    int maximum[NUM_LANES][NUM_QUADRANTS]; // Max quadrants each lane needs
    int allocation[NUM_LANES][NUM_QUADRANTS]; // Currently allocated quadrants
    int need[NUM_LANES][NUM_QUADRANTS];    // Remaining quadrants needed
    pthread_mutex_t resource_lock;         // Protect resource state
    bool safe_state;                       // System safety flag
    int deadlock_preventions;              // Counter for prevented deadlocks
} BankersState;

// Intersection quadrant definitions
typedef enum {
    QUADRANT_NE = 0,  // North-East
    QUADRANT_NW = 1,  // North-West
    QUADRANT_SW = 2,  // South-West
    QUADRANT_SE = 3   // South-East
} IntersectionQuadrant;

// Banker's algorithm lifecycle functions
void init_bankers_state(BankersState* state);
void destroy_bankers_state(BankersState* state);
BankersState* get_global_bankers_state();
void reset_bankers_state();

// Core Banker's algorithm functions
bool request_resources(BankersState* state, int lane_id, int request[NUM_QUADRANTS]);
bool is_safe_state(BankersState* state);
bool safety_algorithm(BankersState* state, bool finish[NUM_LANES]);

// Resource allocation functions
void allocate_resources(BankersState* state, int lane_id, int allocation[NUM_QUADRANTS]);
void deallocate_resources(BankersState* state, int lane_id);
void update_available_resources(BankersState* state, int available[NUM_QUADRANTS]);

// Quadrant calculation functions
void calculate_needed_quadrants(LaneProcess* lane, int need[NUM_QUADRANTS]);
void calculate_maximum_quadrants(int lane_id, int maximum[NUM_QUADRANTS]);
bool are_quadrants_available(BankersState* state, int request[NUM_QUADRANTS]);

// Safety check functions
bool can_lane_finish(BankersState* state, int lane_id);
bool check_resource_request(BankersState* state, int lane_id, int request[NUM_QUADRANTS]);
bool is_deadlock_possible(BankersState* state);

// Utility functions
void print_bankers_state(BankersState* state);
void print_lane_allocation(BankersState* state, int lane_id);
void print_available_quadrants(BankersState* state);
int get_total_available_quadrants(BankersState* state);
int get_total_allocated_quadrants(BankersState* state);

// Statistics functions
int get_deadlock_prevention_count(BankersState* state);
float get_resource_utilization(BankersState* state);
void increment_deadlock_preventions(BankersState* state);
void reset_bankers_state();

// Helper functions for traffic scenarios
void calculate_straight_movement_quadrants(int lane_id, int quadrants[NUM_QUADRANTS]);
void calculate_left_turn_quadrants(int lane_id, int quadrants[NUM_QUADRANTS]);
void calculate_right_turn_quadrants(int lane_id, int quadrants[NUM_QUADRANTS]);
void calculate_u_turn_quadrants(int lane_id, int quadrants[NUM_QUADRANTS]);

#endif // BANKERS_ALGORITHM_H