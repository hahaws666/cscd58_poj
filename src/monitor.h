#ifndef MONITOR_H
#define MONITOR_H
#include <stdint.h>
#include <netinet/in.h>
#include <pthread.h>

#define ALERT_RTT_THRESHOLD 200.0 
#define ALERT_LOSS_THRESHOLD 0.20

#define COLOR_RED     "\x1b[31m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_RESET   "\x1b[0m"
#define COLOR_GREEN   "\x1b[32m"

// sorry for the kind of meessy struct below...
typedef struct {
    char hostname[256];
    int ports[32];
    int port_count;
    int total_sent;
    int total_received;
    double last_rtt;
    double mn_rtt;
    double mx_rtt;
    double sum_rtt;
    double last_jitter; 
    double sum_jitter;
    double mx_jitter;
    double loss_rate; //丢包率
} host_entry_t;
typedef struct {
    host_entry_t *host;
    int cnt;
    const char *log_file; // It is designed to be optional, log file path
} monitor_args_t;
int icmp_ping(const char *host, double *rtt_ms);
// PORT扫描
int scan_port(const char *host, int port);
//multithread监控
void *monitor_thread(void *arg);
#endif
