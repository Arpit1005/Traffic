#include "../include/emergency_system.h"
#include "../include/synchronization.h"
#include "../include/traffic_mutex.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

// Global emergency system instance
static EmergencySystem g_emergency_system = {0};
static bool emergency_system_initialized = false;

// Emergency vehicle configuration
#define DEFAULT_APPROACH_TIME_MIN 5.0f   // seconds
#define DEFAULT_APPROACH_TIME_MAX 15.0f  // seconds
#define DEFAULT_CROSSING_DURATION_MIN 3.0f // seconds
#define DEFAULT_CROSSING_DURATION_MAX 6.0f // seconds
#define DEFAULT_EMERGENCY_PROBABILITY 200 // 1 in 200 chance per check

// Get global emergency system instance
EmergencySystem* get_global_emergency_system() {
    if (!emergency_system_initialized) {
        init_emergency_system(&g_emergency_system);
    }
    return &g_emergency_system;
}

// Initialize emergency system
void init_emergency_system(EmergencySystem* system) {
    if (!system) {
        return;
    }

    memset(&system->current_emergency, 0, sizeof(EmergencyVehicle));
    system->current_emergency.active = false;
    system->emergency_mode = false;
    system->emergency_start_time = 0;
    system->total_emergencies_handled = 0;
    system->total_emergency_response_time = 0.0f;
    system->average_response_time = 0.0f;
    system->preempt_enabled = true;

    if (system == &g_emergency_system) {
        emergency_system_initialized = true;
    }
}

// Destroy emergency system
void destroy_emergency_system(EmergencySystem* system) {
    if (!system) {
        return;
    }

    // Clear any active emergency
    if (system->current_emergency.active) {
        system->current_emergency.active = false;
    }

    if (system == &g_emergency_system) {
        emergency_system_initialized = false;
    }
}

// Reset emergency system
void reset_emergency_system(EmergencySystem* system) {
    if (!system) {
        return;
    }

    memset(&system->current_emergency, 0, sizeof(EmergencyVehicle));
    system->emergency_mode = false;
    system->emergency_start_time = 0;
    system->total_emergencies_handled = 0;
    system->total_emergency_response_time = 0.0f;
    system->average_response_time = 0.0f;
}

// Detect emergency vehicle in a lane
bool detect_emergency_vehicle(LaneProcess* lane) {
    if (!lane) {
        return false;
    }

    // Random emergency vehicle generation
    if (rand() % DEFAULT_EMERGENCY_PROBABILITY == 0) {
        EmergencyVehicle* emergency = generate_random_emergency();
        emergency->lane_id = lane->lane_id;

        EmergencySystem* system = get_global_emergency_system();
        add_emergency_vehicle(system, emergency);

        return true;
    }

    return false;
}

// Generate random emergency vehicle
EmergencyVehicle* generate_random_emergency() {
    static EmergencyVehicle emergency; // Static to return pointer

    emergency.type = rand() % 3 + 1; // Random type 1-3
    emergency.lane_id = rand() % 4;   // Random lane 0-3
    emergency.approach_time = DEFAULT_APPROACH_TIME_MIN +
                             (float)(rand() % (int)(DEFAULT_APPROACH_TIME_MAX - DEFAULT_APPROACH_TIME_MIN));
    emergency.priority_level = 1; // Highest priority
    emergency.crossing_duration = DEFAULT_CROSSING_DURATION_MIN +
                                 (float)(rand() % (int)(DEFAULT_CROSSING_DURATION_MAX - DEFAULT_CROSSING_DURATION_MIN));
    emergency.timestamp = time(NULL);
    emergency.active = true;
    emergency.vehicle_id = rand() % 10000;

    return &emergency;
}

