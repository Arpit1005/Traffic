/*
 * Scheduler - Traffic Signal Scheduling Algorithms
 *
 * Implements multiple scheduling algorithms for intersection signal control:
 * - Shortest Job First (SJF): Prioritizes lanes with fewest vehicles
 * - Multilevel Feedback Queue: Dynamic priority adjustment
 * - Priority Round Robin: Time-sliced scheduling with priorities
 *
 * Manages green light allocation, context switching, and execution history tracking.
 * Thread-safe with mutex and condition variable synchronization.
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <time.h>
#include <pthread.h>
#include "lane_process.h"

typedef enum {
    SJF = 0,
    MULTILEVEL_FEEDBACK = 1,
    PRIORITY_ROUND_ROBIN = 2
} SchedulingAlgorithm;

typedef struct {
    time_t start_time;
    time_t end_time;
    int duration;
    int vehicles_processed;
    int lane_id;
} ExecutionRecord;

typedef struct {
    SchedulingAlgorithm algorithm;
    Queue* ready_queue;
    int time_quantum;
    int context_switch_time;
    int current_lane;
    ExecutionRecord* execution_history;
    int history_size;
    int history_index;
    int total_context_switches;
    time_t last_schedule_time;
    bool scheduler_running;
    pthread_mutex_t scheduler_lock;
    pthread_cond_t scheduler_cond;
} Scheduler;

void init_scheduler(Scheduler* scheduler, SchedulingAlgorithm algorithm);
void destroy_scheduler(Scheduler* scheduler);
void start_scheduler(Scheduler* scheduler);
void stop_scheduler(Scheduler* scheduler);

int schedule_next_lane_sjf(Scheduler* scheduler, LaneProcess lanes[4]);
int schedule_next_lane_multilevel(Scheduler* scheduler, LaneProcess lanes[4]);
int schedule_next_lane_priority_rr(Scheduler* scheduler, LaneProcess lanes[4]);

int schedule_next_lane(Scheduler* scheduler, LaneProcess lanes[4]);
void execute_lane_time_slice(Scheduler* scheduler, LaneProcess* lane, int time_quantum);
void context_switch(Scheduler* scheduler, LaneProcess* from_lane, LaneProcess* to_lane);

void set_scheduling_algorithm(Scheduler* scheduler, SchedulingAlgorithm algorithm);
SchedulingAlgorithm get_scheduling_algorithm(Scheduler* scheduler);
const char* get_algorithm_name(SchedulingAlgorithm algorithm);

void record_execution(Scheduler* scheduler, int lane_id, time_t start_time,
                      time_t end_time, int vehicles_processed);
void print_execution_history(Scheduler* scheduler);
ExecutionRecord* get_execution_history(Scheduler* scheduler, int* count);

float calculate_average_wait_time(Scheduler* scheduler, LaneProcess lanes[4]);
float calculate_throughput(Scheduler* scheduler, time_t time_period);
float calculate_fairness_index(Scheduler* scheduler, LaneProcess lanes[4]);
int calculate_context_switch_overhead(Scheduler* scheduler);

void add_lane_to_ready_queue(Scheduler* scheduler, LaneProcess* lane);
void remove_lane_from_ready_queue(Scheduler* scheduler, LaneProcess* lane);
int get_ready_queue_size(Scheduler* scheduler);
bool is_ready_queue_empty(Scheduler* scheduler);

#endif