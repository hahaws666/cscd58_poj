// monitor.c
#include "monitor.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void *monitor_thread(void *arg) {
    monitor_args_t *args = (monitor_args_t *)arg;
    host_entry_t *host = args->host;
    int sample_count = args->sample_count;
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

        for (int i = 0; i < host->port_count; i++) {
            int st = scan_port(host->hostname, host->ports[i]);
            printf("ATTEMPT %d: [PORT %d] -> %s\n",
                   sample_count,
                   host->ports[i],
                   (st == PORT_OPEN ? "OPEN" :
                    st == PORT_CLOSED ? "CLOSED" : "TIMEOUT"));
        }

        sleep(2);  // 每 2 秒监控一次
        sample_count--;
    }
    printf("Monitoring of %s completed\n> ", host->hostname);
    fflush(stdout);
    free(args);
    return NULL;
}

void start_monitoring(host_entry_t *hosts, int host_count,int sample_count)  {
    printf("Starting monitoring of %s with %d samples\n", hosts[0].hostname, sample_count);
    for (int i = 0; i < host_count; i++) {
        monitor_args_t *args = (monitor_args_t *)malloc(sizeof(monitor_args_t));
        args->host = &hosts[i];
        args->sample_count = sample_count;
        stats_init(&hosts[i].ping_stats);
        pthread_t tid;
        pthread_create(&tid, NULL, monitor_thread, (void *)args);
        pthread_detach(tid);
    }
}

