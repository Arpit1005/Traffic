#include "../include/bankers_algorithm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Global Banker's state instance
static BankersState g_bankers_state = {0};
static bool bankers_initialized = false;

// --- DEADLOCK FIX ---
// Create internal, non-locking versions of the safety algorithms.
// These are to be called ONLY by functions that already hold the lock.
static bool is_safe_state_unlocked(BankersState* state);
static bool safety_algorithm_unlocked(BankersState* state, bool finish[NUM_LANES]);
// --- END DEADLOCK FIX ---


// Initialize Banker's algorithm state
void init_bankers_state(BankersState* state) {
    if (!state) {
        return;
    }

    // Initialize all intersection quadrants as available
    for (int i = 0; i < NUM_QUADRANTS; i++) {
        state->available[i] = 1; // Each quadrant available once
    }

    // Initialize maximum claims for each lane
    for (int lane = 0; lane < NUM_LANES; lane++) {
        for (int quad = 0; quad < NUM_QUADRANTS; quad++) {
            // Each lane may need up to 2 quadrants (for turning)
            state->maximum[lane][quad] = (quad % 2 == lane % 2) ? 1 : 0;
            state->allocation[lane][quad] = 0;
            state->need[lane][quad] = state->maximum[lane][quad];
        }
    }

    pthread_mutex_init(&state->resource_lock, NULL);
    state->safe_state = true;
    state->deadlock_preventions = 0;

    // Use global instance if none provided
    if (state == &g_bankers_state) {
        bankers_initialized = true;
    }
}

// Destroy Banker's algorithm state
void destroy_bankers_state(BankersState* state) {
    if (!state) {
        return;
    }

    pthread_mutex_destroy(&state->resource_lock);

    if (state == &g_bankers_state) {
        bankers_initialized = false;
    }
}

// Get global Banker's state instance
BankersState* get_global_bankers_state() {
    if (!bankers_initialized) {
        init_bankers_state(&g_bankers_state);
    }
    return &g_bankers_state;
}

// Core Banker's algorithm: request resources
bool request_resources(BankersState* state, int lane_id, int request[NUM_QUADRANTS]) {
    if (!state || lane_id < 0 || lane_id >= NUM_LANES || !request) {
        return false;
    }

    pthread_mutex_lock(&state->resource_lock);

    // Step 1: Check if request <= need for the lane
    for (int quad = 0; quad < NUM_QUADRANTS; quad++) {
        if (request[quad] > state->need[lane_id][quad]) {
            printf("Lane %d request exceeds maximum claim for quadrant %d\n", lane_id, quad);
            pthread_mutex_unlock(&state->resource_lock);
            return false;
        }
    }

    // Step 2: Check if request <= available resources
    for (int quad = 0; quad < NUM_QUADRANTS; quad++) {
        if (request[quad] > state->available[quad]) {
            printf("Insufficient resources for quadrant %d\n", quad);
            pthread_mutex_unlock(&state->resource_lock);
            return false;
        }
    }

    // Step 3: Pretend to allocate resources
    for (int quad = 0; quad < NUM_QUADRANTS; quad++) {
        state->available[quad] -= request[quad];
        state->allocation[lane_id][quad] += request[quad];
        state->need[lane_id][quad] -= request[quad];
    }

    // Step 4: Check if system remains in safe state
    // --- DEADLOCK FIX: Call the internal _unlocked version ---
    if (is_safe_state_unlocked(state)) {
    // --- END DEADLOCK FIX ---
        // Allocation is safe, proceed
        printf("Safe allocation for lane %d\n", lane_id);
        pthread_mutex_unlock(&state->resource_lock);
        return true;
    } else {
        // Allocation would lead to unsafe state, rollback
        printf("Unsafe allocation detected for lane %d, rolling back\n", lane_id);
        state->deadlock_preventions++;

        for (int quad = 0; quad < NUM_QUADRANTS; quad++) {
            state->available[quad] += request[quad];
            state->allocation[lane_id][quad] -= request[quad];
            state->need[lane_id][quad] += request[quad];
        }

        pthread_mutex_unlock(&state->resource_lock);
        return false;
    }
}

