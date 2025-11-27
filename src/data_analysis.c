#include "data_analysis.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int data_store_append(const char *filepath, const monitor_record_t *record) {
    if (!filepath || !record) return -1;

    FILE *fp = fopen(filepath, "a");
    if (!fp) return -1;

    fprintf(fp, "%ld,%s,%d,%.3f,%d",
            (long)record->timestamp,
            record->hostname,
            record->ping_success,
            record->rtt_ms,
            record->port_count);

    for (int i = 0; i < record->port_count; i++) {
        fprintf(fp, ",%d:%d",
                record->port_status[i].port,
                (int)record->port_status[i].status);
    }
    fputc('\n', fp);
    fclose(fp);
    return 0;
}

size_t data_store_load(const char *filepath,
                       monitor_record_t *records,
                       size_t max_records) {
    if (!filepath || !records || max_records == 0) return 0;

    FILE *fp = fopen(filepath, "r");
    if (!fp) return 0;

    char line[1024];
    size_t loaded = 0;
    while (loaded < max_records && fgets(line, sizeof(line), fp)) {
        char *token = strtok(line, ",\n");
        if (!token) continue;

        monitor_record_t rec = {0};
        rec.timestamp = (time_t)strtol(token, NULL, 10);

        token = strtok(NULL, ",\n");
        if (!token) continue;
        strncpy(rec.hostname, token, sizeof(rec.hostname) - 1);

        token = strtok(NULL, ",\n");
        if (!token) continue;
        rec.ping_success = atoi(token);

        token = strtok(NULL, ",\n");
        if (!token) continue;
        rec.rtt_ms = strtod(token, NULL);

        token = strtok(NULL, ",\n");
        if (!token) continue;
        rec.port_count = atoi(token);
        if (rec.port_count < 0) rec.port_count = 0;
        if (rec.port_count > 32) rec.port_count = 32;

        for (int i = 0; i < rec.port_count; i++) {
            token = strtok(NULL, ",\n");
            if (!token) break;
            int port = 0;
            int status = 0;
            if (sscanf(token, "%d:%d", &port, &status) == 2) {
                rec.port_status[i].port = port;
                if (status < PORT_UNKNOWN) status = PORT_UNKNOWN;
                if (status > PORT_TIMEOUT) status = PORT_TIMEOUT;
                rec.port_status[i].status = (port_status_t)status;
            }
        }

        records[loaded++] = rec;
    }

    fclose(fp);
    return loaded;
}

void data_generate_report(const monitor_record_t *records,
                          size_t count,
                          ping_stats_t *out_stats) {
    if (!out_stats) return;

    stats_init(out_stats);
    if (!records) return;

    for (size_t i = 0; i < count; i++) {
        const monitor_record_t *rec = &records[i];
        stats_update_ping(out_stats, rec->ping_success, rec->rtt_ms);
    }
}

void uptime_tracker_update(uptime_tracker_t *tracker, int is_up) {
    if (!tracker) return;

    time_t now = time(NULL);
    if (tracker->start_time == 0) {
        tracker->start_time = now;
        tracker->last_change = now;
        tracker->current_state = is_up ? 1 : 0;
    }

    if (is_up) {
        tracker->up_intervals++;
    } else {
        tracker->down_intervals++;
    }

    if (tracker->current_state != (is_up ? 1 : 0)) {
        tracker->current_state = is_up ? 1 : 0;
        tracker->last_change = now;
    }
}

double uptime_tracker_percentage(const uptime_tracker_t *tracker) {
    if (!tracker) return 0.0;
    uint64_t total = tracker->up_intervals + tracker->down_intervals;
    if (total == 0) return 0.0;
    return (double)tracker->up_intervals * 100.0 / (double)total;
}

int alert_check_trigger(const alert_config_t *config,
                        const ping_stats_t *stats,
                        double current_latency_ms,
                        double uptime_pct,
                        char *message_buf,
                        size_t message_len) {
    if (!config || !message_buf || message_len == 0) return 0;

    int triggered = 0;
    size_t written = 0;
    message_buf[0] = '\0';

    if (config->max_latency_ms > 0.0 &&
        current_latency_ms > config->max_latency_ms) {
        int n = snprintf(message_buf + written, message_len - written,
                         "Latency %.2fms exceeds %.2fms. ",
                         current_latency_ms, config->max_latency_ms);
        if (n > 0 && (size_t)n < message_len - written) written += (size_t)n;
        triggered = 1;
    }

    if (stats && config->max_loss_rate > 0.0 &&
        stats->loss_rate > config->max_loss_rate) {
        int n = snprintf(message_buf + written, message_len - written,
                         "Loss %.2f%% exceeds %.2f%%. ",
                         stats->loss_rate * 100.0,
                         config->max_loss_rate * 100.0);
        if (n > 0 && (size_t)n < message_len - written) written += (size_t)n;
        triggered = 1;
    }

    if (config->min_uptime_pct > 0.0 &&
        uptime_pct < config->min_uptime_pct) {
        int n = snprintf(message_buf + written, message_len - written,
                         "Uptime %.2f%% below %.2f%%. ",
                         uptime_pct, config->min_uptime_pct);
        if (n > 0 && (size_t)n < message_len - written) written += (size_t)n;
        triggered = 1;
    }

    if (!triggered) {
        snprintf(message_buf, message_len, "All thresholds normal.");
    }

    return triggered;
}

