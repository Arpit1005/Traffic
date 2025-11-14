/*
 * Performance Metrics - Traffic System Performance Analysis
 *
 * Collects and analyzes performance metrics for the traffic simulation:
 * - Throughput: Vehicles processed per minute
 * - Wait Time: Average waiting time across all lanes
 * - Utilization: Intersection occupancy percentage
 * - Fairness: Jain's fairness index for equitable lane access
 * - Deadlock Prevention: Count and efficiency
 * - Emergency Response: Time to respond to emergency vehicles
 *
 * Provides real-time statistics, comparative analysis, and validation functions.
 */

#ifndef PERFORMANCE_METRICS_H
#define PERFORMANCE_METRICS_H

#include <time.h>
#include <stdbool.h>

typedef struct {
    float vehicles_per_minute;
    float avg_wait_time;
    float utilization;
    float fairness_index;
    int deadlocks_prevented;
    int context_switches;
    float emergency_response_time;
    int total_vehicles_processed;
    int queue_overflow_count;
    time_t measurement_start_time;
    time_t last_update_time;
    float lane_wait_times[4];
    int lane_throughput[4];
    int total_simulation_time;
} PerformanceMetrics;

void init_performance_metrics(PerformanceMetrics* metrics);
void destroy_performance_metrics(PerformanceMetrics* metrics);
void reset_performance_metrics(PerformanceMetrics* metrics);

void calculate_throughput_metrics(PerformanceMetrics* metrics, time_t current_time);
void calculate_wait_time_metrics(PerformanceMetrics* metrics, float lane_wait_times[4]);
void calculate_utilization_metrics(PerformanceMetrics* metrics, time_t active_time, time_t total_time);
void calculate_fairness_index_metrics(PerformanceMetrics* metrics, float wait_times[4]);

void update_vehicle_count(PerformanceMetrics* metrics, int lane_id, int vehicle_count);
void update_wait_time(PerformanceMetrics* metrics, int lane_id, float wait_time);
void update_context_switch_count(PerformanceMetrics* metrics);
void update_emergency_response_time(PerformanceMetrics* metrics, float response_time);
void update_deadlock_prevention_count(PerformanceMetrics* metrics);
void update_queue_overflow_count(PerformanceMetrics* metrics);

float get_throughput(PerformanceMetrics* metrics);
float get_average_wait_time(PerformanceMetrics* metrics);
float get_utilization(PerformanceMetrics* metrics);
float get_fairness_index(PerformanceMetrics* metrics);
int get_total_vehicles_processed(PerformanceMetrics* metrics);
float get_emergency_response_time(PerformanceMetrics* metrics);

void compare_algorithm_performance(PerformanceMetrics* metrics1, PerformanceMetrics* metrics2,
                                  const char* algo1_name, const char* algo2_name);
void generate_performance_summary(PerformanceMetrics* metrics);
void export_metrics_to_csv(PerformanceMetrics* metrics, const char* filename);

bool validate_metrics_consistency(PerformanceMetrics* metrics);
bool check_metrics_bounds(PerformanceMetrics* metrics);
void sanitize_metrics(PerformanceMetrics* metrics);

float calculate_metrics_time_window(PerformanceMetrics* metrics, time_t start_time, time_t end_time);
void update_time_based_metrics(PerformanceMetrics* metrics, time_t current_time);

void print_performance_metrics(PerformanceMetrics* metrics);
void print_detailed_metrics(PerformanceMetrics* metrics);
PerformanceMetrics* copy_metrics(PerformanceMetrics* original);

#endif