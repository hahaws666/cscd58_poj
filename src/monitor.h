#ifndef MONITOR_H
#define MONITOR_H

#include <stdint.h>
#include <netinet/in.h>

/*
 * 端口扫描状态
 */
typedef enum {
    PORT_UNKNOWN = 0,
    PORT_OPEN    = 1,
    PORT_CLOSED  = 2,
    PORT_TIMEOUT = 3
} port_status_t;

/*
 * Ping 统计信息（对应 RTT、丢包等）
 */
typedef struct {
    int total_sent;
    int total_received;
    double last_rtt_ms;
    double min_rtt_ms;
    double max_rtt_ms;
    double sum_rtt_ms;    // 用来计算 avg
    double loss_rate;     // 丢包率
} ping_stats_t;

/*
 * 每个端口的历史状态
 */
typedef struct {
    int port;
    port_status_t status;
} port_stats_t;

/*
 * 主机配置 + 监控数据
 */
typedef struct {
    char hostname[256];
    int ports[32];
    int port_count;

    // 单个 host 的统计
    ping_stats_t ping_stats;
    port_stats_t port_status[32];
} host_entry_t;

/* ============================
 *  函数声明
 * ============================ */

// Raw socket ICMP ping（成功返回 0）
int icmp_ping(const char *host, double *rtt_ms);

// 非阻塞 TCP 扫描
int scan_port(const char *host, int port);

/* Stats 更新函数 */
void stats_init(ping_stats_t *s);
void stats_update_ping(ping_stats_t *s, int success, double rtt);
void stats_print(const ping_stats_t *s);

/* 多线程监控 */
void start_monitoring(host_entry_t *hosts, int host_count);

#endif
