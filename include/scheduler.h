#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <time.h>
#include <pthread.h>
#include "lane_process.h"

typedef enum {
    SJF = 0,                       // Shortest Job First
    MULTILEVEL_FEEDBACK = 1,       // Multilevel Feedback Queue
    PRIORITY_ROUND_ROBIN = 2       // Priority Round Robin
} SchedulingAlgorithm;

typedef struct {
    time_t start_time;              // When execution started
    time_t end_time;                // When execution ended
    int duration;                   // Execution duration in seconds
    int vehicles_processed;         // Number of vehicles processed
    int lane_id;                    // Lane that was executed
} ExecutionRecord;

typedef struct {
    SchedulingAlgorithm algorithm;  // Current algorithm type
    Queue* ready_queue;             // Lanes ready for intersection access
    int time_quantum;               // Time slice for Round Robin variants
    int context_switch_time;        // Time to switch between lanes (ms)
    int current_lane;               // Lane currently with green light
    ExecutionRecord* execution_history; // History for analysis
    int history_size;               // Size of execution history
    int history_index;              // Current index in history
    int total_context_switches;     // Total context switches performed
    time_t last_schedule_time;      // When last scheduling decision was made
    bool scheduler_running;         // Scheduler active flag
    pthread_mutex_t scheduler_lock; // Mutex for scheduler state
    pthread_cond_t scheduler_cond;  // Condition variable for signaling
} Scheduler;

// Scheduler lifecycle functions
void init_scheduler(Scheduler* scheduler, SchedulingAlgorithm algorithm);
void destroy_scheduler(Scheduler* scheduler);
void start_scheduler(Scheduler* scheduler);
void stop_scheduler(Scheduler* scheduler);

// Scheduling algorithm functions
int schedule_next_lane_sjf(Scheduler* scheduler, LaneProcess lanes[4]);
int schedule_next_lane_multilevel(Scheduler* scheduler, LaneProcess lanes[4]);
int schedule_next_lane_priority_rr(Scheduler* scheduler, LaneProcess lanes[4]);

// Core scheduling functions
int schedule_next_lane(Scheduler* scheduler, LaneProcess lanes[4]);
void execute_lane_time_slice(Scheduler* scheduler, LaneProcess* lane, int time_quantum);
void context_switch(Scheduler* scheduler, LaneProcess* from_lane, LaneProcess* to_lane);

// Algorithm management functions
void set_scheduling_algorithm(Scheduler* scheduler, SchedulingAlgorithm algorithm);
SchedulingAlgorithm get_scheduling_algorithm(Scheduler* scheduler);
const char* get_algorithm_name(SchedulingAlgorithm algorithm);

// Execution history functions
void record_execution(Scheduler* scheduler, int lane_id, time_t start_time,
                      time_t end_time, int vehicles_processed);
void print_execution_history(Scheduler* scheduler);
ExecutionRecord* get_execution_history(Scheduler* scheduler, int* count);

// Performance analysis functions
float calculate_average_wait_time(Scheduler* scheduler, LaneProcess lanes[4]);
float calculate_throughput(Scheduler* scheduler, time_t time_period);
float calculate_fairness_index(Scheduler* scheduler, LaneProcess lanes[4]);
int calculate_context_switch_overhead(Scheduler* scheduler);

// Ready queue management
void add_lane_to_ready_queue(Scheduler* scheduler, LaneProcess* lane);
void remove_lane_from_ready_queue(Scheduler* scheduler, LaneProcess* lane);
int get_ready_queue_size(Scheduler* scheduler);
bool is_ready_queue_empty(Scheduler* scheduler);

#endif // SCHEDULER_H