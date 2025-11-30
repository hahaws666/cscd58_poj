#ifndef DATA_ANALYSIS_H
#define DATA_ANALYSIS_H

#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include "monitor.h"
// monitor data
typedef struct {
    char hostname[256];
    time_t timestamp;
    double rtt_ms;
    int ports[32];
    int status[32];
    int cnt;
    int ping;
} monitor_record_t;
//Returns number of records loaded.
size_t dataload(const char *file, monitor_record_t *records, size_t mx);
void datareport(monitor_record_t *records, size_t cnt, ping_stats_t *s);
#endif