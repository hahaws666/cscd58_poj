// monitor.c
#include "monitor.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void *monitor_thread(void *arg) {
    host_entry_t *host = arg;

    while (1) {
        double rtt;
        int ping_ok = icmp_ping(host->hostname, &rtt);

        printf("[PING] %s -> %s, %.2f ms\n",
               host->hostname,
               ping_ok == 0 ? "OK" : "FAIL",
               ping_ok == 0 ? rtt : -1);
        stats_update_ping(&host->ping_stats, ping_ok == 0, rtt);

        for (int i = 0; i < host->port_count; i++) {
            int st = scan_port(host->hostname, host->ports[i]);
            printf("  PORT %d -> %s\n",
                   host->ports[i],
                   (st == PORT_OPEN ? "OPEN" :
                    st == PORT_CLOSED ? "CLOSED" : "TIMEOUT"));
        }

        sleep(2);  // 每 2 秒监控一次
    }
    return NULL;
}

void start_monitoring(host_entry_t *hosts, int count) {
    for (int i = 0; i < count; i++) {
        stats_init(&hosts[i].ping_stats);
        pthread_t tid;
        pthread_create(&tid, NULL, monitor_thread, &hosts[i]);
        pthread_detach(tid);
    }
}

