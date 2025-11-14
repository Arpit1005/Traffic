/*
 * Emergency System - Emergency Vehicle Preemption
 *
 * Manages detection, preemption, and prioritized handling of emergency vehicles.
 * Supports ambulances, fire trucks, and police vehicles with automatic signal override.
 *
 * Features:
 * - Emergency vehicle type detection (Ambulance, Fire Truck, Police)
 * - Preemptive signal control to clear intersections
 * - Priority-based lane allocation
 * - Response time tracking and statistics
 * - Emergency queue management for multiple simultaneous requests
 *
 * Integration: Works with scheduler and synchronization modules for signal override
 */

#ifndef EMERGENCY_SYSTEM_H
#define EMERGENCY_SYSTEM_H

#include <time.h>
#include <stdbool.h>
#include "lane_process.h"

typedef enum {
    EMERGENCY_NONE = 0,
    EMERGENCY_AMBULANCE = 1,
    EMERGENCY_FIRE_TRUCK = 2,
    EMERGENCY_POLICE = 3
} EmergencyType;

typedef struct {
    EmergencyType type;
    int lane_id;
    float approach_time;
    int priority_level;
    float crossing_duration;
    time_t timestamp;
    bool active;
    int vehicle_id;
} EmergencyVehicle;

typedef struct {
    EmergencyVehicle current_emergency;
    bool emergency_mode;
    time_t emergency_start_time;
    int total_emergencies_handled;
    float total_emergency_response_time;
    float average_response_time;
    bool preempt_enabled;
} EmergencySystem;

void init_emergency_system(EmergencySystem* system);
void destroy_emergency_system(EmergencySystem* system);
void reset_emergency_system(EmergencySystem* system);
EmergencySystem* get_global_emergency_system();

bool detect_emergency_vehicle(LaneProcess* lane);
EmergencyVehicle* generate_random_emergency();
void add_emergency_vehicle(EmergencySystem* system, EmergencyVehicle* emergency);

void preempt_for_emergency(EmergencySystem* system, EmergencyVehicle* emergency);
void handle_emergency_clearance(EmergencySystem* system);
void resume_normal_scheduling_after_emergency();

bool process_emergency_request(EmergencySystem* system, EmergencyVehicle* emergency);
void update_emergency_progress(EmergencySystem* system);
bool is_emergency_active(EmergencySystem* system);

void set_emergency_priority(LaneProcess* lane, int priority);
void restore_lane_priority(LaneProcess* lane, int original_priority);
int calculate_emergency_priority(EmergencyType type);

EmergencyVehicle create_ambulance(int lane_id);
EmergencyVehicle create_fire_truck(int lane_id);
EmergencyVehicle create_police_vehicle(int lane_id);
EmergencyVehicle create_custom_emergency(EmergencyType type, int lane_id);

EmergencyVehicle* get_current_emergency(EmergencySystem* system);
bool is_emergency_mode_active(EmergencySystem* system);
time_t get_emergency_start_time(EmergencySystem* system);
int get_total_emergencies_handled(EmergencySystem* system);

void update_emergency_statistics(EmergencySystem* system, float response_time);
float calculate_average_emergency_response_time(EmergencySystem* system);
void increment_emergency_count(EmergencySystem* system);

const char* get_emergency_type_name(EmergencyType type);
void print_emergency_info(EmergencyVehicle* emergency);
void print_emergency_system_status(EmergencySystem* system);
bool validate_emergency_vehicle(EmergencyVehicle* emergency);

void set_preemption_enabled(EmergencySystem* system, bool enabled);
bool is_preemption_enabled(EmergencySystem* system);
void set_emergency_probability(int probability);

EmergencyVehicle* create_test_emergency(EmergencyType type, int lane_id, float approach_time);
void simulate_emergency_scenario(EmergencySystem* system, EmergencyType type, int lane_id);

#endif