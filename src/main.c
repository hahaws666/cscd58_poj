#include "monitor.h"
#include "data_analysis.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_LOG_FILE "monitor_records.log"


/*
 * Read host configuration from file
 * Format: each line is "hostname port1,port2,port3"
 * Returns number of hosts successfully loaded
 */
int read_host_config(const char *filename, host_entry_t *hosts, int max_hosts) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Failed to open host config file: %s\n", filename);
        return 0;
    }

    char line[512];
    int host_count = 0;

    while (host_count < max_hosts && fgets(line, sizeof(line), file)) {
        // Skip empty lines and whitespace-only lines
        int len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[--len] = '\0';
        }

        // Skip empty lines
        int i = 0;
        while (line[i] == ' ' || line[i] == '\t') i++;
        if (line[i] == '\0') continue;

        host_entry_t *host = &hosts[host_count];
        memset(host, 0, sizeof(host_entry_t));

        // Parse hostname and ports
        char hostname[256];
        char port_str[256];

        // Split by space or tab
        if (sscanf(line, "%255s %255s", hostname, port_str) != 2) {
            // Try to parse without ports (hostname only)
            if (sscanf(line, "%255s", hostname) == 1) {
                strncpy(host->hostname, hostname, sizeof(host->hostname) - 1);
                host->port_count = 0;
                host_count++;
            }
            continue;
        }

        // Copy hostname
        strncpy(host->hostname, hostname, sizeof(host->hostname) - 1);
        host->hostname[sizeof(host->hostname) - 1] = '\0';

        // Parse comma-separated ports
        char *token = strtok(port_str, ",");
        int port_count = 0;

        while (token != NULL && port_count < 32) {
            // Remove leading/trailing whitespace
            while (*token == ' ' || *token == '\t') token++;
            char *end = token + strlen(token) - 1;
            while (end > token && (*end == ' ' || *end == '\t')) {
                *end = '\0';
                end--;
            }

            int port = atoi(token);
            if (port > 0 && port <= 65535) {
                host->ports[port_count++] = port;
            }
            token = strtok(NULL, ",");
        }

        host->port_count = port_count;
        host_count++;
    }

    fclose(file);
    return host_count;
}



int main() {


    char cmd[256];

    while (1) {
        printf("Network Monitor CLI\n");
        printf("Commands:\n");
        printf("  ping <host>\n");
        printf("  scan <host> <port>\n");
        printf("  monitor [count]          - Start monitoring (default: 10 samples)\n");
        printf("  report [file]            - Generate statistics report from log file\n");
        printf("  stats [file]             - Show detailed statistics\n");
        printf("  exit\n");
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

            // read host config file
            host_entry_t hosts[100];
            int host_count = read_host_config("host_config.txt", hosts, 100);
            if (host_count == 0) {
                printf("No hosts found in host config file\n");
                continue;
            }

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
            
            pthread_t thread_ids[100];
            int threads_created = start_monitoring(hosts, host_count, monitor_count, DEFAULT_LOG_FILE, thread_ids);
            
            // Wait for all monitoring threads to complete
            for (int i = 0; i < threads_created; i++) {
                pthread_join(thread_ids[i], NULL);
            }
            
            printf("All monitoring completed.\n");

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