// --- DEADLOCK FIX: Renamed to is_safe_state_unlocked and made static ---
// Safety algorithm implementation (INTERNAL, NO LOCK)
static bool is_safe_state_unlocked(BankersState* state) {
// --- END DEADLOCK FIX ---
    if (!state) {
        return false;
    }

    bool finish[NUM_LANES] = {false};
    int work[NUM_QUADRANTS];

    // Initialize work = available
    for (int quad = 0; quad < NUM_QUADRANTS; quad++) {
        work[quad] = state->available[quad];
    }

    // Find a lane whose need <= work
    bool found_lane;
    int iterations = 0;
    const int max_iterations = NUM_LANES * 2; // Prevent infinite loops

    do {
        found_lane = false;

        for (int lane = 0; lane < NUM_LANES; lane++) {
            if (!finish[lane]) {
                bool can_finish = true;

                // Check if need[lane] <= work
                for (int quad = 0; quad < NUM_QUADRANTS; quad++) {
                    if (state->need[lane][quad] > work[quad]) {
                        can_finish = false;
                        break;
                    }
                }

                if (can_finish) {
                    // Lane can finish, add its allocated resources to work
                    for (int quad = 0; quad < NUM_QUADRANTS; quad++) {
                        work[quad] += state->allocation[lane][quad];
                    }

                    finish[lane] = true;
                    found_lane = true;
                    break;
                }
            }
        }

        iterations++;
    } while (found_lane && iterations < max_iterations);

    // Check if all lanes can finish
    bool all_finished = true;
    for (int lane = 0; lane < NUM_LANES; lane++) {
        if (!finish[lane]) {
            all_finished = false;
            break;
        }
    }

    state->safe_state = all_finished;
    return all_finished;
}

// --- DEADLOCK FIX: NEW public, thread-safe version ---
// This is the function external modules should call.
bool is_safe_state(BankersState* state) {
    if (!state) {
        return false;
    }
    pthread_mutex_lock(&state->resource_lock);
    // Call the internal, non-locking function
    bool safe = is_safe_state_unlocked(state);
    pthread_mutex_unlock(&state->resource_lock);
    return safe;
}
// --- END DEADLOCK FIX ---


// --- DEADLOCK FIX: Renamed to safety_algorithm_unlocked and made static ---
// Detailed safety algorithm (INTERNAL, NO LOCK)
static bool safety_algorithm_unlocked(BankersState* state, bool finish[NUM_LANES]) {
// --- END DEADLOCK FIX ---
    if (!state || !finish) {
        return false;
    }

    int work[NUM_QUADRANTS];

    // Initialize work array
    for (int quad = 0; quad < NUM_QUADRANTS; quad++) {
        work[quad] = state->available[quad];
    }

    // Initialize finish array
    for (int lane = 0; lane < NUM_LANES; lane++) {
        finish[lane] = false;
    }

    // Find safe sequence
    for (int count = 0; count < NUM_LANES; count++) {
        int found_lane = -1;

        // Find lane that can finish
        for (int lane = 0; lane < NUM_LANES; lane++) {
            if (!finish[lane]) {
                bool can_finish = true;

                for (int quad = 0; quad < NUM_QUADRANTS; quad++) {
                    if (state->need[lane][quad] > work[quad]) {
                        can_finish = false;
                        break;
                    }
                }

                if (can_finish) {
                    found_lane = lane;
                    break;
                }
            }
        }

        if (found_lane == -1) {
            return false; // No safe sequence found
        }

        // Mark lane as finished and free its resources
        finish[found_lane] = true;
        for (int quad = 0; quad < NUM_QUADRANTS; quad++) {
            work[quad] += state->allocation[found_lane][quad];
        }
    }

    return true; // Safe sequence found
}

// --- DEADLOCK FIX: NEW public, thread-safe version ---
bool safety_algorithm(BankersState* state, bool finish[NUM_LANES]) {
    if (!state || !finish) {
        return false;
    }
    pthread_mutex_lock(&state->resource_lock);
    // Call the internal, non-locking function
    bool safe = safety_algorithm_unlocked(state, finish);
    pthread_mutex_unlock(&state->resource_lock);
    return safe;
}
// --- END DEADLOCK FIX ---


