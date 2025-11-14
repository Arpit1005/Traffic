/*
 * Queue - FIFO Data Structure for Vehicle Management
 *
 * Implements a circular queue for managing vehicle IDs at traffic lanes.
 * Provides FIFO insertion/removal with capacity limits and overflow tracking.
 *
 * Key Features:
 * - Dynamic queue resizing
 * - Circular array implementation for efficient memory usage
 * - Queue overflow detection and counting
 * - Statistics tracking (enqueue/dequeue/overflow counts)
 *
 * Used By: Lane processes for queuing arriving vehicles awaiting intersection access
 */

#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

typedef struct {
    int* vehicles;
    int front;
    int rear;
    int size;
    int capacity;
    int enqueue_count;
    int dequeue_count;
    int overflow_count;
} Queue;

Queue* create_queue(int capacity);
void destroy_queue(Queue* queue);
void resize_queue(Queue* queue, int new_capacity);

bool enqueue(Queue* queue, int vehicle_id);
int dequeue(Queue* queue);
int peek(Queue* queue);

bool is_empty(Queue* queue);
bool is_full(Queue* queue);
int get_size(Queue* queue);
int get_capacity(Queue* queue);

void clear_queue(Queue* queue);
void print_queue(Queue* queue);
float get_queue_utilization(Queue* queue);

int get_total_enqueues(Queue* queue);
int get_total_dequeues(Queue* queue);
int get_overflow_count(Queue* queue);
float get_average_queue_length(Queue* queue, time_t start_time);

#endif