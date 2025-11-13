#define _XOPEN_SOURCE 600
#include "../include/visualization.h"
#include "../include/trafficguru.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// Simple visualization implementation (without ncurses for now)
static bool viz_initialized = false;

// Initialize visualization system
void init_visualization(Visualization* viz) {
    if (!viz) return;

    // Initialize basic fields
    memset(viz, 0, sizeof(Visualization));
    viz->color_enabled = false;
    viz->screen_height = 25;
    viz->screen_width = 80;

    // Initialize signal history
    init_signal_history(&viz->signal_history, 100);

    viz_initialized = true;
}

// Create main window layout
void create_main_windows(Visualization* viz) {
    if (!viz) return;
    // For now, we'll use stdout for display
    printf("Visualization initialized (text mode)\n");
}

// Create intersection status window
void create_intersection_window(Visualization* viz) {
    if (!viz) return;
    // Placeholder for terminal-based intersection display
}

// Create metrics window
void create_metrics_window(Visualization* viz) {
    if (!viz) return;
    // Placeholder for terminal-based metrics display
}

// Create Gantt chart window
void create_gantt_window(Visualization* viz) {
    if (!viz) return;
    // Placeholder for terminal-based Gantt chart
}

// Create signal sequence window
void create_signal_window(Visualization* viz) {
    if (!viz) return;
    // Placeholder for terminal-based signal display
}

// Create control instructions window
void create_control_window(Visualization* viz) {
    if (!viz) return;
    // Placeholder for terminal-based controls
}

// Create emergency window
void create_emergency_window(Visualization* viz) {
    if (!viz) return;
    // Placeholder for terminal-based emergency alerts
}

// Create algorithm display window
void create_algorithm_window(Visualization* viz) {
    if (!viz) return;
    // Placeholder for terminal-based algorithm display
}

// Create status bar window
void create_status_window(Visualization* viz) {
    if (!viz) return;
    // Placeholder for terminal-based status bar
}

// Clear window
void clear_window(WINDOW* win) {
    // Placeholder - in text mode, we don't have windows
    (void)win;
}

// Center text in window
void center_text(WINDOW* win, int y, const char* text) {
    // Placeholder - simple text centering for terminal
    (void)win; (void)y;
    if (text) {
        int len = strlen(text);
        int padding = (80 - len) / 2;
        printf("%*s%s\n", padding, "", text);
    }
}

// Draw box with title
void draw_box(WINDOW* win, const char* title) {
    // Placeholder - simple box drawing for terminal
    (void)win;
    if (title) {
        printf("\n=== %s ===\n", title);
    }
}

// Resize window
void resize_window(WINDOW** win, int height, int width, int start_y, int start_x) {
    // Placeholder - resizing not needed in text mode
    (void)win; (void)height; (void)width; (void)start_y; (void)start_x;
}

// Draw intersection status
void draw_intersection_status(Visualization* viz, LaneProcess lanes[4]) {
    if (!viz || !lanes) return;

    printf("\n=== INTERSECTION STATUS ===\n");
    const char* lane_names[] = {"North", "South", "East", "West"};
    const char* state_names[] = {"WAITING", "READY", "RUNNING", "BLOCKED"};

    for (int i = 0; i < 4; i++) {
        printf("%s Lane: %s (Queue: %d, Priority: %d)\n",
               lane_names[i],
               state_names[lanes[i].state],
               lanes[i].queue_length,
               lanes[i].priority);
    }
    printf("========================\n\n");
}

// Draw performance metrics dashboard
void draw_metrics_dashboard(Visualization* viz, PerformanceMetrics* metrics, SchedulingAlgorithm current_algo) {
    if (!viz || !metrics) return;

    printf("=== PERFORMANCE METRICS ===\n");
    const char* algo_names[] = {"SJF", "MULTILEVEL", "PRIORITY RR"};
    printf("Algorithm: %s\n", algo_names[current_algo]);
    printf("Throughput: %.2f veh/min\n", metrics->vehicles_per_minute);
    printf("Avg Wait Time: %.2fs\n", metrics->avg_wait_time);
    printf("Utilization: %.1f%%\n", metrics->utilization * 100);
    printf("Fairness: %.3f\n", metrics->fairness_index);
    printf("Total Vehicles: %d\n", metrics->total_vehicles_processed);
    printf("Context Switches: %d\n", metrics->context_switches);
    printf("Emergency Response: %.2fs\n", metrics->emergency_response_time);
    printf("Deadlocks Prevented: %d\n", metrics->deadlocks_prevented);
    printf("Queue Overflows: %d\n", metrics->queue_overflow_count);
    printf("=======================\n\n");
}

