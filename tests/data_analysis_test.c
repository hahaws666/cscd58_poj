#include <stdio.h>
#include <time.h>
#include "data_analysis.h"

int main(void) {
    const char *filepath = "build/test_records.log";

    monitor_record_t rec = {0};
    rec.timestamp = time(NULL);
    snprintf(rec.hostname, sizeof(rec.hostname), "%s", "example.com");
    rec.ping_success = 1;
    rec.rtt_ms = 12.5;
    rec.port_cnt = 2;
    rec.port_status[0].port = 80;
    rec.port_status[0].status = PORT_OPEN;
    rec.port_status[1].port = 443;
    rec.port_status[1].status = PORT_TIMEOUT;

    if (dataappend(filepath, &rec) != 0) {
        printf("Failed to append record\n");
        return 1;
    }

    monitor_record_t loaded[4];
    size_t cnt = dataload(filepath, loaded, 4);
    printf("Loaded %zu records\n", cnt);

    ping_stats_t stats;
    datareport(loaded, cnt, &stats);
    statsPrint(&stats);

    uptime_tracker_t tracker = {0};
    uptime_tracker_update(&tracker, 1);
    uptime_tracker_update(&tracker, 0);
    uptime_tracker_update(&tracker, 1);
    double uptime = uptime_tracker_percentage(&tracker);
    printf("Uptime: %.2f%%\n", uptime);

    alert_config_t cfg;
    cfg.mx_latency = 10.0;
    cfg.mx_loss = 0.1;
    cfg.mn_time = 95.0;

    char msg[128];
    int triggered = alert_check_trigger(&cfg, &stats, rec.rtt_ms, uptime, msg, sizeof(msg));
    printf("Alert triggered: %s\n", triggered ? "YES" : "NO");
    printf("Message: %s\n", msg);

    return 0;
}