// Allocate resources to a lane
void allocate_resources(BankersState* state, int lane_id, int allocation[NUM_QUADRANTS]) {
    if (!state || lane_id < 0 || lane_id >= NUM_LANES || !allocation) {
        return;
    }

    pthread_mutex_lock(&state->resource_lock);

    for (int quad = 0; quad < NUM_QUADRANTS; quad++) {
        if (allocation[quad] <= state->available[quad] &&
            allocation[quad] <= state->need[lane_id][quad]) {

            state->available[quad] -= allocation[quad];
            state->allocation[lane_id][quad] += allocation[quad];
            state->need[lane_id][quad] -= allocation[quad];
        }
    }

    pthread_mutex_unlock(&state->resource_lock);
}

// Deallocate resources from a lane
void deallocate_resources(BankersState* state, int lane_id) {
    if (!state || lane_id < 0 || lane_id >= NUM_LANES) {
        return;
    }

    pthread_mutex_lock(&state->resource_lock);

    for (int quad = 0; quad < NUM_QUADRANTS; quad++) {
        state->available[quad] += state->allocation[lane_id][quad];
        state->need[lane_id][quad] += state->allocation[lane_id][quad];
        state->allocation[lane_id][quad] = 0;
    }

    pthread_mutex_unlock(&state->resource_lock);
}

// Update available resources
void update_available_resources(BankersState* state, int available[NUM_QUADRANTS]) {
    if (!state || !available) {
        return;
    }

    pthread_mutex_lock(&state->resource_lock);

    for (int quad = 0; quad < NUM_QUADRANTS; quad++) {
        state->available[quad] = available[quad];
    }

    pthread_mutex_unlock(&state->resource_lock);
}

// Calculate quadrants needed for different movements
void calculate_needed_quadrants(LaneProcess* lane, int need[NUM_QUADRANTS]) {
    if (!lane || !need) {
        return;
    }

    // Initialize need array
    for (int quad = 0; quad < NUM_QUADRANTS; quad++) {
        need[quad] = 0;
    }

    // Calculate quadrants based on lane and typical movement patterns
    // This is simplified - in practice, would depend on vehicle destination
    calculate_straight_movement_quadrants(lane->lane_id, need);
}

// Calculate maximum quadrants a lane might need
void calculate_maximum_quadrants(int lane_id, int maximum[NUM_QUADRANTS]) {
    if (lane_id < 0 || lane_id >= NUM_LANES || !maximum) {
        return;
    }

    // Initialize maximum array
    for (int quad = 0; quad < NUM_QUADRANTS; quad++) {
        maximum[quad] = 0;
    }

    // Calculate maximum based on worst-case scenario (left turn needs most quadrants)
    calculate_left_turn_quadrants(lane_id, maximum);
}

// Check if quadrants are available
bool are_quadrants_available(BankersState* state, int request[NUM_QUADRANTS]) {
    if (!state || !request) {
        return false;
    }

    pthread_mutex_lock(&state->resource_lock);

    bool available = true;
    for (int quad = 0; quad < NUM_QUADRANTS; quad++) {
        if (request[quad] > state->available[quad]) {
            available = false;
            break;
        }
    }

    pthread_mutex_unlock(&state->resource_lock);
    return available;
}

// Check if a specific lane can finish
bool can_lane_finish(BankersState* state, int lane_id) {
    if (!state || lane_id < 0 || lane_id >= NUM_LANES) {
        return false;
    }

    pthread_mutex_lock(&state->resource_lock);

    bool can_finish = true;
    for (int quad = 0; quad < NUM_QUADRANTS; quad++) {
        if (state->need[lane_id][quad] > state->available[quad]) {
            can_finish = false;
            break;
        }
    }

    pthread_mutex_unlock(&state->resource_lock);
    return can_finish;
}

