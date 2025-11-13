#include "../include/queue.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>

// Create a new queue with specified capacity
Queue* create_queue(int capacity) {
    if (capacity <= 0) {
        return NULL;
    }

    Queue* queue = (Queue*)malloc(sizeof(Queue));
    if (!queue) {
        return NULL;
    }

    queue->vehicles = (int*)malloc(capacity * sizeof(int));
    if (!queue->vehicles) {
        free(queue);
        return NULL;
    }

    queue->front = 0;
    queue->rear = -1;
    queue->size = 0;
    queue->capacity = capacity;
    queue->enqueue_count = 0;
    queue->dequeue_count = 0;
    queue->overflow_count = 0;

    return queue;
}

// Destroy queue and free memory
void destroy_queue(Queue* queue) {
    if (queue) {
        free(queue->vehicles);
        free(queue);
    }
}

// Resize queue to new capacity
void resize_queue(Queue* queue, int new_capacity) {
    if (!queue || new_capacity <= 0 || new_capacity < queue->size) {
        return;
    }

    int* new_vehicles = (int*)malloc(new_capacity * sizeof(int));
    if (!new_vehicles) {
        return;
    }

    // Copy existing elements to new array
    for (int i = 0; i < queue->size; i++) {
        int index = (queue->front + i) % queue->capacity;
        new_vehicles[i] = queue->vehicles[index];
    }

    free(queue->vehicles);
    queue->vehicles = new_vehicles;
    queue->capacity = new_capacity;
    queue->front = 0;
    queue->rear = queue->size - 1;
}

// Add vehicle to queue rear
bool enqueue(Queue* queue, int vehicle_id) {
    if (!queue) {
        return false;
    }

    if (is_full(queue)) {
        queue->overflow_count++;
        return false;
    }

    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->vehicles[queue->rear] = vehicle_id;
    queue->size++;
    queue->enqueue_count++;

    return true;
}

// Remove vehicle from queue front
int dequeue(Queue* queue) {
    if (!queue || is_empty(queue)) {
        return -1; // Error value
    }

    int vehicle_id = queue->vehicles[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;
    queue->dequeue_count++;

    // Reset front and rear when queue becomes empty
    if (queue->size == 0) {
        queue->front = 0;
        queue->rear = -1;
    }

    return vehicle_id;
}

// Peek at front vehicle without removing
int peek(Queue* queue) {
    if (!queue || is_empty(queue)) {
        return -1;
    }

    return queue->vehicles[queue->front];
}

// Check if queue is empty
bool is_empty(Queue* queue) {
    return queue ? queue->size == 0 : true;
}

// Check if queue is full
bool is_full(Queue* queue) {
    return queue ? queue->size == queue->capacity : true;
}

// Get current queue size
int get_size(Queue* queue) {
    return queue ? queue->size : 0;
}

// Get queue capacity
int get_capacity(Queue* queue) {
    return queue ? queue->capacity : 0;
}

// Clear all vehicles from queue
void clear_queue(Queue* queue) {
    if (queue) {
        queue->front = 0;
        queue->rear = -1;
        queue->size = 0;
    }
}

// Print queue contents for debugging
void print_queue(Queue* queue) {
    if (!queue) {
        printf("Queue: NULL\n");
        return;
    }

    printf("Queue (size=%d, capacity=%d): [", queue->size, queue->capacity);

    if (queue->size > 0) {
        for (int i = 0; i < queue->size; i++) {
            int index = (queue->front + i) % queue->capacity;
            printf("%d", queue->vehicles[index]);
            if (i < queue->size - 1) {
                printf(", ");
            }
        }
    }

    printf("]\n");
}

// Get queue utilization as percentage
float get_queue_utilization(Queue* queue) {
    if (!queue || queue->capacity == 0) {
        return 0.0f;
    }

    return (float)queue->size / queue->capacity * 100.0f;
}

// Get total number of enqueued vehicles
int get_total_enqueues(Queue* queue) {
    return queue ? queue->enqueue_count : 0;
}

// Get total number of dequeued vehicles
int get_total_dequeues(Queue* queue) {
    return queue ? queue->dequeue_count : 0;
}

// Get overflow count
int get_overflow_count(Queue* queue) {
    return queue ? queue->overflow_count : 0;
}

// Calculate average queue length over time period
float get_average_queue_length(Queue* queue, time_t start_time) {
    if (!queue || start_time <= 0) {
        return 0.0f;
    }

    time_t current_time = time(NULL);
    time_t elapsed_time = current_time - start_time;

    if (elapsed_time <= 0) {
        return (float)queue->size;
    }

    // Simple approximation: current size represents average
    // In a more sophisticated implementation, we would track size over time
    return (float)queue->size;
}