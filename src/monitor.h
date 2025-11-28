#ifndef MONITOR_H
#define MONITOR_H

#include <stdint.h>
#include <netinet/in.h>
#include <pthread.h>

/*
 * Threshold configuration for alerting.
 */
typedef struct {
    double max_latency_ms;
    double max_loss_rate;
    double min_uptime_pct;
} alert_config_t;

/*
 * Port Scan Status
 */
typedef enum {
    PORT_UNKNOWN = 0,
    PORT_OPEN    = 1,
    PORT_CLOSED  = 2,
    PORT_TIMEOUT = 3
} port_status_t;

/*
 * Ping Stats
 */
typedef struct {
    int total_sent;
    int total_received;
    double last_rtt_ms;
    double min_rtt_ms;
    double max_rtt_ms;
    double sum_rtt_ms;    
    double loss_rate;     
    double jitter_ms;
    double previous_rtt;
} ping_stats_t;

/*
 * Port Stats History
 */
typedef struct {
    int port;
    port_status_t status;
} port_stats_t;

/*
 * Host Entry
 */
typedef struct {
    char hostname[256];
    int ports[32];
    int port_count;

    ping_stats_t ping_stats;
    port_stats_t port_status[32];
} host_entry_t;

/*
 * Monitor Thread Arguments
 */
typedef struct {
    host_entry_t *host;
    int sample_count;
    const char *log_file;
    alert_config_t alert_conf;
} monitor_args_t;


/* ============================
 * Function Declarations
 * ============================ */

int icmp_ping(const char *host, double *rtt_ms);
int scan_port(const char *host, int port);

void stats_init(ping_stats_t *s);
void stats_update_ping(ping_stats_t *s, int success, double rtt);
void stats_print(const ping_stats_t *s);

int start_monitoring(host_entry_t *hosts, int host_count, int sample_count, const char *log_file, pthread_t *thread_ids);

#endif