// Check resource request validity
bool check_resource_request(BankersState* state, int lane_id, int request[NUM_QUADRANTS]) {
    if (!state || lane_id < 0 || lane_id >= NUM_LANES || !request) {
        return false;
    }

    pthread_mutex_lock(&state->resource_lock);

    bool valid = true;

    // Check if request exceeds need
    for (int quad = 0; quad < NUM_QUADRANTS; quad++) {
        if (request[quad] > state->need[lane_id][quad]) {
            valid = false;
            break;
        }
    }

    // Check if request exceeds available resources
    if (valid) {
        for (int quad = 0; quad < NUM_QUADRANTS; quad++) {
            if (request[quad] > state->available[quad]) {
                valid = false;
                break;
            }
        }
    }

    pthread_mutex_unlock(&state->resource_lock);
    return valid;
}

// Check if deadlock is possible
bool is_deadlock_possible(BankersState* state) {
    // This function now correctly calls the thread-safe version
    return !is_safe_state(state);
}

// Movement-specific quadrant calculations
void calculate_straight_movement_quadrants(int lane_id, int quadrants[NUM_QUADRANTS]) {
    if (!quadrants || lane_id < 0 || lane_id >= NUM_LANES) {
        return;
    }

    switch (lane_id) {
        case LANE_NORTH: // North going straight needs SE quadrant
            quadrants[QUADRANT_SE] = 1;
            break;
        case LANE_SOUTH: // South going straight needs NW quadrant
            quadrants[QUADRANT_NW] = 1;
            break;
        case LANE_EAST: // East going straight needs NW quadrant
            quadrants[QUADRANT_NW] = 1;
            break;
        case LANE_WEST: // West going straight needs SE quadrant
            quadrants[QUADRANT_SE] = 1;
            break;
    }
}

void calculate_left_turn_quadrants(int lane_id, int quadrants[NUM_QUADRANTS]) {
    if (!quadrants || lane_id < 0 || lane_id >= NUM_LANES) {
        return;
    }

    switch (lane_id) {
        case LANE_NORTH: // North turning left needs SW and SE quadrants
            quadrants[QUADRANT_SW] = 1;
            quadrants[QUADRANT_SE] = 1;
            break;
        case LANE_SOUTH: // South turning left needs NE and NW quadrants
            quadrants[QUADRANT_NE] = 1;
            quadrants[QUADRANT_NW] = 1;
            break;
        case LANE_EAST: // East turning left needs NE and SE quadrants
            quadrants[QUADRANT_NE] = 1;
            quadrants[QUADRANT_SE] = 1;
            break;
        case LANE_WEST: // West turning left needs NW and SW quadrants
            quadrants[QUADRANT_NW] = 1;
            quadrants[QUADRANT_SW] = 1;
            break;
    }
}

void calculate_right_turn_quadrants(int lane_id, int quadrants[NUM_QUADRANTS]) {
    if (!quadrants || lane_id < 0 || lane_id >= NUM_LANES) {
        return;
    }

    switch (lane_id) {
        case LANE_NORTH: // North turning right needs NE quadrant
            quadrants[QUADRANT_NE] = 1;
            break;
        case LANE_SOUTH: // South turning right needs SW quadrant
            quadrants[QUADRANT_SW] = 1;
            break;
        case LANE_EAST: // East turning right needs SE quadrant
            quadrants[QUADRANT_SE] = 1;
            break;
        case LANE_WEST: // West turning right needs NW quadrant
            quadrants[QUADRANT_NW] = 1;
            break;
    }
}

void calculate_u_turn_quadrants(int lane_id, int quadrants[NUM_QUADRANTS]) {
    if (!quadrants || lane_id < 0 || lane_id >= NUM_LANES) {
        return;
    }

    // U-turn needs all quadrants (worst case)
    for (int quad = 0; quad < NUM_QUADRANTS; quad++) {
        quadrants[quad] = 1;
    }
}

