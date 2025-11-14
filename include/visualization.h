/*
 * Visualization - Terminal UI Display and Input Handling
 *
 * Provides ncurses-based real-time visualization of traffic simulation.
 * Displays lanes, metrics, signal state, and emergency status.
 *
 * Features:
 * - Multi-window layout (lanes, metrics, status, help)
 * - Color support for visual distinction
 * - User input handling (pause, algorithm selection, emergency trigger)
 * - Signal history tracking
 * - Non-blocking input to prevent UI freezing
 * - Deadlock-free mutex usage with trylock instead of lock
 *
 * Controls: q=quit, Space=pause, 1-3=algorithm, e=emergency, h=help
 */

#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include <ncurses.h>
#include <time.h>
#include "scheduler.h"
#include "lane_process.h"
#include "performance_metrics.h"

typedef struct SignalEvent_t {
    int lane_id;
    int state;
    time_t timestamp;
} SignalEvent;

typedef struct {
    struct SignalEvent_t* events;
    int capacity;
    int size;
    int head;
    int tail;
} SignalHistory;

typedef struct {
    WINDOW* main_window;
    WINDOW* lane_window;
    WINDOW* metrics_window;
    WINDOW* status_window;
    WINDOW* help_window;
    int screen_height;
    int screen_width;
    bool color_enabled;
    SignalHistory signal_history;
} Visualization;

void init_visualization(Visualization* viz);
void destroy_visualization(Visualization* viz);

void display_real_time_status();

int handle_user_input(Visualization* viz);

void display_help_screen(Visualization* viz);

void init_signal_history(SignalHistory* history, int capacity);
void destroy_signal_history(SignalHistory* history);
void add_signal_event(SignalHistory* history, int lane_id, int state, time_t timestamp);

void update_signal_display(Visualization* viz, int lane_id, int new_state, time_t timestamp);
void display_real_time_status_text();
void display_detailed_intersection_status(Visualization* viz, LaneProcess lanes[4]);
void display_enhanced_metrics_dashboard(Visualization* viz, PerformanceMetrics* metrics, SchedulingAlgorithm current_algo);

#endif
