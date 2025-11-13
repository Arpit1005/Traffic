#define _XOPEN_SOURCE 600
#include "../include/performance_metrics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

// Initialize performance metrics
void init_performance_metrics(PerformanceMetrics* metrics) {
    if (!metrics) return;

    memset(metrics, 0, sizeof(PerformanceMetrics));
    metrics->measurement_start_time = time(NULL);
    metrics->last_update_time = metrics->measurement_start_time;
    metrics->fairness_index = 1.0f; // Perfect fairness initially

    // Initialize lane-specific metrics
    for (int i = 0; i < 4; i++) {
        metrics->lane_wait_times[i] = 0.0f;
        metrics->lane_throughput[i] = 0;
    }
}

// Destroy performance metrics
void destroy_performance_metrics(PerformanceMetrics* metrics) {
    // Nothing to clean up for now
    (void)metrics;
}

// Reset performance metrics
void reset_performance_metrics(PerformanceMetrics* metrics) {
    if (!metrics) return;

    time_t current_time = time(NULL);

    // Reset counters while preserving timing
    metrics->vehicles_per_minute = 0.0f;
    metrics->avg_wait_time = 0.0f;
    metrics->utilization = 0.0f;
    metrics->fairness_index = 1.0f;
    metrics->deadlocks_prevented = 0;
    metrics->context_switches = 0;
    metrics->emergency_response_time = 0.0f;
    metrics->total_vehicles_processed = 0;
    metrics->queue_overflow_count = 0;
    metrics->total_simulation_time = 0;

    // Reset lane-specific metrics
    for (int i = 0; i < 4; i++) {
        metrics->lane_wait_times[i] = 0.0f;
        metrics->lane_throughput[i] = 0;
    }

    metrics->measurement_start_time = current_time;
    metrics->last_update_time = current_time;
}

// Calculate throughput metrics
void calculate_throughput_metrics(PerformanceMetrics* metrics, time_t current_time) {
    if (!metrics || current_time <= metrics->measurement_start_time) return;

    double elapsed_minutes = difftime(current_time, metrics->measurement_start_time) / 60.0;
    if (elapsed_minutes > 0) {
        metrics->vehicles_per_minute = (float)metrics->total_vehicles_processed / elapsed_minutes;
    }
}

// Calculate wait time metrics
void calculate_wait_time_metrics(PerformanceMetrics* metrics, float lane_wait_times[4]) {
    if (!metrics || !lane_wait_times) return;

    float total_wait = 0.0f;
    int active_lanes = 0;

    // --- FIX: Calculate average wait time across all lanes ---
    // Use both accumulated wait times and per-lane throughput for proper averaging
    for (int i = 0; i < 4; i++) {
        metrics->lane_wait_times[i] = lane_wait_times[i];
        if (metrics->lane_throughput[i] > 0) {
            // Average = total_wait_time_for_lane / vehicles_processed_in_lane
            float lane_avg = lane_wait_times[i] / metrics->lane_throughput[i];
            total_wait += lane_avg;
            active_lanes++;
        }
    }

    // Average wait time across all active lanes
    metrics->avg_wait_time = active_lanes > 0 ? (total_wait / active_lanes) : 0.0f;
}

// Calculate utilization metrics
void calculate_utilization_metrics(PerformanceMetrics* metrics, time_t active_time, time_t total_time) {
    if (!metrics || total_time <= 0) return;

    metrics->utilization = (float)active_time / total_time;
    if (metrics->utilization > 1.0f) metrics->utilization = 1.0f;
}

// Calculate fairness index metrics
void calculate_fairness_index_metrics(PerformanceMetrics* metrics, float wait_times[4]) {
    if (!metrics || !wait_times) return;

    float sum_wait = 0.0f, sum_wait_sq = 0.0f;
    int active_lanes = 0;

    for (int i = 0; i < 4; i++) {
        if (wait_times[i] > 0) {
            sum_wait += wait_times[i];
            sum_wait_sq += wait_times[i] * wait_times[i];
            active_lanes++;
        }
    }

    if (sum_wait > 0 && active_lanes > 0) {
        metrics->fairness_index = (sum_wait * sum_wait) / (active_lanes * sum_wait_sq);
    } else {
        metrics->fairness_index = 1.0f; // Perfect fairness when no waiting
    }

    if (metrics->fairness_index > 1.0f) metrics->fairness_index = 1.0f;
}

