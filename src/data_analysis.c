#include "data_analysis.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// return the size of loaded
size_t dataload(const char *file, monitor_record_t *records, size_t mx) {
    printf("Now we are at the data loading process\n");
    FILE *fp = fopen(file, "r");
    if (!fp) return 0;
    char line[1024];
    size_t ans = 0;
    while (ans < mx && fgets(line, sizeof(line), fp)) {
        char *token = strtok(line, ",\n");
        monitor_record_t ans_record;
        printf("Split the input by comma or newline \n");
        ans_record.timestamp = (time_t)strtol(token, NULL, 10);
        token = strtok(NULL, ",\n");
        strncpy(ans_record.hostname, token, sizeof(ans_record.hostname) - 1);
        token = strtok(NULL, ",\n");
        ans_record.ping = atoi(token);
        token = strtok(NULL, ",\n");
        ans_record.rtt_ms = strtod(token, NULL);
        token = strtok(NULL, ",\n");
        ans_record.cnt = atoi(token);
        for (int i = 0; i < ans_record.cnt; i++) {
            token = strtok(NULL, ",\n");
            int port = 0;
            int status = 0;
            if (sscanf(token, "%d:%d", &port, &status) == 2) {
                ans_record.ports[i] = port;
                ans_record.status[i] = status;
            }
        }
        records[ans] = ans_record;
        ans++;
    }
    fclose(fp);
    return ans;
}

void datareport(monitor_record_t *records, size_t cnt, ping_stats_t *s) {
    s->total_sent = 0;
    s->total_received = 0;
    s->last_rtt = 0.0;
    s->mn_rtt = 1e9;
    s->mx_rtt = 0.0;
    s->sum_rtt = 0.0;
    s->loss_rate = 0.0;
    for (size_t i = 0; i < cnt; i++) {
        monitor_record_t *ans_record = &records[i];
        statsUpdate(s, ans_record->ping, ans_record->rtt_ms);
    }
}

