#include "monitor.h"
#include "data_analysis.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_LOG_FILE "monitor_records.log"

int main() {
    printf("Network Monitor CLI\n");
    printf("Commands:\n");
    printf("  ping <host>\n");
    printf("  scan <host> <port>\n");
    printf("  monitor [count]          - Start monitoring (default: 10 samples)\n");
    printf("  report [file]            - Generate statistics report from log file\n");
    printf("  stats [file]             - Show detailed statistics\n");
    printf("  exit\n");

    char cmd[256];

    while (1) {
        printf("\n> ");
        fgets(cmd, sizeof(cmd), stdin);

        if (strncmp(cmd, "ping", 4) == 0) {
            char host[256];
            sscanf(cmd, "ping %s", host);

            double rtt;
            if (icmp_ping(host, &rtt) == 0)
                printf("RTT = %.2f ms\n", rtt);
            else
                printf("Ping failed\n");

        } else if (strncmp(cmd, "scan", 4) == 0) {
            char host[256];
            int port;
            sscanf(cmd, "scan %s %d", host, &port);

            int st = scan_port(host, port);
            printf("Port %d is %d\n", port, st);

        } else if (strncmp(cmd, "monitor", 7) == 0) {
            printf("Starting multi-thread monitoring...\n");
            int monitor_count = 10;
            char *ptr = cmd + 7;
            while (*ptr == ' ') ptr++;
            if (*ptr != '\0') {
                monitor_count = atoi(ptr);
                if (monitor_count <= 0) {
                    printf("Invalid monitor count, using default 10\n");
                    monitor_count = 10;
                }
            }
            host_entry_t hosts[1] = {
                {.hostname = "8.8.8.8", .ports = {80, 443}, .port_count = 2}
            };
            start_monitoring(hosts, 1, monitor_count, DEFAULT_LOG_FILE);

        } else if (strncmp(cmd, "report", 6) == 0) {
            char *ptr = cmd + 6;
            while (*ptr == ' ') ptr++;
            const char *log_file = (*ptr != '\n' && *ptr != '\0') ? ptr : DEFAULT_LOG_FILE;
            
            // Remove trailing newline
            char filepath[256];
            strncpy(filepath, log_file, sizeof(filepath) - 1);
            filepath[sizeof(filepath) - 1] = '\0';
            size_t len = strlen(filepath);
            if (len > 0 && filepath[len - 1] == '\n') {
                filepath[len - 1] = '\0';
            }
            
            monitor_record_t records[1000];
            size_t count = data_store_load(filepath, records, 1000);
            
            if (count == 0) {
                printf("No records found in %s\n", filepath);
            } else {
                ping_stats_t stats;
                data_generate_report(records, count, &stats);
                printf("\n=== Monitoring Report ===\n");
                printf("Total records: %zu\n", count);
                stats_print(&stats);
            }

        } else if (strncmp(cmd, "stats", 5) == 0) {
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
            size_t count = data_store_load(filepath, records, 1000);
            
            if (count == 0) {
                printf("No records found in %s\n", filepath);
            } else {
                printf("\n=== Detailed Statistics ===\n");
                printf("Total records: %zu\n\n", count);
                
                for (size_t i = 0; i < count && i < 10; i++) {
                    struct tm *tm_info = localtime(&records[i].timestamp);
                    char time_str[64];
                    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
                    
                    printf("[%s] %s: RTT=%.2f ms, Status=%s\n",
                           time_str,
                           records[i].hostname,
                           records[i].rtt_ms,
                           records[i].ping_success ? "OK" : "FAIL");
                }
                
                if (count > 10) {
                    printf("... (%zu more records)\n", count - 10);
                }
                
                ping_stats_t stats;
                data_generate_report(records, count, &stats);
                printf("\n");
                stats_print(&stats);
            }

        } else if (strncmp(cmd, "exit", 4) == 0) {
            break;
        }
    }
}
