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
    int port_count;
    int ping_success;
} monitor_record_t;

/*
 * Keeps track of uptime information.
 */
typedef struct {
    time_t start_time;
    time_t last_change;
    uint64_t up_intervals;
    uint64_t down_intervals;
    int current_state; // 1 = up, 0 = down
} uptime_tracker_t;


/*
 * Append a monitoring record to persistent storage.
 */
int data_store_append(const char *filepath, const monitor_record_t *record);

/*
 * Load historical records from storage.
 */
size_t data_store_load(const char *filepath,
                       monitor_record_t *records,
                       size_t max_records);

/*
 * Generate statistical report.
 */
void data_generate_report(const monitor_record_t *records,
                          size_t count,
                          ping_stats_t *out_stats);

/*
 * Update uptime tracker.
 */
void uptime_tracker_update(uptime_tracker_t *tracker, int is_up);
double uptime_tracker_percentage(const uptime_tracker_t *tracker);

/*
 * Check triggers.
 */
int alert_check_trigger(const alert_config_t *config,
                        const ping_stats_t *stats,
                        double current_latency_ms,
                        double uptime_pct,
                        char *message_buf,
                        size_t message_len);

#endif /* DATA_ANALYSIS_H */