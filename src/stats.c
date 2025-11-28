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
    s->jitter_ms = 0.0;
    s->previous_rtt = 0.0;
}

void statsUpdate(ping_stats_t *s, int success, double rtt) {
    s->total_sent++;

    if (success) {
        if (s->total_received > 1) {
            double diff = rtt - s->previous_rtt;
            if (diff < 0) diff = -diff;
            s->jitter_ms += (diff - s->jitter_ms) / 16.0;
        }
        
        s->previous_rtt = rtt;
        s->total_received++;
        s->last_rtt_ms = rtt;
        if (rtt < s->min_rtt_ms) s->min_rtt_ms = rtt;
        if (rtt > s->max_rtt_ms) s->max_rtt_ms = rtt;
        s->sum_rtt_ms += rtt;
    }

    s->loss_rate = 1.0 - ((double)s->total_received / (double)s->total_sent);
}

void stats_print(const ping_stats_t *s) {
    printf("\n+----------------------+-----------------+\n");
    printf("| Metric               | Value           |\n");
    printf("+----------------------+-----------------+\n");
    printf("| Total Sent           | %-15d |\n", s->total_sent);
    printf("| Total Received       | %-15d |\n", s->total_received);
    printf("| Loss Rate            | %-14.2f%% |\n", s->loss_rate * 100.0);
    
    if (s->total_received > 0) {
        printf("| Min RTT              | %-12.2f ms |\n", s->min_rtt_ms);
        printf("| Max RTT              | %-12.2f ms |\n", s->max_rtt_ms);
        printf("| Avg RTT              | %-12.2f ms |\n", s->sum_rtt_ms / s->total_received);
        printf("| Jitter               | %-12.2f ms |\n", s->jitter_ms);
    }
    printf("+----------------------+-----------------+\n");
}
