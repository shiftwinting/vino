/*
 *  Copyright (C) Chenyang Li
 *  Copyright (C) Vino
 */
#include <sys/time.h>

#include "vn_event_timer.h"
#include "vn_priority_queue.h"
#include "error.h"

extern vn_priority_queue pq;

int vn_event_timer_init(void) {
    vn_pq_init(&pq);
    vn_time_update();
    return 0;
}

void vn_time_update(void) {
    struct timeval tv;
    vn_sec_t sec;
    vn_msec_t msec;

    if (gettimeofday(&tv, NULL) < 0) {
        err_sys("[vn_time_update] gettimeofday error");
    }
    sec = tv.tv_sec;
    msec = tv.tv_usec / 1000;

    vn_current_msec = sec * 1000 + msec;
}

vn_msec_t vn_event_find_timer(void) {
    long timer;
    vn_priority_queue_node node;

    if (vn_pq_isempty(&pq)) {
        return 0;
    }

    node = vn_pq_min(&pq);
    timer = node.key - vn_current_msec;
    return timer > 0 ? timer : 0;
}

void vn_event_expire_timers(void) {
    vn_priority_queue_node node;
    vn_http_event *event;

    while (!vn_pq_isempty(&pq)) {
        node = vn_pq_min(&pq);
        /* node.key > vn_current_msec */
        if (node.key - vn_current_msec > 0) {
            return;
        }

        node = vn_pq_delete_min(&pq);
        event = node.data;
        if (!(*event->handler)) {
            (*event->handler)(event);
        }
    }
}

void vn_event_add_timer(vn_http_event *event, vn_msec_t timer) {
    vn_priority_queue_node node;
    vn_msec_t key;

    key = vn_current_msec + timer;
    node.key = key;
    node.data = event;
    vn_pq_insert(&pq, node);
}