// Utility functions
void print_bankers_state(BankersState* state) {
    if (!state) {
        printf("Bankers State: NULL\n");
        return;
    }

    pthread_mutex_lock(&state->resource_lock);

    printf("\n=== BANKER'S ALGORITHM STATE ===\n");
    printf("Safe State: %s\n", state->safe_state ? "Yes" : "No");
    printf("Deadlocks Prevented: %d\n", state->deadlock_preventions);

    printf("\nAvailable Resources: ");
    for (int quad = 0; quad < NUM_QUADRANTS; quad++) {
        printf("%d ", state->available[quad]);
    }
    printf("\n");

    printf("\nAllocation Matrix:\n");
    for (int lane = 0; lane < NUM_LANES; lane++) {
        printf("Lane %d: ", lane);
        for (int quad = 0; quad < NUM_QUADRANTS; quad++) {
            printf("%d ", state->allocation[lane][quad]);
        }
        printf("\n");
    }

    printf("\nNeed Matrix:\n");
    for (int lane = 0; lane < NUM_LANES; lane++) {
        printf("Lane %d: ", lane);
        for (int quad = 0; quad < NUM_QUADRANTS; quad++) {
            printf("%d ", state->need[lane][quad]);
        }
        printf("\n");
    }

    printf("===============================\n\n");

    pthread_mutex_unlock(&state->resource_lock);
}

void print_lane_allocation(BankersState* state, int lane_id) {
    if (!state || lane_id < 0 || lane_id >= NUM_LANES) {
        return;
    }

    pthread_mutex_lock(&state->resource_lock);

    printf("Lane %d Allocation: ", lane_id);
    for (int quad = 0; quad < NUM_QUADRANTS; quad++) {
        printf("%d ", state->allocation[lane_id][quad]);
    }
    printf("\n");

    printf("Lane %d Need: ", lane_id);
    for (int quad = 0; quad < NUM_QUADRANTS; quad++) {
        printf("%d ", state->need[lane_id][quad]);
    }
    printf("\n");

    pthread_mutex_unlock(&state->resource_lock);
}

void print_available_quadrants(BankersState* state) {
    if (!state) {
        return;
    }

    pthread_mutex_lock(&state->resource_lock);

    printf("Available Quadrants: ");
    for (int quad = 0; quad < NUM_QUADRANTS; quad++) {
        printf("%d ", state->available[quad]);
    }
    printf("\n");

    pthread_mutex_unlock(&state->resource_lock);
}

int get_total_available_quadrants(BankersState* state) {
    if (!state) {
        return 0;
    }

    pthread_mutex_lock(&state->resource_lock);
    int total = 0;
    for (int quad = 0; quad < NUM_QUADRANTS; quad++) {
        total += state->available[quad];
    }
    pthread_mutex_unlock(&state->resource_lock);

    return total;
}

int get_total_allocated_quadrants(BankersState* state) {
    if (!state) {
        return 0;
    }

    pthread_mutex_lock(&state->resource_lock);
    int total = 0;
    for (int lane = 0; lane < NUM_LANES; lane++) {
        for (int quad = 0; quad < NUM_QUADRANTS; quad++) {
            total += state->allocation[lane][quad];
        }
    }
    pthread_mutex_unlock(&state->resource_lock);

    return total;
}

int get_deadlock_prevention_count(BankersState* state) {
    if (!state) {
        return 0;
    }

    pthread_mutex_lock(&state->resource_lock);
    int count = state->deadlock_preventions;
    pthread_mutex_unlock(&state->resource_lock);

    return count;
}

float get_resource_utilization(BankersState* state) {
    if (!state) {
        return 0.0f;
    }

    int total_allocated = get_total_allocated_quadrants(state);
    int total_resources = NUM_QUADRANTS; // Total available quadrants

    return total_resources > 0 ? (float)total_allocated / total_resources : 0.0f;
}

void increment_deadlock_preventions(BankersState* state) {
    if (!state) {
        return;
    }

    pthread_mutex_lock(&state->resource_lock);
    state->deadlock_preventions++;
    pthread_mutex_unlock(&state->resource_lock);
}

void reset_bankers_state() {
    bankers_initialized = false;
    init_bankers_state(&g_bankers_state);
}
