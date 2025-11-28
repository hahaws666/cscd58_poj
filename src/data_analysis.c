#include "data_analysis.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// return -1 if failed 0 if success
int dataappend(const char *path, const monitor_record_t *record) {
    // path and record are required
    if (!path || !record) return -1;

    FILE *fp = fopen(path, "a");
    // make sure file exsists
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
size_t dataload(const char *path, monitor_record_t *records, size_t mx) {
    // some previous check as well
    if (!path || !records || mx == 0) return 0;

    FILE *fp = fopen(path, "r");
    if (!fp) return 0;

    char line[1024];
    size_t ans = 0;
    while (ans < mx && fgets(line, sizeof(line), fp)) {
        char *token = strtok(line, ",\n");
        if (!token) continue;

        monitor_record_t rec = {0};
        rec.timestamp = (time_t)strtol(token, NULL, 10);

        token = strtok(NULL, ",\n");
        if (!token) continue;
        strncpy(rec.hostname, token, sizeof(rec.hostname) - 1);

        token = strtok(NULL, ",\n");
        if (!token) continue;
        rec.ping = atoi(token);

        token = strtok(NULL, ",\n");
        if (!token) continue;
        rec.rtt_ms = strtod(token, NULL);

        token = strtok(NULL, ",\n");
        if (!token) continue;
        rec.cnt = atoi(token);
        if (rec.cnt < 0) rec.cnt = 0;
        if (rec.cnt > 32) rec.cnt = 32;

        for (int i = 0; i < rec.cnt; i++) {
            token = strtok(NULL, ",\n");
            // if null
            if (!token) break;
            int port = 0;
            int status = 0;
            if (sscanf(token, "%d:%d", &port, &status) == 2) {
                rec.port_status[i].port = port;
                // 更新状态
                // status updated
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

void datareport(const monitor_record_t *records, size_t count, ping_stats_t *out_stats) {
    if (!out_stats) return;

    statsInit(out_stats);
    if (!records) return;

    for (size_t i = 0; i < count; i++) {
        const monitor_record_t *rec = &records[i];
        statsUpdate(out_stats, rec->ping, rec->rtt_ms);
    }
}

void uptime_tracker_update(uptime_tracker_t *tracker, int up) {
    if (!tracker) return;

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
    if (!tracker) return 0.0;
    uint64_t total = tracker->up + tracker->down;
    if (total == 0) return 0.0;
    return (double)tracker->up * 100.0 / (double)total;
}

int alert_check_trigger(const alert_config_t *config, const ping_stats_t *stats, double late, double time, char *record, size_t len) {
    if (!config || !record || len == 0) return 0;

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

