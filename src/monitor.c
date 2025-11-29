#include "monitor.h"
#include "data_analysis.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// thanks to C69 for doing the multi-thread stuffs
void *monitor_thread(void *arg) {
    printf("111111 Now we are monitoring\n");
    monitor_args_t *args = (monitor_args_t *)arg;
    host_entry_t *host = args->host;
    int cnt = args->cnt;
    const char *thefile = args->log_file ? args->log_file : "monitor_records.log";
    
    uptime_tracker_t uptime;
    uptime.start = time(NULL);
    printf("Now we are at the thread of monotor, it is started for %s with %d samples\n", host->hostname, cnt);
    printf("We have a loop of it\n");
    while (cnt > 0) {
        double rtt;
        // int ans_ping = icmp_ping(host->hostname, &rtt);
        int ans = (icmp_ping(host->hostname, &rtt) == 0);
        double ans2;
        if (ans) ans2 = rtt;
        else {
            ans2 = -1.0;
        }
        printf("2222222222222222222\n");
        printf("ATTEMPT %d: PING %s -> %s, %.2f ms\n", cnt, host->hostname, ans ? "OK" : "FAIL", ans2);
        // int ans = (icmp_ping(host->hostname, &rtt) == 0);
        statsUpdate(&host->ping_stats, ans, rtt);
        uptime_tracker_update(&uptime, ans);
        // Now we need to record it
        monitor_record_t record;
        record.timestamp = time(NULL);
        strcpy(record.hostname, host->hostname);
        record.rtt_ms = ans ? rtt : 0.0;
        if (ans) {
            record.ping_success = 1;
        }
        else record.ping_success = 0;
        record.port_count = host->port_count;
        for (int i = 0; i < host->port_count; i++) {
            char ans_status[10];
            int st = scan_port(host->hostname, host->ports[i]);
            if (st == 1) strcpy(ans_status, "OPEN\0");
            else if (st == 2) strcpy(ans_status, "CLOSED\0");
            else strcpy(ans_status, "TIMEOUT\0");
            record.port_status[i].port = host->ports[i];
            record.port_status[i].status = st;
            printf("PORT %d -> %s\n", host->ports[i], ans_status);
        }
        printf("Here we go at the data append process...\n")
        File *fp = fopen(thefile, "a");
        fprintf(fp, "%ld, %s, %d, %.2f, %d", (long)record->timestamp, record->hostname, record->ping, record->rtt_ms, record->cnt);
        for (int i = 0; i < record->cnt; i++) {
            fprintf(fp, ",%d: %d", record->port_status[i].port, (int)record->port_status[i].status);
        }
        fprintf(fp, "\n");
        // close the file
        fclose(fp);
        // maybe we will need to change this later for different values
        sleep(2);  // 每 2 秒监控一次
        cnt--;
    }
    
    double uptime_pct = uptime_tracker_percentage(&uptime);
    printf("End of a monitor, the result is: %.2f%%", uptime_pct);
    fflush(stdout);
    free(args);
    return NULL;
}

