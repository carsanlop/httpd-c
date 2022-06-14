#include <stdio.h>
#include <stdlib.h>

typedef struct QueueNode {
    struct QueueNode    *prev;
    struct QueueNode    *next;
    char                *content;
} QueueNode;

typedef struct Queue {
    unsigned int        count;
    unsigned int        total;
    QueueNode           *front;
    QueueNode           *rear;
} Queue;

// Hash: Collection of pointers to Queue Nodes.
typedef struct QueueHash {
    int         capacity; // Maximum capacity.
    QueueNode   **array;
} QueueHash;

QueueNode* create_node(char *content) {
    QueueNode *node = malloc(sizeof(QueueNode));

    node->content = malloc(strlen(content) + 1);
    strcopy(node->content, content);

    node->prev = NULL;
    node->next = NULL;

    return node;
}

Queue* create_queue(int total) {
    Queue *queue = malloc(sizeof(Queue));

    queue->count = 0;
    queue->total = total;

    queue->front = NULL;
    queue->rear = NULL;

    return queue;
}

void Dequeue(Queue* queue) {
    //if empty return
    if (queue->rear == NULL) {
        return;
    }

    // If the queue only has a node,.
    if (queue->front == queue->rear) {
        queue->front = NULL;
    }


    // Set a new rear, and remove the old one.
    QueueNode *node = queue->rear;
    queue->rear = node->prev;

    if (queue->rear) {
        queue->rear->next = NULL;
    }

    free(node->content);
    free(node);

    queue->count--;
}