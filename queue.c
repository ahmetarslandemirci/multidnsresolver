#include "queue.h"
#include <pthread.h>
#include <stdlib.h>

typedef struct telement {
    void *next;
    void *data;
} telement;

struct theader {
    telement *head;
    telement *tail;
    pthread_mutex_t *mutex;
};

static theader* create();
static theader* create() {
    theader *handle = malloc(sizeof(*handle));
    handle->head = NULL;
    handle->tail = NULL;

    pthread_mutex_t *mutex = malloc(sizeof(*mutex));
    handle->mutex = mutex;

    return handle;
}

static void destroy(theader *header);
static void destroy(theader *header) {
    free(header->mutex);
    free(header);
    header = NULL;
}

static void push(theader *header, void *elem);
static void push(theader *header, void *elem) {
    // Create new element
    telement *element = malloc(sizeof(*element));
    element->data = elem;
    element->next = NULL;

    pthread_mutex_lock(header->mutex);
    // Is list empty
    if (header->head == NULL) {
        header->head = element;
        header->tail = element;
    } else {
        // Rewire
        telement* oldTail = header->tail;
        oldTail->next = element;
        header->tail = element;
    }
    pthread_mutex_unlock(header->mutex);
}

static void* pop(theader *header);
static void* pop(theader *header) {
    pthread_mutex_lock(header->mutex);
    telement *head = header->head;

    // Is empty?
    if (head == NULL) {
        pthread_mutex_unlock(header->mutex);
        return NULL;
    } else {
        // Rewire
        header->head = head->next;

        // Get head and free element memory
        void *value = head->data;
        free(head);

        pthread_mutex_unlock(header->mutex);
        return value;
    }
}

_tqueue const tqueue = {
        create,
        destroy,
        push,
        pop
};