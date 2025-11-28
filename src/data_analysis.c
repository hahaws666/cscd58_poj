#include "data_analysis.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// return -1 if failed 0 if success
int dataappend(const char *file, const monitor_record_t *record) {
    FILE *fp = fopen(file, "a");
    if (!fp) return -1;
    fprintf(fp, "%ld,%s,%d,%.3f,%d", (long)record->timestamp, record->hostname, record->ping, record->rtt_ms, record->cnt);

    for (int i = 0; i < record->cnt; i++) {
        fprintf(fp, ",%d:%d", record->port_status[i].port, (int)record->port_status[i].status);
    }
    fputc('\n', fp);
    // close the file for closing
    fclose(fp);
    return 0;
}

// return the size of loaded
size_t dataload(const char *file, monitor_record_t *records, size_t mx) {
    FILE *fp = fopen(file, "r");
    if (!fp) return -1;
    char line[1024];
    size_t ans = 0;
    while (ans < mx && fgets(line, sizeof(line), fp)) {
        char *token = strtok(line, ",\n");
        monitor_record_t rec = {0};
        rec.timestamp = (time_t)strtol(token, NULL, 10);
        token = strtok(NULL, ",\n");
        strncpy(rec.hostname, token, sizeof(rec.hostname) - 1);
        token = strtok(NULL, ",\n");
        rec.ping = atoi(token);
        token = strtok(NULL, ",\n");
        rec.rtt_ms = strtod(token, NULL);
        token = strtok(NULL, ",\n");
        rec.cnt = atoi(token);
        for (int i = 0; i < rec.cnt; i++) {
            token = strtok(NULL, ",\n");
            int port = 0;
            int status = 0;
            if (sscanf(token, "%d:%d", &port, &status) == 2) {
                rec.port_status[i].port = port;
                // 更新状态
                if (status < PORT_UNKNOWN) status = PORT_UNKNOWN;
                if (status > PORT_TIMEOUT) status = PORT_TIMEOUT;
                rec.port_status[i].status = (port_status_t)status;
            }
        }
        records[ans++] = rec;
    }

    fclose(fp);
    return ans;
}

void datareport(const monitor_record_t *records, size_t count, ping_stats_t *s) {
    s->total_sent = 0;
    s->total_received = 0;
    s->last_rtt_ms = 0.0;
    s->min_rtt_ms = 1e9;
    s->max_rtt_ms = 0.0;
    s->sum_rtt_ms = 0.0;
    s->loss_rate = 0.0;
    for (size_t i = 0; i < count; i++) {
        const monitor_record_t *rec = &records[i];
        statsUpdate(s, rec->ping, rec->rtt_ms);
    }
}

void uptime_tracker_update(uptime_tracker_t *tracker, int up) {
    // get the current time
    time_t now = time(NULL);
    if (tracker->start == 0) {
        tracker->start = now;
        tracker->last_change = now;
        tracker->current_state = up ? 1 : 0;
    }

    if (up) tracker->up++;
    else tracker->down++;
    

    if (tracker->current_state != (up ? 1 : 0)) {
        tracker->current_state = up ? 1 : 0;
        tracker->last_change = now;
    }
}

double uptime_tracker_percentage(const uptime_tracker_t *tracker) {
    uint64_t total = tracker->up + tracker->down;
    if (total == 0) return 0.0;
    return (double)tracker->up * 100.0 / (double)total;
}

// return 1 if something is triggered else return 0
int alert_check_trigger(const alert_config_t *config, const ping_stats_t *stats, double late, double time, char *record, size_t len) {
    int ans = 0;
    size_t res = 0;
    record[0] = '\0';

    if (config->mx_latency > 0.0 && late > config->mx_latency) {
        int n = snprintf(record + res, len - res, "Latency %.2fms exceeds by %.2fms.", late, config->mx_latency);
        if (n > 0 && (size_t)n < len - res) res += (size_t)n;
        ans = 1;
    }

    if (stats && config->mx_loss > 0.0 && stats->loss_rate > config->mx_loss) {
        int n = snprintf(record + res, len - res, "Loss %.2f%% exceeds by that much: %.2f%%.", stats->loss_rate * 100.0, config->mx_loss * 100.0);
        if (n > 0 && (size_t)n < len - res) res += (size_t)n;
        ans = 1;
    }

    if (config->min_time > 0.0 && time < config->min_time) {
        int n = snprintf(record + res, len - res, "Uptime %.2f%% is below by %.2f%%. ", time, config->min_time);
        if (n > 0 && (size_t) n < len - res) res += (size_t)n;
        ans = 1;
    }

    if (!ans) snprintf(record, len, "Everything is good");

    return ans;
}

