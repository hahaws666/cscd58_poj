#include "monitor.h"
#include "data_analysis.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// thanks to C69 for doing the multi-thread stuffs
// and sorry for the messy debug info printed....
void *monitor_thread(void *arg) {
    printf("111111 Now we are monitoring\n");
    monitor_args_t *ans_args = (monitor_args_t *)arg;
    host_entry_t *host = ans_args->host;
    int cnt = ans_args->cnt;
    const char *thefile = ans_args->log_file ? ans_args->log_file : "monitor_records.log";
    int tot = 0;
    int up = 0;
    printf("Now we are at the thread of monotor, it is started for %s with %d samples\n", host->hostname, cnt);
    printf("We have a loop of it\n");
    while (cnt > 0) {
        double rtt;
        // int ans_ping = icmp_ping(host->hostname, &rtt);
        int ans = (icmp_ping(host->hostname, &rtt) == 0);
        tot++;
        if (ans) up++;
        double ans2;
        if (ans) ans2 = rtt;
        else {
            ans2 = -1.0;
        }
        printf("2222222222222222222 Now it comes with the attempt coming1\n");
        printf("ATTEMPT %d: PING %s -> %s, %.2f ms\n", cnt, host->hostname, ans ? "OK" : "FAIL", ans2);
        // int ans = (icmp_ping(host->hostname, &rtt) == 0);
        host->total_sent++;
        if (ans) {
            host->total_received++;
            host->last_rtt = rtt;
            if (rtt < host->mn_rtt) host->mn_rtt = rtt;
            if (rtt > host->mx_rtt) host->mx_rtt = rtt;
            host->sum_rtt += rtt;
        }
        host->loss_rate = 1.0 - (double)host->total_received / (double)host->total_sent;
        // Now we need to record it
        monitor_record_t record;
        record.timestamp = time(NULL);
        strcpy(record.hostname, host->hostname);
        record.rtt_ms = ans ? rtt : 0.0;
        if (ans) {
            record.ping = 1;
        }
        else record.ping = 0;
        record.cnt = host->port_count;
        for (int i = 0; i < host->port_count; i++) {
            char ans_status[10];
            int ans_scan = scan_port(host->hostname, host->ports[i]);
            if (ans_scan == 1) strcpy(ans_status, "OPEN\0");
            else if (ans_scan == 2) strcpy(ans_status, "CLOSED\0");
            else strcpy(ans_status, "TIMEOUT\0");
            record.ports[i] = host->ports[i];
            record.status[i] = ans_scan;
            printf("PORT %d with a status of: %s\n", host->ports[i], ans_status);
        }
        printf("3333 Here we go at the data append process...\n");
        FILE *fp = fopen(thefile, "a");
        fprintf(fp, "%ld,%s,%d,%.2f,%d", (long) record.timestamp, record.hostname, record.ping, record.rtt_ms, record.cnt);
        for (int i = 0; i < record.cnt; i++) fprintf(fp, ",%d:%d", record.ports[i], record.status[i]);
        fprintf(fp, "\n");
        // close the file
        fclose(fp);
        // maybe we will need to change this later for different values
        sleep(2);  // 每 2 秒监控一次
        cnt--;
    }
    double ans_uptime = (double)up * 100.0 / (double)tot;
    printf("End of a monitor, the result is: %.2f%%\n", ans_uptime);
    fflush(stdout);
    free(ans_args);
    return NULL;
}