// Draw Gantt chart timeline
void draw_gantt_chart(Visualization* viz, ExecutionRecord* history, int record_count) {
    if (!viz || !history || record_count <= 0) return;

    printf("=== GANTT CHART ===\n");
    printf("Lane Execution Timeline:\n");

    const char* lane_names[] = {"North", "South", "East", "West"};
    int time_limit = 50; // 50 second timeline

    // Draw timeline
    printf("Time: ");
    for (int t = 0; t <= time_limit; t += 10) {
        printf("%3ds ", t);
    }
    printf("\n");

    // Draw execution blocks
    for (int lane = 0; lane < 4; lane++) {
        printf("%-6s: ", lane_names[lane]);

        for (int t = 0; t <= time_limit; t++) {
            int lane_executing = 0;
            for (int i = 0; i < record_count; i++) {
                if (history[i].lane_id == lane &&
                    t >= history[i].start_time &&
                    t < history[i].start_time + history[i].duration) {
                    lane_executing = 1;
                    break;
                }
            }
            printf("%-4s", lane_executing ? "█" : ".");
        }
        printf("\n");
    }
    printf("==================\n\n");
}

// Display current algorithm
void display_current_algorithm(Visualization* viz, SchedulingAlgorithm algorithm) {
    if (!viz) return;

    const char* algo_names[] = {"SJF", "MULTILEVEL", "PRIORITY RR"};
    printf("Current Algorithm: %s\n", algo_names[algorithm]);
}

// Display control instructions
void display_control_instructions(Visualization* viz) {
    if (!viz) return;

    printf("=== CONTROLS ===\n");
    printf("1-3: Switch scheduling algorithms\n");
    printf("SPACE: Pause/Resume simulation\n");
    printf("e: Trigger emergency vehicle\n");
    printf("r: Reset simulation\n");
    printf("q: Quit simulation\n");
    printf("h: Show this help\n");
    printf("===============\n\n");
}

