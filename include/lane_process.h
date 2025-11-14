/*
 * Lane Process - Traffic Lane State Management
 *
 * Manages individual traffic lane processes representing North, South, East, and West
 * approaches to an intersection. Each lane maintains vehicle queue, state tracking,
 * performance metrics, and intersection resource allocation.
 *
 * Lane States: WAITING, READY, RUNNING, BLOCKED
 * Resources: Requested and allocated intersection quadrants
 *
 * Key Features:
 * - Queue management for arriving vehicles
 * - State transitions and priority scheduling
 * - Performance tracking (wait times, throughput)
 * - Thread-safe synchronization with mutex and condition variables
 * - Intersection quadrant allocation for deadlock-free crossing
 */

#ifndef LANE_PROCESS_H
#define LANE_PROCESS_H

#include <pthread.h>
#include <time.h>
#include "queue.h"

#define BATCH_EXIT_SIZE 3

typedef enum {
    WAITING = 0,
    READY = 1,
    RUNNING = 2,
    BLOCKED = 3
} LaneState;

typedef struct {
    int lane_id;
    Queue* queue;
    int queue_length;
    int max_queue_length;
    LaneState state;
    int priority;
    int waiting_time;
    pthread_t thread_id;
    pthread_mutex_t queue_lock;
    pthread_cond_t queue_cond;
    time_t last_arrival_time;
    time_t last_service_time;
    int total_vehicles_served;
    int total_waiting_time;
    int requested_quadrants;
    int allocated_quadrants;
} LaneProcess;

void init_lane_process(LaneProcess* lane, int lane_id, int max_capacity);
void destroy_lane_process(LaneProcess* lane);
void* lane_process_thread(void* arg);

void add_vehicle_to_lane(LaneProcess* lane, int vehicle_id);
int remove_vehicle_from_lane(LaneProcess* lane);
int remove_vehicle_from_lane_unlocked(LaneProcess* lane);
int get_lane_queue_length(LaneProcess* lane);

void update_lane_state(LaneProcess* lane, LaneState new_state);
int is_lane_ready(LaneProcess* lane);
int is_lane_blocked(LaneProcess* lane);

void update_lane_metrics(LaneProcess* lane);
float get_lane_average_wait_time(LaneProcess* lane);
int get_lane_throughput(LaneProcess* lane);

void request_intersection_quadrants(LaneProcess* lane, int quadrants);
void release_intersection_quadrants(LaneProcess* lane);

const char* get_lane_name(int lane_id);
void print_lane_info(LaneProcess* lane);

#endif
