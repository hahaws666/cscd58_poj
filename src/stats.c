#include "monitor.h"
#include <stdio.h>

void stats_init(ping_stats_t *s) {
    s->total_sent = 0;
    s->total_received = 0;
    s->last_rtt_ms = 0.0;
    s->min_rtt_ms = 1e9;
    s->max_rtt_ms = 0.0;
    s->sum_rtt_ms = 0.0;
    s->loss_rate = 0.0;
}

void stats_update_ping(ping_stats_t *s, int success, double rtt) {
    s->total_sent++;

    if (success) {
        s->total_received++;
        s->last_rtt_ms = rtt;

        // 更新 min/max
        if (rtt < s->min_rtt_ms) s->min_rtt_ms = rtt;
        if (rtt > s->max_rtt_ms) s->max_rtt_ms = rtt;

        // 用于平均值
        s->sum_rtt_ms += rtt;
    }

    // 更新丢包率
    s->loss_rate =
        1.0 - ((double)s->total_received / (double)s->total_sent);
}

void stats_print(const ping_stats_t *s) {
    printf("---- Ping Stats ----\n");
    printf("Total sent:      %d\n", s->total_sent);
    printf("Total received:  %d\n", s->total_received);
    printf("Loss rate:       %.2f%%\n", s->loss_rate * 100);

    if (s->total_received > 0) {
        double avg = s->sum_rtt_ms / (double)s->total_received;
        printf("Last RTT:        %.2f ms\n", s->last_rtt_ms);
        printf("Min RTT:         %.2f ms\n", s->min_rtt_ms);
        printf("Max RTT:         %.2f ms\n", s->max_rtt_ms);
        printf("Avg RTT:         %.2f ms\n", avg);
    }
    printf("---------------------\n");
}
