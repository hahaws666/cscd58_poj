#ifndef MONITOR_H
#define MONITOR_H
#include <stdint.h>
#include <netinet/in.h>
#include <pthread.h>
// sorry for the kind of meessy struct below...
// 主机配置and监控数据
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
    double loss_rate; //丢包率
} host_entry_t;
typedef struct {
    host_entry_t *host;
    int cnt;
    const char *log_file; // Optional, log file path
} monitor_args_t;
// 成功的话会返回0，这个是一个ICMP ping
int icmp_ping(const char *host, double *rtt_ms);
// PORT扫描
int scan_port(const char *host, int port);
//multithread监控
void *monitor_thread(void *arg);
#endif
