#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

typedef struct {
    int* vehicles;                  // Array of vehicle positions (0 = front of queue)
    int front;                      // Index of queue front
    int rear;                       // Index of queue rear
    int size;                       // Current queue size
    int capacity;                   // Maximum queue capacity
    int enqueue_count;              // Total vehicles enqueued
    int dequeue_count;              // Total vehicles dequeued
    int overflow_count;             // Number of times queue overflowed
} Queue;

// Queue lifecycle functions
Queue* create_queue(int capacity);
void destroy_queue(Queue* queue);
void resize_queue(Queue* queue, int new_capacity);

// Core queue operations
bool enqueue(Queue* queue, int vehicle_id);
int dequeue(Queue* queue);
int peek(Queue* queue);

// Queue state functions
bool is_empty(Queue* queue);
bool is_full(Queue* queue);
int get_size(Queue* queue);
int get_capacity(Queue* queue);

// Queue utility functions
void clear_queue(Queue* queue);
void print_queue(Queue* queue);
float get_queue_utilization(Queue* queue);

// Performance metrics
int get_total_enqueues(Queue* queue);
int get_total_dequeues(Queue* queue);
int get_overflow_count(Queue* queue);
float get_average_queue_length(Queue* queue, time_t start_time);

#endif // QUEUE_H