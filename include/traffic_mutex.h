#ifndef TRAFFIC_MUTEX_H
#define TRAFFIC_MUTEX_H

#include <stdbool.h>
#include <time.h>
#include "lane_process.h"
#include "synchronization.h"
#include "bankers_algorithm.h"

// Traffic mutex system lifecycle
void init_traffic_mutex_system();
void reset_traffic_mutex_system();

// Enhanced lock acquisition with Banker's algorithm integration
bool acquire_intersection_with_bankers(LaneProcess* lane);
void release_intersection_with_bankers(LaneProcess* lane);

// Advanced deadlock detection and resolution
bool detect_and_resolve_advanced_deadlock(LaneProcess lanes[4]);
void resolve_advanced_deadlock(LaneProcess lanes[4]);

// Lock acquisition with timeout and retry
bool acquire_intersection_with_timeout(LaneProcess* lane, int timeout_seconds);

// Priority-based acquisition with preemption
bool acquire_intersection_with_preemption(LaneProcess* lane);

// Performance monitoring
void init_mutex_performance_monitoring();
void record_mutex_acquisition(bool success, bool timeout, bool preemptive, float wait_time);
void print_mutex_performance_stats();

// Configuration functions
void set_allocation_strategy(int strategy);
int get_allocation_strategy();
void set_enhanced_mode(bool enabled);
bool is_enhanced_mode_enabled();

// Internal functions (exposed for testing)
bool acquire_intersection_hybrid(LaneProcess* lane, int needed_quadrants[NUM_QUADRANTS]);

#endif // TRAFFIC_MUTEX_H