// Add emergency vehicle to system
void add_emergency_vehicle(EmergencySystem* system, EmergencyVehicle* emergency) {
    if (!system || !emergency) {
        return;
    }

    printf("🚨 EMERGENCY DETECTED: %s approaching lane %d (Vehicle ID: %d) 🚨\n",
           get_emergency_type_name(emergency->type), emergency->lane_id, emergency->vehicle_id);

    // If there's already an active emergency, queue this one
    if (system->current_emergency.active) {
        printf("Emergency already active, queuing new emergency\n");
        // In a more sophisticated implementation, we'd maintain a queue
        return;
    }

    // Set as current emergency
    memcpy(&system->current_emergency, emergency, sizeof(EmergencyVehicle));
    system->current_emergency.active = true;

    // Trigger immediate preemption
    preempt_for_emergency(system, emergency);
}

// Preempt normal traffic for emergency vehicle
void preempt_for_emergency(EmergencySystem* system, EmergencyVehicle* emergency) {
    if (!system || !emergency) {
        return;
    }

    if (!system->preempt_enabled) {
        printf("Preemption disabled, emergency vehicle must wait\n");
        return;
    }

    printf("PREEMPTING: Clearing intersection for emergency vehicle\n");

    // Set emergency mode
    system->emergency_mode = true;
    system->emergency_start_time = time(NULL);

    // Reset intersection state to clear any current allocation
    reset_intersection_state();

    // In a real implementation, we would:
    // 1. Force current lane to stop
    // 2. Set all signals to red
    // 3. Set emergency lane to green
    // 4. Monitor emergency progress

    printf("Intersection cleared for emergency vehicle in lane %d\n", emergency->lane_id);
}

// Handle emergency vehicle clearance
void handle_emergency_clearance(EmergencySystem* system) {
    if (!system || !system->current_emergency.active) {
        return;
    }

    EmergencyVehicle* emergency = &system->current_emergency;
    time_t elapsed = time(NULL) - system->emergency_start_time;

    // Check if emergency vehicle should have cleared intersection
    if (elapsed >= (time_t)emergency->crossing_duration) {
        printf("Emergency vehicle cleared intersection\n");

        // Update statistics
        update_emergency_statistics(system, emergency->approach_time);

        // Clear emergency
        memset(&system->current_emergency, 0, sizeof(EmergencyVehicle));
        system->current_emergency.active = false;
        system->emergency_mode = false;

        // Resume normal scheduling
        resume_normal_scheduling_after_emergency();

        printf("Normal traffic scheduling resumed\n");
    }
}

// Resume normal scheduling after emergency
void resume_normal_scheduling_after_emergency() {
    // Reset any emergency-related state changes
    // Signal all lanes that normal scheduling is resuming
    signal_all_lanes();
}

// Process emergency request
bool process_emergency_request(EmergencySystem* system, EmergencyVehicle* emergency) {
    if (!system || !emergency) {
        return false;
    }

    // Check if preemption is allowed
    if (!system->preempt_enabled) {
        printf("Emergency preemption is disabled\n");
        return false;
    }

    // Add to system and trigger preemption
    add_emergency_vehicle(system, emergency);
    return true;
}

// Update emergency progress
void update_emergency_progress(EmergencySystem* system) {
    if (!system) {
        return;
    }

    // Handle clearance if emergency is active
    if (system->current_emergency.active) {
        handle_emergency_clearance(system);
    }
}

// Check if emergency is active
bool is_emergency_active(EmergencySystem* system) {
    return system ? system->current_emergency.active : false;
}

// Set emergency priority for lane
void set_emergency_priority(LaneProcess* lane, int priority) {
    if (!lane) {
        return;
    }

    lane->priority = priority;
}


// Calculate emergency priority based on type
int calculate_emergency_priority(EmergencyType type) {
    // All emergency vehicles get highest priority
    // Can be extended for different emergency types
    switch (type) {
        case EMERGENCY_AMBULANCE:
        case EMERGENCY_FIRE_TRUCK:
        case EMERGENCY_POLICE:
            return 1; // Highest priority
        default:
            return 2; // Normal priority
    }
}

