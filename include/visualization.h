#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include <ncurses.h>
#include <time.h>
#include "scheduler.h" // For ExecutionRecord, SchedulingAlgorithm
#include "lane_process.h" // For LaneProcess

// --- FIX: Added this include ---
// This defines 'PerformanceMetrics' before it is used below.
#include "performance_metrics.h" 
// --- END FIX ---


// --- UPDATED STRUCT DEFINITION ---
// This struct now holds the ncurses window pointers
typedef struct {
    WINDOW* main_window;
    WINDOW* lane_window;
    WINDOW* metrics_window;
    WINDOW* status_window;
    WINDOW* help_window;
    
    int screen_height;
    int screen_width;
    bool color_enabled;
    
    // Moved SignalHistory definitions here
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
// --- END UPDATED STRUCT ---


// --- Signal History Structs (Moved here) ---
typedef struct SignalEvent_t {
    int lane_id;
    int state;
    time_t timestamp;
} SignalEvent;
// --- END Structs ---


// Lifecycle
void init_visualization(Visualization* viz);
void destroy_visualization(Visualization* viz);

// Main drawing loop
void display_real_time_status();

// Input
int handle_user_input(Visualization* viz);

// Help Screen
void display_help_screen(Visualization* viz); // <-- Correct signature

// Signal History
void init_signal_history(SignalHistory* history, int capacity);
void destroy_signal_history(SignalHistory* history);
void add_signal_event(SignalHistory* history, int lane_id, int state, time_t timestamp);

// Deprecated text-based functions (for compatibility)
void update_signal_display(Visualization* viz, int lane_id, int new_state, time_t timestamp);
void display_real_time_status_text();
void display_detailed_intersection_status(Visualization* viz, LaneProcess lanes[4]);
void display_enhanced_metrics_dashboard(Visualization* viz, PerformanceMetrics* metrics, SchedulingAlgorithm current_algo);

#endif // VISUALIZATION_H
