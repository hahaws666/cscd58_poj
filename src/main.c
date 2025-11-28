#include "monitor.h"
#include "data_analysis.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_LOG_FILE "monitor_records.log"
#define MX 65535
#define SIZE 512


/*
 * Read a given file one by one line and then record each line's info to hostname
 * what is returned is how many hosts are loaded
 */
int read_host_config(const char *file, host_entry_t *hosts, int mx) {
    FILE *fp = fopen(file, "r");
    char line[SIZE];
    int cnt = 0;
    while (cnt < mx && fgets(line, sizeof(line), fp)) {
        host_entry_t *host = &hosts[cnt];
        memset(host, 0, sizeof(host_entry_t));
        char hostname[256];
        char port_str[256];

        // Split by space or tab
        if (sscanf(line, "%255s %255s", hostname, port_str) != 2) {
            // Try to parse without ports (hostname only)
            if (sscanf(line, "%255s", hostname) == 1) {
                strncpy(host->hostname, hostname, sizeof(host->hostname) - 1);
                host->port_cnt = 0;
                cnt++;
            }
            continue;
        }
        printf("debug noe copy to host name\n");
        strncpy(host->hostname, hostname, sizeof(host->hostname) - 1);
        host->hostname[sizeof(host->hostname) - 1] = '\0';
        char *token = strtok(port_str, ",");
        int port_cnt = 0;
        while (token && port_cnt < 32) {
            int port = atoi(token);
            host->ports[port_cnt++] = port;
            token = strtok(NULL, ",");
        }

        host->port_cnt = port_cnt;
        cnt++;
    }
    fclose(fp);
    return cnt;
}



int main() {
    char cmd[100];
    while (1) {
        printf("Network Monitor\n");
        printf("Commands:\n");
        fgets(cmd, sizeof(cmd), stdin);

        // ping case
        if (strcmp(cmd, "ping") == 0) {
            char host[256];
            sscanf(cmd, "ping %s", host);

            double rtt;
            if (icmp_ping(host, &rtt) == 0) printf("RTT = %.2f ms\n", rtt);
            else printf("Ping failed\n");
        } // scan port case
        else if (strcmp(cmd, "scan") == 0) {
            char host[256];
            int port;
            sscanf(cmd, "scan %s %d", host, &port);
            int ans = scan_port(host, port);
        } // monitor case
        else if (strcmp(cmd, "monitor") == 0) {
            // read host config file
            host_entry_t hosts[100];
            int cnt = read_host_config("host_config.txt", hosts, 100);
            int monitor_count = 10;
            char *ptr = cmd + 7;
            // go until we find a white space
            while (*ptr == ' ') ptr++;
            if (*ptr != '\0') {
                monitor_count = atoi(ptr);
            }
            
            pthread_t thread_ids[100];
            int threads_created = start_monitoring(hosts, cnt, monitor_count, DEFAULT_LOG_FILE, thread_ids);
            for (int i = 0; i < cnt; i++) {
                monitor_args_t *ans_args = malloc(sizeof(monitor_args_t));
                args->host = &hosts[i];
                args->cnt = monitor_count;
                args->log_file = DEFAULT_LOG_FILE;
                hosts[i].ping_stats.total_sent = 0.0;
                hosts[i].ping_stats.total_received = 0;
                hosts[i].ping_stats.last_rtt_ms = 0.0;
                hosts[i].ping_stats.min_rtt_ms = 1e9;
                hosts[i].ping_stats.max_rtt_ms = 0.0;
                hosts[i].ping_stats.sum_rtt_ms = 0.0;
                hosts[i].ping_stats.loss_rate = 0.0;
                pthread_create(&thread_ids[i], NULL, monitor_thread, args);
            }

            // 等全部结束。。。
            for (int i = 0; i < threads_created; i++) {
                pthread_join(thread_ids[i], NULL);
            }
            
            printf("All monitoring completed.\n");

        } else if (strcmp(cmd, "report") == 0) {
            char *ptr = cmd + 6;
            while (*ptr == ' ') ptr++;
            const char *log_file = (*ptr != '\n' && *ptr != '\0') ? ptr : DEFAULT_LOG_FILE;
            
            // Remove trailing newline
            char filepath[256];
            strncpy(filepath, log_file, sizeof(filepath) - 1);
            filepath[sizeof(filepath) - 1] = '\0';
            size_t n = strlen(filepath);
            if (n > 0 && filepath[n - 1] == '\n') {
                filepath[n - 1] = '\0';
            }
            
            monitor_record_t records[1000];
            size_t cnt = dataload(filepath, records, 1000);
            
            if (cnt == 0) {
                printf("No records found in %s\n", filepath);
            } else {
                ping_stats_t stats;
                datareport(records, cnt, &stats);
                printf("\n=== Monitoring Report ===\n");
                printf("Total records: %zu\n", cnt);
                statsPrint(&stats);
            }

        } else if (strcmp(cmd, "stats") == 0) {
            char *ptr = cmd + 5;
            while (*ptr == ' ') ptr++;
            const char *log_file = (*ptr != '\n' && *ptr != '\0') ? ptr : DEFAULT_LOG_FILE;
            
            char filepath[256];
            strncpy(filepath, log_file, sizeof(filepath) - 1);
            filepath[sizeof(filepath) - 1] = '\0';
            size_t len = strlen(filepath);
            if (len > 0 && filepath[len - 1] == '\n') {
                filepath[len - 1] = '\0';
            }
            
            monitor_record_t records[1000];
            size_t cnt = dataload(filepath, records, 1000);
            
            if (cnt == 0) {
                printf("No records found in %s\n", filepath);
            } else {
                printf("\n=== Detailed Statistics ===\n");
                printf("Total records: %zu\n\n", cnt);
                
                for (size_t i = 0; i < cnt && i < 10; i++) {
                    struct tm *tm_info = localtime(&records[i].timestamp);
                    char time_str[64];
                    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
                    
                    printf("[%s] %s: RTT=%.2f ms, Status=%s\n", time_str, records[i].hostname, records[i].rtt_ms, records[i].ping_success ? "OK" : "FAIL");
                }
                
                if (cnt > 10) printf("... (%zu more records)\n", cnt - 10);
                
                ping_stats_t stats;
                datareport(records, cnt, &stats);
                printf("\n");
                statsPrint(&stats);
            }

        } else if (strcmp(cmd, "exit") == 0) {
            break;
        }
    }
}