// Create specific emergency vehicle types
EmergencyVehicle create_ambulance(int lane_id) {
    EmergencyVehicle ambulance = {
        .type = EMERGENCY_AMBULANCE,
        .lane_id = lane_id,
        .approach_time = DEFAULT_APPROACH_TIME_MIN + (float)(rand() % 5),
        .priority_level = 1,
        .crossing_duration = DEFAULT_CROSSING_DURATION_MIN + (float)(rand() % 2),
        .timestamp = time(NULL),
        .active = true,
        .vehicle_id = rand() % 10000
    };
    return ambulance;
}

EmergencyVehicle create_fire_truck(int lane_id) {
    EmergencyVehicle fire_truck = {
        .type = EMERGENCY_FIRE_TRUCK,
        .lane_id = lane_id,
        .approach_time = DEFAULT_APPROACH_TIME_MIN + (float)(rand() % 8),
        .priority_level = 1,
        .crossing_duration = DEFAULT_CROSSING_DURATION_MIN + 2.0f + (float)(rand() % 2),
        .timestamp = time(NULL),
        .active = true,
        .vehicle_id = rand() % 10000
    };
    return fire_truck;
}

EmergencyVehicle create_police_vehicle(int lane_id) {
    EmergencyVehicle police = {
        .type = EMERGENCY_POLICE,
        .lane_id = lane_id,
        .approach_time = DEFAULT_APPROACH_TIME_MIN + (float)(rand() % 6),
        .priority_level = 1,
        .crossing_duration = DEFAULT_CROSSING_DURATION_MIN + (float)(rand() % 3),
        .timestamp = time(NULL),
        .active = true,
        .vehicle_id = rand() % 10000
    };
    return police;
}

EmergencyVehicle create_custom_emergency(EmergencyType type, int lane_id) {
    EmergencyVehicle custom = {
        .type = type,
        .lane_id = lane_id,
        .approach_time = DEFAULT_APPROACH_TIME_MIN + (float)(rand() % 10),
        .priority_level = 1,
        .crossing_duration = DEFAULT_CROSSING_DURATION_MIN + (float)(rand() % 4),
        .timestamp = time(NULL),
        .active = true,
        .vehicle_id = rand() % 10000
    };
    return custom;
}

// Get current emergency vehicle
EmergencyVehicle* get_current_emergency(EmergencySystem* system) {
    if (!system || !system->current_emergency.active) {
        return NULL;
    }
    return &system->current_emergency;
}

// Check if emergency mode is active
bool is_emergency_mode_active(EmergencySystem* system) {
    return system ? system->emergency_mode : false;
}

// Get emergency start time
time_t get_emergency_start_time(EmergencySystem* system) {
    return system ? system->emergency_start_time : 0;
}

// Get total emergencies handled
int get_total_emergencies_handled(EmergencySystem* system) {
    return system ? system->total_emergencies_handled : 0;
}

// Update emergency statistics
void update_emergency_statistics(EmergencySystem* system, float response_time) {
    if (!system) {
        return;
    }

    system->total_emergencies_handled++;
    system->total_emergency_response_time += response_time;

    if (system->total_emergencies_handled > 0) {
        system->average_response_time = system->total_emergency_response_time /
                                       system->total_emergencies_handled;
    }
}

// Calculate average emergency response time
float calculate_average_emergency_response_time(EmergencySystem* system) {
    return system ? system->average_response_time : 0.0f;
}

// Increment emergency count
void increment_emergency_count(EmergencySystem* system) {
    if (system) {
        system->total_emergencies_handled++;
    }
}

// Get emergency type name
const char* get_emergency_type_name(EmergencyType type) {
    switch (type) {
        case EMERGENCY_AMBULANCE:
            return "AMBULANCE";
        case EMERGENCY_FIRE_TRUCK:
            return "FIRE TRUCK";
        case EMERGENCY_POLICE:
            return "POLICE";
        default:
            return "UNKNOWN";
    }
}

