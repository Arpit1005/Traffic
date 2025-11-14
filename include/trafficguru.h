/*
 * TrafficGuru - Intelligent Traffic Intersection Management System
 * 
 * Main system header file defining the core traffic simulation framework.
 * Integrates lane processing, scheduling algorithms, synchronization primitives,
 * deadlock prevention (Banker's algorithm), emergency vehicle handling, and
 * performance metrics collection.
 *
 * Key Components:
 * - Lane processes: North, South, East, West traffic lanes
 * - Scheduler: Multiple scheduling algorithms (SJF, Multilevel Feedback, Priority RR)
 * - Synchronization: Intersection mutex and condition variables
 * - Banker's Algorithm: Deadlock prevention for resource allocation
 * - Emergency System: Preemptive handling of emergency vehicles
 * - Performance Metrics: Real-time traffic statistics and analysis
 * - Visualization: Terminal UI with ncurses display
 *
 * Author: TrafficGuru Development Team
 * Version: 1.0
 */

#ifndef TRAFFICGURU_H
#define TRAFFICGURU_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <stdbool.h>

#include "lane_process.h"
#include "queue.h"
#include "scheduler.h"
#include "synchronization.h"
#include "bankers_algorithm.h"
#include "performance_metrics.h"
#include "emergency_system.h"
#include "visualization.h"
#include "traffic_mutex.h"

#define NUM_LANES 4
#define MAX_QUEUE_CAPACITY 20
#define DEFAULT_TIME_QUANTUM 3
#define CONTEXT_SWITCH_TIME 500
#define VEHICLE_CROSS_TIME 3
#define BATCH_EXIT_SIZE 3
#define EMERGENCY_PROBABILITY 100
#define SIMULATION_UPDATE_INTERVAL 300000
#define SIMULATION_DURATION 200

#define LANE_NORTH 0
#define LANE_SOUTH 1
#define LANE_EAST 2
#define LANE_WEST 3

#define VEHICLE_ARRIVAL_RATE_MIN 1
#define VEHICLE_ARRIVAL_RATE_MAX 3


typedef struct {
    LaneProcess lanes[NUM_LANES];
    Scheduler scheduler;
    IntersectionMutex intersection;
    BankersState bankers_state;
    PerformanceMetrics metrics;
    EmergencySystem emergency_system;
    Visualization visualization;
    SignalHistory signal_history;
    bool simulation_running;
    bool simulation_paused;
    time_t simulation_start_time;
    time_t simulation_end_time;
    int total_vehicles_generated;
    pthread_t simulation_thread;
    pthread_mutex_t global_state_lock;
    int min_arrival_rate;
    int max_arrival_rate;
    pthread_t vehicle_generator_thread;
} TrafficGuruSystem;

extern TrafficGuruSystem* g_traffic_system;
extern volatile bool keep_running;

void init_traffic_guru_system();
void destroy_traffic_guru_system();
int start_traffic_simulation();
void stop_traffic_simulation();
void pause_traffic_simulation();
void resume_traffic_simulation();

void* simulation_main_loop(void* arg);

#endif
void update_simulation_state();
void process_traffic_events();

// --- DELETED ---
// Removed update_visualization_snapshot prototype
// --- END DELETED ---

// Signal handlers
void handle_signal_interrupt(int sig);
void handle_signal_terminate(int sig);
void setup_signal_handlers();

// Utility functions
void print_system_info();
void print_usage(const char* program_name);
void cleanup_and_exit(int exit_code);
bool validate_system_state();

// Configuration functions
void set_simulation_duration(int seconds);
void set_vehicle_arrival_rate(int min_seconds, int max_seconds);
void set_time_quantum(int seconds);
void set_debug_mode(bool enabled);

// Logging functions
void log_system_event(const char* event);
void log_error(const char* error);
void log_debug(const char* message);
void log_performance_summary();

// Command line argument parsing
typedef struct {
    int duration;
    int min_arrival_rate;
    int max_arrival_rate;
    int time_quantum;
    SchedulingAlgorithm algorithm;
    bool debug_mode;
    bool no_color;
    bool help_requested;
} CommandLineArgs;

CommandLineArgs parse_command_line_args(int argc, char* argv[]);
void print_command_line_help();
void validate_command_line_args(CommandLineArgs* args);

#endif // TRAFFICGURU_H