// Display status bar
void display_status_bar(Visualization* viz, const char* status) {
    if (!viz) return;

    if (status) {
        printf("Status: %s\n", status);
    } else {
        time_t now = time(NULL);
        struct tm* tm_info = localtime(&now);
        printf("TrafficGuru v1.0 - %02d:%02d:%02d\n",
               tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
    }
}

// Display emergency alert
void display_emergency_alert(Visualization* viz, EmergencyVehicle* emergency) {
    if (!viz) return;

    if (emergency && emergency->active) {
        printf("\n🚨🚨🚨 EMERGENCY ALERT 🚨🚨🚨\n");
        const char* emergency_types[] = {"NONE", "AMBULANCE", "FIRE TRUCK", "POLICE"};
        const char* lane_names[] = {"North", "South", "East", "West"};

        printf("%s approaching %s lane! (ETA: %.1fs)\n",
               emergency_types[emergency->type],
               lane_names[emergency->lane_id],
               emergency->approach_time);
        printf("Response Time: %.1fs\n", emergency->crossing_duration);
        printf("🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨\n\n");
    } else {
        printf("No active emergency alerts\n");
    }
}

// Initialize signal history
void init_signal_history(SignalHistory* history, int capacity) {
    if (!history || capacity <= 0) return;

    history->events = (SignalEvent*)malloc(capacity * sizeof(SignalEvent));
    if (history->events) {
        history->capacity = capacity;
        history->size = 0;
        history->head = 0;
        history->tail = 0;
    }
}

// Destroy signal history
void destroy_signal_history(SignalHistory* history) {
    if (!history) return;

    if (history->events) {
        free(history->events);
        history->events = NULL;
    }

    history->capacity = 0;
    history->size = 0;
    history->head = 0;
    history->tail = 0;
}

// Add signal event to history
void add_signal_event(SignalHistory* history, int lane_id, int state, time_t timestamp) {
    if (!history || !history->events || lane_id < 0 || lane_id >= 4) return;

    SignalEvent* event = &history->events[history->tail];
    event->lane_id = lane_id;
    event->state = state;
    event->timestamp = timestamp;

    history->tail = (history->tail + 1) % history->capacity;
    if (history->size < history->capacity) {
        history->size++;
    } else {
        history->head = (history->head + 1) % history->capacity;
    }
}

// Draw signal sequence display
void draw_signal_sequence(Visualization* viz, SignalHistory* history) {
    if (!viz || !history || history->size == 0) return;

    printf("=== SIGNAL HISTORY ===\n");
    const char* lane_names[] = {"N", "S", "E", "W"};
    const char* signal_chars[] = {"R", "Y", "G"};

    // Show recent signal changes
    int display_count = history->size < 5 ? history->size : 5;
    for (int i = 0; i < display_count; i++) {
        int idx = (history->tail - 1 - i + history->capacity) % history->capacity;
        SignalEvent* event = &history->events[idx];

        time_t time_ago = time(NULL) - event->timestamp;
        printf("%s:%s (%lds ago)\n",
               lane_names[event->lane_id],
               signal_chars[event->state],
               time_ago);
    }
    printf("====================\n\n");
}

// Update signal display
void update_signal_display(Visualization* viz, int lane_id, int new_state, time_t timestamp) {
    if (!viz || lane_id < 0 || lane_id >= 4) return;

    add_signal_event(&viz->signal_history, lane_id, new_state, timestamp);
    draw_signal_sequence(viz, &viz->signal_history);
}

// Handle user input
int handle_user_input(Visualization* viz) {
    if (!viz) return -1;

    // Simple keyboard input handling
    // In a real implementation, this would use getch() from ncurses
    // For now, return 0 (no action)
    return 0;
}

// Refresh all windows
void refresh_all_windows(Visualization* viz) {
    if (!viz) return;

    // In text mode, we just print the current state
    display_status_bar(viz, NULL);
}

// Resize visualization on terminal size change
void resize_visualization(Visualization* viz) {
    if (!viz) return;

    // In text mode, no resizing needed
    viz->screen_height = 25;
    viz->screen_width = 80;
}

// Destroy visualization system
void destroy_visualization(Visualization* viz) {
    if (!viz) return;

    // Clean up signal history
    destroy_signal_history(&viz->signal_history);

    // Reset global state
    viz_initialized = false;
}

// Show help screen
void show_help_screen(Visualization* viz) {
    if (!viz) return;

    printf("\n=== TRAFFICGURU HELP ===\n");
    printf("CONTROLS:\n");
    printf("  1, 2, 3  - Switch scheduling algorithms\n");
    printf("    1 - Shortest Job First (SJF)\n");
    printf("    2 - Multilevel Feedback Queue\n");
    printf("    3 - Priority Round Robin\n");
    printf("  SPACE    - Pause/Resume simulation\n");
    printf("  e        - Trigger emergency vehicle\n");
    printf("  r        - Reset simulation\n");
    printf("  q        - Quit program\n");
    printf("  h        - This help screen\n\n");
    printf("ALGORITHMS:\n");
    printf("  SJF - Shortest Job First: Prioritizes lanes with shortest estimated processing time\n");
    printf("  Multilevel - Dynamic priority adjustment with aging to prevent starvation\n");
    printf("  Priority RR - Priority-based with time slicing and emergency preemption\n\n");
    printf("Press any key to continue...\n");
    getchar();
}

// Get color for lane state
int get_color_for_state(LaneState state) {
    // In text mode, we don't use colors
    (void)state;
    return 0;
}

// Get state name
const char* get_state_name(LaneState state) {
    switch (state) {
        case RUNNING: return "RUNNING";
        case READY: return "READY";
        case WAITING: return "WAITING";
        case BLOCKED: return "BLOCKED";
        default: return "UNKNOWN";
    }
}

// Real-time visualization functions
void display_real_time_status() {
    if (!g_traffic_system) return;

    // Clear screen for fresh display
    system("clear || cls");

    // Get current time for display
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);

    // Display header
    printf("╔══════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                    🚦 TrafficGuru Real-Time Simulation 🚦                  ║\n");
    printf("║                           Time: %s | Algorithm: %s                   ║\n",
           time_str, get_algorithm_name(g_traffic_system->scheduler.algorithm));
    printf("╚══════════════════════════════════════════════════════════════════════════════╝\n\n");

    // Display intersection status and vehicle details
    display_detailed_intersection_status(&g_traffic_system->visualization, g_traffic_system->lanes);
    display_detailed_vehicle_information(&g_traffic_system->visualization, g_traffic_system->lanes);

    // Display enhanced metrics
    display_enhanced_metrics_dashboard(&g_traffic_system->visualization,
                                      &g_traffic_system->metrics,
                                      g_traffic_system->scheduler.algorithm);

    // Display controls
    printf("╔══════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║  🎮 CONTROLS: 1-3: Algorithm | SPACE: Pause | E: Emergency | Q: Quit         ║\n");
    printf("╚══════════════════════════════════════════════════════════════════════════════╝\n");

    fflush(stdout);
}