// Update vehicle count for a lane
void update_vehicle_count(PerformanceMetrics* metrics, int lane_id, int vehicle_count) {
    if (!metrics || lane_id < 0 || lane_id >= 4) return;

    metrics->total_vehicles_processed += vehicle_count;
    metrics->lane_throughput[lane_id] += vehicle_count;
    metrics->last_update_time = time(NULL);
}

// Update wait time for a lane
void update_wait_time(PerformanceMetrics* metrics, int lane_id, float wait_time) {
    if (!metrics || lane_id < 0 || lane_id >= 4) return;

    metrics->lane_wait_times[lane_id] = wait_time;
    metrics->last_update_time = time(NULL);
}

// Update context switch count
void update_context_switch_count(PerformanceMetrics* metrics) {
    if (!metrics) return;

    metrics->context_switches++;
    metrics->last_update_time = time(NULL);
}

// Update emergency response time
void update_emergency_response_time(PerformanceMetrics* metrics, float response_time) {
    if (!metrics) return;

    // Calculate running average
    if (metrics->emergency_response_time == 0.0f) {
        metrics->emergency_response_time = response_time;
    } else {
        metrics->emergency_response_time = (metrics->emergency_response_time + response_time) / 2.0f;
    }

    metrics->last_update_time = time(NULL);
}

// Update deadlock prevention count
void update_deadlock_prevention_count(PerformanceMetrics* metrics) {
    if (!metrics) return;

    metrics->deadlocks_prevented++;
    metrics->last_update_time = time(NULL);
}

// Update queue overflow count
void update_queue_overflow_count(PerformanceMetrics* metrics) {
    if (!metrics) return;

    metrics->queue_overflow_count++;
    metrics->last_update_time = time(NULL);
}

// Get current throughput
float get_throughput(PerformanceMetrics* metrics) {
    return metrics ? metrics->vehicles_per_minute : 0.0f;
}

// Get average wait time
float get_average_wait_time(PerformanceMetrics* metrics) {
    return metrics ? metrics->avg_wait_time : 0.0f;
}

// Get utilization percentage
float get_utilization(PerformanceMetrics* metrics) {
    return metrics ? (metrics->utilization * 100.0f) : 0.0f;
}

// Get fairness index
float get_fairness_index(PerformanceMetrics* metrics) {
    return metrics ? metrics->fairness_index : 0.0f;
}

// Get total vehicles processed
int get_total_vehicles_processed(PerformanceMetrics* metrics) {
    return metrics ? metrics->total_vehicles_processed : 0;
}

// Get emergency response time
float get_emergency_response_time(PerformanceMetrics* metrics) {
    return metrics ? metrics->emergency_response_time : 0.0f;
}

// Update time-based metrics
void update_time_based_metrics(PerformanceMetrics* metrics, time_t current_time) {
    if (!metrics) return;

    metrics->total_simulation_time = (int)(current_time - metrics->measurement_start_time);
    calculate_throughput_metrics(metrics, current_time);
    
    // --- FIX: Calculate wait time metrics from lane wait times ---
    calculate_wait_time_metrics(metrics, metrics->lane_wait_times);
    
    // --- FIX: Calculate fairness index metrics ---
    calculate_fairness_index_metrics(metrics, metrics->lane_wait_times);
    
    // --- NEW: Calculate utilization metrics ---
    // Utilization = vehicles processed / (total_time * expected_arrivals_per_second)
    if (metrics->total_simulation_time > 0) {
        // Expected arrivals per second (based on arrival rate between 1-3 sec)
        float expected_arrivals_per_sec = 1.0f / 2.0f;  // Average of 1-3 seconds
        float expected_vehicles = expected_arrivals_per_sec * metrics->total_simulation_time;
        if (expected_vehicles > 0) {
            metrics->utilization = (float)metrics->total_vehicles_processed / expected_vehicles;
            if (metrics->utilization > 1.0f) metrics->utilization = 1.0f;
        }
    }
    // --- END NEW ---
    
    metrics->last_update_time = current_time;
}

