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
    EmergencyType type;                  // Type of emergency vehicle
    int lane_id;                         // Lane where emergency vehicle is approaching
    float approach_time;                 // Time until emergency vehicle reaches intersection
    int priority_level;                  // Emergency priority (1-5, 1=highest)
    float crossing_duration;             // Time needed to clear intersection
    time_t timestamp;                    // When emergency was detected
    bool active;                         // Is emergency currently active
    int vehicle_id;                      // Unique emergency vehicle identifier
} EmergencyVehicle;

typedef struct {
    EmergencyVehicle current_emergency;  // Currently active emergency
    bool emergency_mode;                 // System-wide emergency flag
    time_t emergency_start_time;         // When emergency mode started
    int total_emergencies_handled;       // Total emergencies processed
    float total_emergency_response_time; // Cumulative response time
    float average_response_time;         // Average emergency response time
    bool preempt_enabled;                // Enable preemption for emergencies
} EmergencySystem;

// Emergency system lifecycle functions
void init_emergency_system(EmergencySystem* system);
void destroy_emergency_system(EmergencySystem* system);
void reset_emergency_system(EmergencySystem* system);
EmergencySystem* get_global_emergency_system();

// Emergency detection functions
bool detect_emergency_vehicle(LaneProcess* lane);
EmergencyVehicle* generate_random_emergency();
void add_emergency_vehicle(EmergencySystem* system, EmergencyVehicle* emergency);

// Emergency preemption functions
void preempt_for_emergency(EmergencySystem* system, EmergencyVehicle* emergency);
void handle_emergency_clearance(EmergencySystem* system);
void resume_normal_scheduling_after_emergency();

// Emergency processing functions
bool process_emergency_request(EmergencySystem* system, EmergencyVehicle* emergency);
void update_emergency_progress(EmergencySystem* system);
bool is_emergency_active(EmergencySystem* system);

// Priority management functions
void set_emergency_priority(LaneProcess* lane, int priority);
void restore_lane_priority(LaneProcess* lane, int original_priority);
int calculate_emergency_priority(EmergencyType type);

// Emergency vehicle creation functions
EmergencyVehicle create_ambulance(int lane_id);
EmergencyVehicle create_fire_truck(int lane_id);
EmergencyVehicle create_police_vehicle(int lane_id);
EmergencyVehicle create_custom_emergency(EmergencyType type, int lane_id);

// Emergency system state functions
EmergencyVehicle* get_current_emergency(EmergencySystem* system);
bool is_emergency_mode_active(EmergencySystem* system);
time_t get_emergency_start_time(EmergencySystem* system);
int get_total_emergencies_handled(EmergencySystem* system);

// Statistics and metrics functions
void update_emergency_statistics(EmergencySystem* system, float response_time);
float calculate_average_emergency_response_time(EmergencySystem* system);
void increment_emergency_count(EmergencySystem* system);

// Utility functions
const char* get_emergency_type_name(EmergencyType type);
void print_emergency_info(EmergencyVehicle* emergency);
void print_emergency_system_status(EmergencySystem* system);
bool validate_emergency_vehicle(EmergencyVehicle* emergency);

// Configuration functions
void set_preemption_enabled(EmergencySystem* system, bool enabled);
bool is_preemption_enabled(EmergencySystem* system);
void set_emergency_probability(int probability);

// Testing and simulation functions
EmergencyVehicle* create_test_emergency(EmergencyType type, int lane_id, float approach_time);
void simulate_emergency_scenario(EmergencySystem* system, EmergencyType type, int lane_id);

#endif // EMERGENCY_SYSTEM_H