void display_detailed_intersection_status(Visualization* viz, LaneProcess lanes[4]) {
    if (!viz || !lanes) return;

    printf("╔═════════════════════════════╦═══════════════════════════════════════════════╗\n");
    printf("║      INTERSECTION STATUS     ║             LANE SUMMARY                      ║\n");
    printf("╠═════════════════════════════╣═══════════════════════════════════════════════╣\n");
    printf("║                             ║                                                 ║\n");

    // Create ASCII intersection art
    create_intersection_ascii_art(lanes);

    printf("║                             ║                                                 ║\n");

    // Display lane summary
    const char* lane_names[] = {"NORTH", "SOUTH", "EAST", "WEST"};
    const char* state_emojis[] = {"🟢", "🟡", "🔴", "🚫"};
    const char* state_names[] = {"RUNNING", "READY", "WAITING", "BLOCKED"};

    for (int i = 0; i < 4; i++) {
        pthread_mutex_lock(&lanes[i].queue_lock);
        int queue_len = lanes[i].queue_length;
        int wait_time = lanes[i].waiting_time;
        pthread_mutex_unlock(&lanes[i].queue_lock);

        printf("║  %s: %s %s | Queue: %d | Wait: %ds      ║\n",
               lane_names[i],
               state_emojis[lanes[i].state],
               state_names[lanes[i].state],
               queue_len,
               wait_time);
    }

    printf("║                             ║                                                 ║\n");

    // Show active lane and emergency status
    int active_lane = -1;
    for (int i = 0; i < 4; i++) {
        if (lanes[i].state == RUNNING) {
            active_lane = i;
            break;
        }
    }

    if (active_lane != -1) {
        printf("║  Active Lane: %s | Signal: 🟢 GREEN         ║\n", lane_names[active_lane]);
    } else {
        printf("║  Active Lane: None | Signal: 🔴 RED           ║\n");
    }

    // Emergency status
    if (g_traffic_system->emergency_system.emergency_mode) {
        const char* emergency_types[] = {"NONE", "🚑 Ambulance", "🚒 Fire Truck", "🚓 Police"};
        printf("║  Emergency Status: %s Active             ║\n",
               emergency_types[g_traffic_system->emergency_system.current_emergency.type]);
    } else {
        printf("║  Emergency Status: 🚑 None Active             ║\n");
    }

    printf("║                             ║                                                 ║\n");
    printf("╚═════════════════════════════╩═══════════════════════════════════════════════╝\n\n");
}

