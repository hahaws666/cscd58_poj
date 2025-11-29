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
    port_stats_t port_status[32];
    int cnt;
    int ping;
} monitor_record_t;
// uptime info
typedef struct {
    time_t start;
    time_t last;
    uint64_t up;
    uint64_t down;
    int cur; //1 =up 0 = down
} uptime_tracker_t;
// alert config
typedef struct {
    double mx_latency;
    double mx_loss;
    double mn_time;
} alert_config_t;
//Returns number of records loaded.
size_t dataload(const char *file, monitor_record_t *records, size_t mx);
void datareport(monitor_record_t *records, size_t cnt, ping_stats_t *s);
// refactor these two later...
void uptime_tracker_update(uptime_tracker_t *tracker, int is_up);
double uptime_tracker_percentage(const uptime_tracker_t *tracker);
#endif