// Print emergency information
void print_emergency_info(EmergencyVehicle* emergency) {
    if (!emergency) {
        printf("Emergency Vehicle: NULL\n");
        return;
    }

    printf("\n=== EMERGENCY VEHICLE INFO ===\n");
    printf("Type: %s\n", get_emergency_type_name(emergency->type));
    printf("Vehicle ID: %d\n", emergency->vehicle_id);
    printf("Lane: %d\n", emergency->lane_id);
    printf("Approach Time: %.1f seconds\n", emergency->approach_time);
    printf("Priority Level: %d\n", emergency->priority_level);
    printf("Crossing Duration: %.1f seconds\n", emergency->crossing_duration);
    printf("Timestamp: %ld\n", emergency->timestamp);
    printf("Active: %s\n", emergency->active ? "Yes" : "No");
    printf("============================\n\n");
}

// Print emergency system status
void print_emergency_system_status(EmergencySystem* system) {
    if (!system) {
        printf("Emergency System: NULL\n");
        return;
    }

    printf("\n=== EMERGENCY SYSTEM STATUS ===\n");
    printf("Emergency Mode: %s\n", system->emergency_mode ? "Active" : "Inactive");
    printf("Total Emergencies Handled: %d\n", system->total_emergencies_handled);
    printf("Average Response Time: %.2f seconds\n", system->average_response_time);
    printf("Preemption Enabled: %s\n", system->preempt_enabled ? "Yes" : "No");

    if (system->current_emergency.active) {
        printf("\nCurrent Emergency:\n");
        print_emergency_info(&system->current_emergency);
    } else {
        printf("\nNo Active Emergency\n");
    }

    printf("===============================\n\n");
}

// Validate emergency vehicle data
bool validate_emergency_vehicle(EmergencyVehicle* emergency) {
    if (!emergency) {
        return false;
    }

    // Check type validity
    if (emergency->type < EMERGENCY_NONE || emergency->type > EMERGENCY_POLICE) {
        return false;
    }

    // Check lane validity
    if (emergency->lane_id < 0 || emergency->lane_id >= 4) {
        return false;
    }

    // Check time validity
    if (emergency->approach_time <= 0 || emergency->crossing_duration <= 0) {
        return false;
    }

    // Check priority validity
    if (emergency->priority_level <= 0 || emergency->priority_level > 5) {
        return false;
    }

    return true;
}

// Set preemption enabled/disabled
void set_preemption_enabled(EmergencySystem* system, bool enabled) {
    if (system) {
        system->preempt_enabled = enabled;
        printf("Emergency preemption %s\n", enabled ? "enabled" : "disabled");
    }
}

// Check if preemption is enabled
bool is_preemption_enabled(EmergencySystem* system) {
    return system ? system->preempt_enabled : false;
}

// Set emergency probability (for testing)
void set_emergency_probability(int probability) {
    // This would typically be a global variable or config setting
    printf("Emergency probability set to 1 in %d\n", probability);
}

// Create test emergency vehicle
EmergencyVehicle* create_test_emergency(EmergencyType type, int lane_id, float approach_time) {
    static EmergencyVehicle test_emergency;

    test_emergency.type = type;
    test_emergency.lane_id = lane_id;
    test_emergency.approach_time = approach_time;
    test_emergency.priority_level = 1;
    test_emergency.crossing_duration = 4.0f;
    test_emergency.timestamp = time(NULL);
    test_emergency.active = true;
    test_emergency.vehicle_id = 99999; // Test vehicle ID

    return &test_emergency;
}

// Simulate emergency scenario
void simulate_emergency_scenario(EmergencySystem* system, EmergencyType type, int lane_id) {
    if (!system) {
        return;
    }

    printf("\n=== SIMULATING EMERGENCY SCENARIO ===\n");
    printf("Emergency Type: %s\n", get_emergency_type_name(type));
    printf("Target Lane: %d\n", lane_id);

    EmergencyVehicle* emergency = create_test_emergency(type, lane_id, 5.0f);
    add_emergency_vehicle(system, emergency);

    printf("Emergency simulation initiated\n");
    printf("================================\n\n");
}