void display_detailed_vehicle_information(Visualization* viz, LaneProcess lanes[4]) {
    if (!viz || !lanes) return;

    printf("╔══════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                           VEHICLE DETAILS BY LANE                            ║\n");
    printf("╠══════════════════════════════════════════════════════════════════════════════╣\n");
    printf("║                                                                              ║\n");

    const char* lane_names[] = {"NORTH", "SOUTH", "EAST", "WEST"};
    const char* state_names[] = {"RUNNING", "READY", "WAITING", "BLOCKED"};
    const char* state_emojis[] = {"🟢", "🟡", "🔴", "🚫"};

    for (int lane_idx = 0; lane_idx < 4; lane_idx++) {
        LaneProcess* lane = &lanes[lane_idx];

        pthread_mutex_lock(&lane->queue_lock);
        int queue_len = lane->queue_length;

        if (queue_len > 0) {
            printf("║  🚗 %s LANE (%d vehicles, State: %s %s)                                  ║\n",
                   lane_names[lane_idx], queue_len, state_emojis[lane->state], state_names[lane->state]);

            // Display individual vehicles
            if (lane->queue && lane->queue->vehicles) {
                for (int pos = 0; pos < queue_len && pos < 10; pos++) { // Limit to 10 vehicles for display
                    int vehicle_id = lane->queue->vehicles[pos];
                    time_t current_time = time(NULL);
                    int waiting_time = current_time - lane->last_arrival_time; // Estimated wait time

                    const char* status_indicator = "";
                    if (pos == 0 && lane->state == RUNNING) {
                        status_indicator = "⚡ Next in service";
                    } else if (pos == 0) {
                        status_indicator = "⚡ Ready for service";
                    } else if (waiting_time > 30) {
                        status_indicator = "⏳ Longest waiting";
                    } else {
                        char eta_str[20];
                        sprintf(eta_str, "ETA: %ds", pos * 3); // Rough estimate
                        status_indicator = eta_str;
                    }

                    printf("║     Car #%03d - Position %d - Waiting %ds - %s                 ║\n",
                           vehicle_id, pos + 1, waiting_time, status_indicator);
                }
            }

            // Lane statistics
            printf("║     Lane Stats: Served %d | Priority %d | Util %d%%                            ║\n",
                   lane->total_vehicles_served, lane->priority,
                   (int)((double)lane->total_vehicles_served / (lane->total_vehicles_served + queue_len) * 100));
        } else {
            printf("║  🚗 %s LANE (0 vehicles, State: %s %s)                                   ║\n",
                   lane_names[lane_idx], state_emojis[lane->state], state_names[lane->state]);
            printf("║     Lane Stats: Served %d | Priority %d | Util 0%%                             ║\n",
                   lane->total_vehicles_served, lane->priority);
        }

        pthread_mutex_unlock(&lane->queue_lock);
        printf("║                                                                              ║\n");
    }

    printf("╚══════════════════════════════════════════════════════════════════════════════╝\n\n");
}

