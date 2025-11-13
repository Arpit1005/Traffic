#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include <ncurses.h>
#include <time.h>
#include <stdbool.h>
#include "lane_process.h"
#include "scheduler.h"
#include "performance_metrics.h"
#include "emergency_system.h"

typedef struct {
    int lane_id;                         // Lane identifier
    int state;                           // Signal state (0=red, 1=yellow, 2=green)
    time_t timestamp;                    // When signal change occurred
} SignalEvent;

typedef struct {
    SignalEvent* events;                 // Array of signal events
    int capacity;                        // Buffer capacity
    int size;                            // Current number of events
    int head;                            // Head index for circular buffer
    int tail;                            // Tail index for circular buffer
} SignalHistory;

typedef struct {
    WINDOW* main_window;                 // Primary ncurses window
    WINDOW* intersection_win;            // Intersection status display
    WINDOW* metrics_win;                 // Performance metrics panel
    WINDOW* gantt_win;                   // Gantt chart timeline
    WINDOW* signal_win;                  // Signal sequence display
    WINDOW* control_win;                 // Control instructions
    WINDOW* emergency_win;               // Emergency vehicle alerts
    WINDOW* algorithm_win;               // Current algorithm display
    WINDOW* status_win;                  // System status bar
    chtype colors[8];                    // Color pairs for different states
    bool color_enabled;                  // Color support flag
    int screen_height;                   // Terminal height
    int screen_width;                    // Terminal width
    SignalHistory signal_history;         // Signal change history buffer
} Visualization;

// Color definitions
#define COLOR_RED_BLACK      1
#define COLOR_GREEN_BLACK    2
#define COLOR_YELLOW_BLACK   3
#define COLOR_WHITE_BLACK    4
#define COLOR_CYAN_BLACK     5
#define COLOR_MAGENTA_BLACK  6
#define COLOR_BLUE_BLACK     7
#define COLOR_EMERGENCY       8

// Visualization lifecycle functions
void init_visualization(Visualization* viz);
void destroy_visualization(Visualization* viz);
void resize_visualization(Visualization* viz);
void refresh_all_windows(Visualization* viz);

// Window creation functions
void create_main_windows(Visualization* viz);
void create_intersection_window(Visualization* viz);
void create_metrics_window(Visualization* viz);
void create_gantt_window(Visualization* viz);
void create_signal_window(Visualization* viz);
void create_control_window(Visualization* viz);
void create_emergency_window(Visualization* viz);
void create_algorithm_window(Visualization* viz);
void create_status_window(Visualization* viz);

// Color and appearance functions
void init_colors();
void set_color_pair(int pair_number, short foreground, short background);
bool has_color_support();
void apply_theme(Visualization* viz);

// Intersection display functions
void draw_intersection_status(Visualization* viz, LaneProcess lanes[4]);
void draw_lane_status(Visualization* viz, LaneProcess* lane, int y, int x);
void draw_traffic_lights(Visualization* viz, LaneProcess lanes[4]);
void draw_queue_bars(Visualization* viz, LaneProcess lanes[4]);

// Signal sequence visualization functions
void init_signal_history(SignalHistory* history, int capacity);
void destroy_signal_history(SignalHistory* history);
void add_signal_event(SignalHistory* history, int lane_id, int state, time_t timestamp);
void draw_signal_sequence(Visualization* viz, SignalHistory* history);
void update_signal_display(Visualization* viz, int lane_id, int new_state, time_t timestamp);

// Gantt chart visualization functions
void draw_gantt_chart(Visualization* viz, ExecutionRecord* history, int record_count);
void draw_gantt_timeline(Visualization* viz, int time_units);
void draw_gantt_lanes(Visualization* viz);
void draw_execution_block(Visualization* viz, int lane_id, int start_pos, int duration);
void update_gantt_chart(Visualization* viz, Scheduler* scheduler);

// Performance metrics display functions
void draw_metrics_dashboard(Visualization* viz, PerformanceMetrics* metrics, SchedulingAlgorithm current_algo);
void draw_throughput_metrics(Visualization* viz, PerformanceMetrics* metrics);
void draw_performance_metrics(Visualization* viz, PerformanceMetrics* metrics);
void draw_system_metrics(Visualization* viz, PerformanceMetrics* metrics);
void update_metrics_display(Visualization* viz, PerformanceMetrics* metrics, SchedulingAlgorithm algo);

// Emergency vehicle visualization functions
void display_emergency_alert(Visualization* viz, EmergencyVehicle* emergency);
void highlight_emergency_lane(Visualization* viz, int lane_id);
void show_emergency_response_timer(Visualization* viz, float remaining_time);
void clear_emergency_display(Visualization* viz);
void flash_emergency_alert(Visualization* viz, const char* message);

// Algorithm and control display functions
void display_current_algorithm(Visualization* viz, SchedulingAlgorithm algorithm);
void display_control_instructions(Visualization* viz);
void display_help_screen(Visualization* viz);
void display_status_bar(Visualization* viz, const char* status);

// Interactive control handling
int handle_user_input(Visualization* viz);
void process_algorithm_switch(Visualization* viz, int new_algorithm);
void process_pause_resume(Visualization* viz);
void process_emergency_trigger(Visualization* viz);
void process_reset(Visualization* viz);

// Animation and real-time updates
void animate_signal_change(Visualization* viz, int lane_id, int from_state, int to_state);
void animate_vehicle_movement(Visualization* viz, int lane_id, int from_pos, int to_pos);
void update_real_time_displays(Visualization* viz, LaneProcess lanes[4],
                               PerformanceMetrics* metrics, Scheduler* scheduler);

// Utility functions
void clear_window(WINDOW* win);
void center_text(WINDOW* win, int y, const char* text);
void draw_box(WINDOW* win, const char* title);
int get_color_for_state(LaneState state);
const char* get_state_name(LaneState state);
void resize_window(WINDOW** win, int height, int width, int start_y, int start_x);

// Real-time visualization functions
void display_real_time_status();
void display_detailed_intersection_status(Visualization* viz, LaneProcess lanes[4]);
void display_detailed_vehicle_information(Visualization* viz, LaneProcess lanes[4]);
void display_enhanced_metrics_dashboard(Visualization* viz, PerformanceMetrics* metrics, SchedulingAlgorithm current_algo);
void create_intersection_ascii_art(LaneProcess lanes[4]);

// Error handling
bool validate_visualization_state(Visualization* viz);
void handle_ncurses_error(const char* operation);
void cleanup_visualization_on_error(Visualization* viz);

#endif // VISUALIZATION_H