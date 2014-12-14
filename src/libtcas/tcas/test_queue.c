#include <stdio.h>
#include "queue.h"

void print(const int *p) {
    printf("%i\n", *p);
}

int main() {
    int elem;
    QueuePtr qptr;
    queue_create(&qptr, sizeof(int), 5, NULL, NULL);
    elem = 0;
    queue_push_rear(qptr, &elem);
    elem = 1;
    queue_push_rear(qptr, &elem);
    elem = 4;
    queue_push_rear(qptr, &elem);
    elem = 2;
    queue_push_rear(qptr, &elem);
    elem = 9;
    queue_push_rear(qptr, &elem);
    queue_pop_front(qptr, &elem);
    printf("popped: %i\n", elem);
    elem = 5;
    queue_push_rear(qptr, &elem);
    queue_pop_front(qptr, &elem);
    printf("popped: %i\n", elem);
    elem = 9;
    queue_push_rear(qptr, &elem);
    queue_traverse(qptr, (queue_traverse_func)print);
    queue_pop_front(qptr, &elem);
    printf("popped: %i\n", elem);
    queue_pop_front(qptr, &elem);
    printf("popped: %i\n", elem);
    queue_pop_front(qptr, &elem);
    printf("popped: %i\n", elem);
    queue_pop_front(qptr, &elem);
    printf("popped: %i\n", elem);
    queue_pop_front(qptr, &elem);
    printf("popped: %i\n", elem);
    queue_pop_front(qptr, &elem);
    printf("popped: %i\n", elem);
    queue_pop_front(qptr, &elem);
    printf("popped: %i\n", elem);
    elem = 0;
    queue_push_rear(qptr, &elem);
    elem = 1;
    queue_push_rear(qptr, &elem);
    elem = 4;
    queue_push_rear(qptr, &elem);
    elem = 2;
    queue_push_rear(qptr, &elem);
    elem = 9;
    queue_push_rear(qptr, &elem);
    queue_pop_front(qptr, &elem);
    printf("popped: %i\n", elem);
    elem = 5;
    queue_push_rear(qptr, &elem);
    queue_pop_front(qptr, &elem);
    printf("popped: %i\n", elem);
    elem = 9;
    queue_push_rear(qptr, &elem);
    queue_traverse(qptr, (queue_traverse_func)print);
    queue_destroy(qptr);
    return 0;
}