void display_enhanced_metrics_dashboard(Visualization* viz, PerformanceMetrics* metrics, SchedulingAlgorithm current_algo) {
    if (!viz || !metrics) return;

    printf("╔══════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                          PERFORMANCE METRICS                                  ║\n");
    printf("╠══════════════════════════════════════════════════════════════════════════════╣\n");

    // Calculate trends (simplified)
    static double last_throughput = 0.0;
    static double last_wait_time = 0.0;
    static double last_utilization = 0.0;

    const char* throughput_trend = (metrics->vehicles_per_minute > last_throughput) ? "↑" :
                                   (metrics->vehicles_per_minute < last_throughput) ? "↓" : "→";
    const char* wait_trend = (metrics->avg_wait_time < last_wait_time) ? "↑" :
                            (metrics->avg_wait_time > last_wait_time) ? "↓" : "→";
    const char* util_trend = (metrics->utilization > last_utilization) ? "↑" :
                            (metrics->utilization < last_utilization) ? "↓" : "→";

    printf("║  Throughput: %.1f veh/min %s    Wait Time: %.1fs %s    Utilization: %.1f%% %s    ║\n",
           metrics->vehicles_per_minute, throughput_trend,
           metrics->avg_wait_time, wait_trend,
           metrics->utilization * 100, util_trend);

    printf("║  Total Served: %d             Fairness: %.3f%s      Context Switches: %d    ║\n",
           metrics->total_vehicles_processed,
           metrics->fairness_index, (metrics->fairness_index > 0.8) ? "↑" : "↓",
           metrics->context_switches);

    printf("║  Emergency Response: %.1fs       Deadlocks Prevented: %d  Queue Overflows: %d    ║\n",
           metrics->emergency_response_time,
           metrics->deadlocks_prevented,
           metrics->queue_overflow_count);

    printf("║                                                                              ║\n");

    // Algorithm performance
    const char* algo_names[] = {"SJF", "MULTILEVEL", "PRIORITY RR"};
    printf("║  Algorithm: %s | Time Quantum: %ds | Efficiency: %.1f%%                    ║\n",
           algo_names[current_algo],
           g_traffic_system->scheduler.time_quantum,
           (metrics->vehicles_per_minute > 0) ?
           (metrics->avg_wait_time / metrics->vehicles_per_minute) * 100 : 0.0);

    printf("╚══════════════════════════════════════════════════════════════════════════════╝\n\n");

    // Update previous values for trend calculation
    last_throughput = metrics->vehicles_per_minute;
    last_wait_time = metrics->avg_wait_time;
    last_utilization = metrics->utilization;
}

void create_intersection_ascii_art(LaneProcess lanes[4]) {
    if (!lanes) return;

    // Get vehicle counts for each lane
    int vehicle_counts[4];
    const char* vehicle_emojis[] = {"", "🚗", "🚗🚙", "🚗🚙🚕", "🚗🚙🚕🚌", "🚗🚙🚕🚌🚐"};

    for (int i = 0; i < 4; i++) {
        pthread_mutex_lock(&lanes[i].queue_lock);
        vehicle_counts[i] = lanes[i].queue_length;
        pthread_mutex_unlock(&lanes[i].queue_lock);
    }

    // Display intersection with vehicles
    printf("║    ▲ NORTH (%d car%s)         ║", vehicle_counts[0], (vehicle_counts[0] != 1) ? "s" : "");
    if (vehicle_counts[0] > 0 && vehicle_counts[0] <= 5) {
        printf("    │ %s", vehicle_emojis[vehicle_counts[0]]);
    } else if (vehicle_counts[0] > 5) {
        printf("    │ %s...", vehicle_emojis[5]);
    }
    printf("                 ║\n");

    printf("║    │                        ║                                                 ║\n");

    printf("║◄WEST─┼─EAST► (%d car%s)       ║", vehicle_counts[2], (vehicle_counts[2] != 1) ? "s" : "");
    if (vehicle_counts[2] > 0 && vehicle_counts[2] <= 5) {
        printf("    │   %s", vehicle_emojis[vehicle_counts[2]]);
    } else if (vehicle_counts[2] > 5) {
        printf("    │   %s...", vehicle_emojis[5]);
    }
    printf("                 ║\n");

    printf("║    ▼ SOUTH (%d car%s)          ║", vehicle_counts[1], (vehicle_counts[1] != 1) ? "s" : "");
    if (vehicle_counts[1] > 0 && vehicle_counts[1] <= 5) {
        printf("      %s", vehicle_emojis[vehicle_counts[1]]);
    } else if (vehicle_counts[1] > 5) {
        printf("      %s...", vehicle_emojis[5]);
    }
    printf("                 ║\n");

    // Find active lane for signal display
    const char* signal_emojis[] = {"🔴", "🟡", "🟢"};
    int active_lane = -1;
    for (int i = 0; i < 4; i++) {
        if (lanes[i].state == RUNNING) {
            active_lane = i;
            break;
        }
    }

    if (active_lane != -1) {
        const char* lane_names[] = {"NORTH", "SOUTH", "EAST", "WEST"};
        printf("║                             ║  Active Signal: %s %s               ║\n",
               signal_emojis[2], lane_names[active_lane]);
    } else {
        printf("║                             ║  Active Signal: 🔴 ALL RED             ║\n");
    }
}