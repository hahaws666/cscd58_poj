#include "monitor.h"
#include "data_analysis.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

// Global lock for clean terminal output
static pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;

// thanks to C69 for doing the multi-thread stuffs
// and sorry for the messy debug info printed....
void *monitor_thread(void *arg) {
    // printf("111111 Now we are monitoring\n");
    monitor_args_t *ans_args = (monitor_args_t *)arg;
    host_entry_t *host = ans_args->host;
    int cnt = ans_args->cnt;
    int tot = 0;
    int up = 0;
    // printf("Now we are at the thread of monotor, it is started for %s with %d samples\n", host->hostname, cnt);
    // printf("We have a loop of it\n");
    while (cnt > 0) {
        double rtt = 0.0;
        // int ans_ping = icmp_ping(host->hostname, &rtt);
        int ans = (icmp_ping(host->hostname, &rtt) == 0);
        
        tot++;
        if (ans) up++;

        // Update stats
        host->total_sent++;
        if (ans) {
            double current_jitter = 0.0;
            if (host->total_received > 0) {
                current_jitter = fabs(rtt - host->last_rtt);
                
                // Update Jitter Stats
                host->last_jitter = current_jitter;
                host->sum_jitter += current_jitter;
                if (current_jitter > host->mx_jitter) host->mx_jitter = current_jitter;
            }
            host->total_received++;
            host->last_rtt = rtt;
            if (rtt < host->mn_rtt) host->mn_rtt = rtt;
            if (rtt > host->mx_rtt) host->mx_rtt = rtt;
            host->sum_rtt += rtt;
        }
        host->loss_rate = 1.0 - (double)host->total_received / (double)host->total_sent;
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
            record.ports[i] = host->ports[i];
            record.status[i] = scan_port(host->hostname, host->ports[i]);
        }
        pthread_mutex_lock(&print_lock);

        printf("---------------------------------------------------\n");
        printf("REPORT: %s\n", host->hostname);

        double ans2;
        if (ans) {
            ans2 = rtt;
            if (rtt > ALERT_RTT_THRESHOLD) {
                printf("%s[WARNING] High Latency detected on %s: %.2f ms (Threshold: %.0f ms)%s\n", 
                        COLOR_YELLOW, host->hostname, rtt, ALERT_RTT_THRESHOLD, COLOR_RESET);
            } else {
                printf("PING Status: SUCCESS (%.2f ms) | Jitter: %.2f ms\n", rtt, host->last_jitter);
            }
        }
        else {
            // printf("4444 it means failure ans2 = -1.0")
            printf("%s[CRITICAL ALERT] Host %s is DOWN!%s\n", 
                    COLOR_RED, host->hostname, COLOR_RESET);
            printf("PING Status: FAIL\n");
        }
        
        // printf("2222222222222222222 Now it comes with the attempt coming1\n");
        // printf("PING %s with status of: %s, %.2f ms\n", host->hostname, ans ? "SUCCESS!" : "FAIL!", ans ? ans2 : 0.0);
        // int ans = (icmp_ping(host->hostname, &rtt) == 0);
        if (host->loss_rate > ALERT_LOSS_THRESHOLD) {
             printf("%s[WARNING] High Packet Loss on %s: %.2f%% (Threshold: %.0f%%)%s\n", 
                    COLOR_YELLOW, host->hostname, host->loss_rate * 100.0, ALERT_LOSS_THRESHOLD * 100.0, COLOR_RESET);
        }

        for (int i = 0; i < record.cnt; i++) {
            char ans_status[10];
            int ans_scan = record.status[i];
            
            if (ans_scan == 1) strcpy(ans_status, "OPEN\0");
            else if (ans_scan == 2) strcpy(ans_status, "CLOSED\0");
            else strcpy(ans_status, "TIMEOUT\0");
            
            printf("  - Port %d: %s\n", record.ports[i], ans_status);
        }
        printf("---------------------------------------------------\n");
        
        pthread_mutex_unlock(&print_lock);

        // printf("3333 Here we go at the data append process...\n");
        FILE *fp = fopen(ans_args->log_file, "a");
        fprintf(fp, "%ld,%s,%d,%.2f,%d", (long)record.timestamp, record.hostname, record.ping, record.rtt_ms, record.cnt);
        for (int i = 0; i < record.cnt; i++) fprintf(fp, ",%d: %d", record.ports[i], record.status[i]);
        fprintf(fp, "\n");
        // close the file
        fclose(fp);
        sleep(2);  //每2秒监控一次
        cnt--;
    }
    // double ans_uptime = (double)up * 100.0 / (double)tot;
    // printf("End of a monitor, the result is: %.2f\n", ans_uptime);
    fflush(stdout);
    free(ans_args);
    return NULL;
}