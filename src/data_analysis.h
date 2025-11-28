#ifndef DATA_ANALYSIS_H
#define DATA_ANALYSIS_H

#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include "monitor.h"

/*
 * Historical data for a single monitoring sample.
 */
typedef struct {
    char hostname[256];
    time_t timestamp;
    double rtt_ms;
    port_stats_t port_status[32];
    int cnt;
    int ping;
} monitor_record_t;

/*
 * Keeps track of uptime information.
 */
typedef struct {
    time_t start;
    time_t last_change;
    uint64_t up;
    uint64_t down;
    int current_state; // 1 = up, 0 = down
} uptime_tracker_t;

/*
 * Threshold configuration for alerting.
 */
typedef struct {
    double mx_latency;
    double mx_loss;
    double mn_time;
} alert_config_t;

/*
 * Returns 0 on success.
 */
int dataappend(const char *path, const monitor_record_t *record);

/*
 * Returns number of records loaded.
 */
size_t dataload(const char *path, monitor_record_t *records, size_t mx);

void datareport(const monitor_record_t *records, size_t count, ping_stats_t *out_stats);

void uptime_tracker_update(uptime_tracker_t *tracker, int is_up);

double uptime_tracker_percentage(const uptime_tracker_t *tracker);

int alert_check_trigger(const alert_config_t *config, const ping_stats_t *stats, double current_latency_ms, double uptime_pct, char *message_buf, size_t message_len);

#endif /* DATA_ANALYSIS_H */