// Print performance metrics
void print_performance_metrics(PerformanceMetrics* metrics) {
    if (!metrics) {
        printf("Performance Metrics: NULL\n");
        return;
    }

    printf("\n=== PERFORMANCE METRICS ===\n");
    printf("Throughput: %.2f vehicles/minute\n", metrics->vehicles_per_minute);
    printf("Average Wait Time: %.2f seconds\n", metrics->avg_wait_time);
    printf("Intersection Utilization: %.1f%%\n", metrics->utilization * 100);
    printf("Fairness Index: %.3f\n", metrics->fairness_index);
    printf("Total Vehicles Processed: %d\n", metrics->total_vehicles_processed);
    printf("Context Switches: %d\n", metrics->context_switches);
    printf("Emergency Response Time: %.2f seconds\n", metrics->emergency_response_time);
    printf("Deadlocks Prevented: %d\n", metrics->deadlocks_prevented);
    printf("Queue Overflows: %d\n", metrics->queue_overflow_count);
    printf("Simulation Time: %d seconds\n", metrics->total_simulation_time);
    printf("===========================\n\n");
}

// Validate metrics consistency
bool validate_metrics_consistency(PerformanceMetrics* metrics) {
    if (!metrics) return false;

    // Check for negative values
    if (metrics->vehicles_per_minute < 0 || metrics->avg_wait_time < 0 ||
        metrics->utilization < 0 || metrics->fairness_index < 0) {
        return false;
    }

    // Check bounds
    if (metrics->utilization > 1.0f || metrics->fairness_index > 1.0f) {
        return false;
    }

    // Check timing consistency
    if (metrics->last_update_time < metrics->measurement_start_time) {
        return false;
    }

    return true;
}

// Sanitize metrics values
void sanitize_metrics(PerformanceMetrics* metrics) {
    if (!metrics) return;

    // Clamp values to reasonable bounds
    if (metrics->vehicles_per_minute < 0) metrics->vehicles_per_minute = 0;
    if (metrics->avg_wait_time < 0) metrics->avg_wait_time = 0;
    if (metrics->utilization < 0) metrics->utilization = 0;
    if (metrics->utilization > 1.0f) metrics->utilization = 1.0f;
    if (metrics->fairness_index < 0) metrics->fairness_index = 0;
    if (metrics->fairness_index > 1.0f) metrics->fairness_index = 1.0f;
    if (metrics->emergency_response_time < 0) metrics->emergency_response_time = 0;
}

// Copy metrics structure
PerformanceMetrics* copy_metrics(PerformanceMetrics* original) {
    if (!original) return NULL;

    PerformanceMetrics* copy = (PerformanceMetrics*)malloc(sizeof(PerformanceMetrics));
    if (copy) {
        memcpy(copy, original, sizeof(PerformanceMetrics));
    }

    return copy;
}

// Export metrics to CSV file
void export_metrics_to_csv(PerformanceMetrics* metrics, const char* filename) {
    if (!metrics || !filename) return;

    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Failed to open file %s for writing\n", filename);
        return;
    }

    // Write header
    fprintf(file, "timestamp,vehicles_per_minute,avg_wait_time,utilization,fairness_index,");
    fprintf(file, "total_vehicles,context_switches,emergency_response_time,");
    fprintf(file, "deadlocks_prevented,queue_overflows,simulation_time\n");

    // Write data
    fprintf(file, "%ld,%.2f,%.2f,%.3f,%.3f,%d,%d,%.2f,%d,%d,%d\n",
            time(NULL),
            metrics->vehicles_per_minute,
            metrics->avg_wait_time,
            metrics->utilization,
            metrics->fairness_index,
            metrics->total_vehicles_processed,
            metrics->context_switches,
            metrics->emergency_response_time,
            metrics->deadlocks_prevented,
            metrics->queue_overflow_count,
            metrics->total_simulation_time);

    fclose(file);
    printf("Metrics exported to %s\n", filename);
}