#ifndef MONITOR_H
#define MONITOR_H
#include <stdint.h>
#include <netinet/in.h>
#include <pthread.h>
// sorry for the kind of meessy struct below...
// Ping统计信息，对应的包括RTT、丢包等
typedef struct {
    int total_sent;
    int total_received;
    double last_rtt;
    double mn_rtt;
    double mx_rtt;
    double sum_rtt; //总和，主要用来计算avg
    double loss_rate; //丢包率
} ping_stats_t;
// 主机配置and监控数据
typedef struct {
    char hostname[256];
    int ports[32];
    int port_count;
    ping_stats_t ping_stats;
} host_entry_t;
typedef struct {
    host_entry_t *host; // 第 i 个 host 的地址
    int cnt; // 每个 host 的样本数
    const char *log_file; // Optional log file path for data storage
} monitor_args_t;
// 成功的话会返回0，这个是一个ICMP ping
int icmp_ping(const char *host, double *rtt_ms);
// PORT扫描
int scan_port(const char *host, int port);
// 和数据有关的功能实现！
void statsUpdate(ping_stats_t *s, int success, double rtt);
void statsPrint(const ping_stats_t *s);
//multithread监控
void *monitor_thread(void *arg);
#endif
