#include "monitor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    printf("Network Monitor CLI\n");
    printf("Commands:\n");
    printf("  ping <host>\n");
    printf("  scan <host> <port>\n");
    printf("  monitor\n");
    printf("  exit\n");

    char cmd[256];

    while (1) {
        printf("> ");
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
            host_entry_t hosts[1] = {
                {.hostname = "8.8.8.8", .ports = {80, 443}, .port_count = 2}
            };
            start_monitoring(hosts, 1);

        } else if (strncmp(cmd, "exit", 4) == 0) {
            break;
        }
    }
}
