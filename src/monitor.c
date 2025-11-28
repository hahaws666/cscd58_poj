// monitor.c
#include "monitor.h"
#include "data_analysis.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define DEFAULT_LOG_FILE "monitor_records.log"
extern alert_config_t global_alerts;

static void *monitor_thread(void *arg) {
    monitor_args_t *args = (monitor_args_t *)arg;
    host_entry_t *host = args->host;
    int sample_count = args->sample_count;
    const char *log_file = args->log_file ? args->log_file : DEFAULT_LOG_FILE;
    
    uptime_tracker_t uptime = {0};
    uptime.start_time = time(NULL);
    
    printf("Monitoring thread started for %s with %d samples\n", host->hostname, sample_count);
    while (sample_count > 0) {
        double rtt;
        int ping_ok = icmp_ping(host->hostname, &rtt);

        printf("ATTEMPT %d: [PING] %s -> %s, %.2f ms\n",
               sample_count,
               host->hostname,
               ping_ok == 0 ? "OK" : "FAIL",
               ping_ok == 0 ? rtt : -1);
        stats_update_ping(&host->ping_stats, ping_ok == 0, rtt);
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
            printf("ATTEMPT %d: [PORT %d] -> %s\n",
                   sample_count,
                   host->ports[i],
                   (st == PORT_OPEN ? "OPEN" :
                    st == PORT_CLOSED ? "CLOSED" : "TIMEOUT"));
            
            // Store port status in record
            record.port_status[i].port = host->ports[i];
            record.port_status[i].status = (port_status_t)st;
        }

        // Save record to file
        data_store_append(log_file, &record);
        char alert_msg[256];
        double uptime_pct = uptime_tracker_percentage(&uptime);

        if (alert_check_trigger(&args->alert_conf, &host->ping_stats, rtt, uptime_pct, alert_msg, sizeof(alert_msg))) {
            fprintf(stderr, "[ALERT] Host %s: %s\n", host->hostname, alert_msg);
        }
        sleep(2);  // 每 2 秒监控一次
        sample_count--;
    }
    
    double uptime_pct = uptime_tracker_percentage(&uptime);
    printf("Monitoring of %s completed (Uptime: %.2f%%)\n", host->hostname, uptime_pct);
    fflush(stdout);
    free(args);
    return NULL;
}

int start_monitoring(host_entry_t *hosts, int host_count, int sample_count, const char *log_file, pthread_t *thread_ids)  {
    printf("Starting monitoring of %d hosts with %d samples each...\n", host_count, sample_count);
    for (int i = 0; i < host_count; i++) {
        monitor_args_t *args = (monitor_args_t *)malloc(sizeof(monitor_args_t));
        if (!args) continue; // Safety check
        
        args->host = &hosts[i];
        args->sample_count = sample_count;
        args->log_file = log_file;
        
        // COPY THE GLOBAL ALERT CONFIG
        args->alert_conf = global_alerts;

        stats_init(&hosts[i].ping_stats);
        pthread_create(&thread_ids[i], NULL, monitor_thread, (void *)args);
    }
    return host_count;
}

