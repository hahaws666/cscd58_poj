#include "monitor.h"
#include "data_analysis.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 

#define DEFAULT_LOG_FILE "monitor_records.log"

// Global alert configuration (Default values)
alert_config_t global_alerts = {
    .max_latency_ms = 100.0,
    .max_loss_rate = 0.05,
    .min_uptime_pct = 95.0
};

/*
 * Read host configuration from file
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
        // cleanup line endings
        int len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[--len] = '\0';
        }

        // Skip empty lines or spaces
        int i = 0;
        while (line[i] == ' ' || line[i] == '\t') i++;
        if (line[i] == '\0') continue;
        
        // Skip comments
        if (line[i] == '#') continue;

        host_entry_t *host = &hosts[host_count];
        memset(host, 0, sizeof(host_entry_t));

        char hostname[256];
        char port_str[256];

        // Parse: hostname ports...
        if (sscanf(line, "%255s %255s", hostname, port_str) != 2) {
            if (sscanf(line, "%255s", hostname) == 1) {
                strncpy(host->hostname, hostname, sizeof(host->hostname) - 1);
                host->port_count = 0;
                host_count++;
            }
            continue;
        }

        strncpy(host->hostname, hostname, sizeof(host->hostname) - 1);
        
        char *token = strtok(port_str, ",");
        int port_count = 0;

        while (token != NULL && port_count < 32) {
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
        printf("Network Monitor CLI (type 'help' for commands)\n> ");
        if (!fgets(cmd, sizeof(cmd), stdin)) break;

        if (strncmp(cmd, "ping", 4) == 0) {
            char host[256];
            if (sscanf(cmd, "ping %s", host) == 1) {
                double rtt;
                if (icmp_ping(host, &rtt) == 0)
                    printf("RTT = %.2f ms\n", rtt);
                else
                    printf("Ping failed\n");
            }

        } else if (strncmp(cmd, "scan", 4) == 0) {
            char host[256];
            int port;
            if (sscanf(cmd, "scan %s %d", host, &port) == 2) {
                int st = scan_port(host, port);
                printf("Port %d is %s\n", port, 
                    st == PORT_OPEN ? "OPEN" : 
                    st == PORT_CLOSED ? "CLOSED" : "TIMEOUT");
            }

        } else if (strncmp(cmd, "monitor", 7) == 0) {
            host_entry_t hosts[100];
            int host_count = read_host_config("host_config.txt", hosts, 100);
            
            if (host_count == 0) {
                printf("No valid hosts found in host_config.txt\n");
                continue;
            }

            int monitor_count = 10;
            char *ptr = cmd + 7;
            while (*ptr == ' ') ptr++;
            if (*ptr >= '0' && *ptr <= '9') {
                monitor_count = atoi(ptr);
            }
            if (monitor_count <= 0) monitor_count = 10;
            
            // Start monitoring threads
            // The global_alerts are copied to thread args inside start_monitoring (in src/monitor.c)
            pthread_t thread_ids[100];
            int threads_created = start_monitoring(hosts, host_count, monitor_count, DEFAULT_LOG_FILE, thread_ids);
            
            for (int i = 0; i < threads_created; i++) {
                pthread_join(thread_ids[i], NULL);
            }
            printf("All monitoring completed.\n");

        } else if (strncmp(cmd, "report", 6) == 0) {
            char *ptr = cmd + 6;
            while (*ptr == ' ') ptr++;
            const char *log_file = (*ptr != '\n' && *ptr != '\0') ? ptr : DEFAULT_LOG_FILE;
            
            // Clean newline
            char filepath[256];
            strncpy(filepath, log_file, sizeof(filepath)-1);
            filepath[sizeof(filepath)-1] = '\0';
            size_t len = strlen(filepath);
            if(len>0 && filepath[len-1]=='\n') filepath[len-1]='\0';

            monitor_record_t records[1000];
            size_t count = data_store_load(filepath, records, 1000);
            
            if (count > 0) {
                ping_stats_t stats;
                data_generate_report(records, count, &stats);
                printf("\n=== Monitoring Report (%s) ===\n", filepath);
                stats_print(&stats);
            } else {
                printf("No records found in %s\n", filepath);
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
                
                // Show last 10 records
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
            
        } else if (strncmp(cmd, "help", 4) == 0) {
            printf("\n=== Network Monitor Help ===\n");
            printf("  ping <host>          : Send ICMP echo\n");
            printf("  scan <host> <port>   : Scan TCP port\n");
            printf("  monitor [count]      : Monitor hosts from config (default 10 samples)\n");
            printf("  report [file]        : View summary stats\n");
            printf("  stats [file]         : View detailed logs + summary\n");
            printf("  exit                 : Quit\n");
            printf("============================\n");

        } else if (strncmp(cmd, "exit", 4) == 0) {
            break;
        }
    }
    return 0;
}