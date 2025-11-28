// monitor.c
#include "monitor.h"
#include "data_analysis.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define DEFAULT_LOG_FILE "records.log"
#define SIZE_MONITOR_ARGS sizeof(monitor_args_t)

static void *monitor_thread(void *arg) {
    printf("111111 Now we are monitoring\n");
    monitor_args_t *args = (monitor_args_t *)arg;
    host_entry_t *host = args->host;
    int cnt = args->cnt;
    const char *thefile = args->log_file ? args->log_file : DEFAULT_LOG_FILE;
    
    uptime_tracker_t uptime = {0};
    uptime.start = time(NULL);
    
    printf("Monitoring thread started for %s with %d samples\n", host->hostname, cnt);
    while (cnt) {
        double rtt;
        int ping_ok = icmp_ping(host->hostname, &rtt);

        printf("ATTEMPT %d: [PING] %s -> %s, %.2f ms\n", cnt, host->hostname, ping_ok == 0 ? "OK" : "FAIL", ping_ok == 0 ? rtt : -1);
        statsUpdate(&host->ping_stats, ping_ok == 0, rtt);
        uptime_tracker_update(&uptime, ping_ok == 0);

        // Create monitoring record
        monitor_record_t record = {0};
        record.timestamp = time(NULL);
        strncpy(record.hostname, host->hostname, sizeof(record.hostname) - 1);
        record.rtt_ms = ping_ok == 0 ? rtt : 0.0;
        record.ping_success = ping_ok == 0;
        record.port_count = host->port_count;

        for (int i = 0; i < host->port_count; i++) {
            int st = scan_port(host->hostname, host->ports[i]);
            printf("ATTEMPT %d: [PORT %d] -> %s\n", cnt, host->ports[i], (st == PORT_OPEN ? "OPEN" : st == PORT_CLOSED ? "CLOSED" : "TIMEOUT"));
            
            record.port_status[i].port = host->ports[i];
            record.port_status[i].status = (port_status_t)st;
        }

        dataappend(thefile, &record);

        sleep(2);  // 每 2 秒监控一次
        cnt--;
    }
    
    double uptime_pct = uptime_tracker_percentage(&uptime);
    printf("Monitoring of %s completed (Uptime: %.2f%%)\n", host->hostname, uptime_pct);
    fflush(stdout);
    free(args);
    return NULL;
}

int start_monitoring(host_entry_t *hosts, int host_cnt, int cnt, const char *log_file, pthread_t *thread_ids)  {
    printf("Now we are doing monitoring of %d hosts with %d samples\n", host_cnt, cnt);
    for (int i = 0; i < host_cnt; i++) {
        monitor_args_t *args = (monitor_args_t *)malloc(SIZE_MONITOR_ARGS);
        args->host = &hosts[i];
        args->cnt = cnt;
        args->log_file = log_file;
        statsInit(&hosts[i].ping_stats);
        pthread_create(&thread_ids[i], NULL, monitor_thread, (void *)args);
    }
    return host_cnt;
}

