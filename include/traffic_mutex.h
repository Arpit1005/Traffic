/*
 * Traffic Mutex - Enhanced Lock Management with Banker's Algorithm
 *
 * Combines traditional intersection mutex with Banker's algorithm for enhanced
 * deadlock prevention. Supports multiple allocation strategies (FIFO, Banker's, Hybrid).
 *
 * Features:
 * - Hybrid allocation with Banker's algorithm safety checks
 * - Emergency vehicle priority override
 * - Timeout-based acquisition attempts
 * - Preemptive lock acquisition
 * - Detailed performance monitoring
 *
 * Strategies:
 * 0: FIFO - Simple queue-based allocation
 * 1: Banker's - Banker's algorithm only
 * 2: Hybrid - Banker's with traditional fallback
 */

#ifndef TRAFFIC_MUTEX_H
#define TRAFFIC_MUTEX_H

#include <stdbool.h>
#include <time.h>
#include "lane_process.h"
#include "synchronization.h"
#include "bankers_algorithm.h"

void init_traffic_mutex_system();
void reset_traffic_mutex_system();

bool acquire_intersection_with_bankers(LaneProcess* lane);
void release_intersection_with_bankers(LaneProcess* lane);

bool detect_and_resolve_advanced_deadlock(LaneProcess lanes[4]);
void resolve_advanced_deadlock(LaneProcess lanes[4]);

bool acquire_intersection_with_timeout(LaneProcess* lane, int timeout_seconds);

bool acquire_intersection_with_preemption(LaneProcess* lane);

void init_mutex_performance_monitoring();
void record_mutex_acquisition(bool success, bool timeout, bool preemptive, float wait_time);
void print_mutex_performance_stats();

void set_allocation_strategy(int strategy);
int get_allocation_strategy();
void set_enhanced_mode(bool enabled);
bool is_enhanced_mode_enabled();

bool acquire_intersection_hybrid(LaneProcess* lane, int needed_quadrants[NUM_QUADRANTS]);

#endif