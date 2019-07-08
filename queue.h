#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>

typedef struct theader theader;

typedef struct {
    theader* (* const create)();
    void (* const destroy)(theader *handle);
    void (* const push)(theader *handle, void *data);
    void *(* const pop)(theader *handle);
} _tqueue;

extern _tqueue const tqueue;


#endif // QUEUE_H
