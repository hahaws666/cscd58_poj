#ifndef MONITOR_H
#define MONITOR_H

#include <stdint.h>
#include <netinet/in.h>
#include <pthread.h>

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

typedef struct {
    host_entry_t *host;   // 第 i 个 host 的地址
    int cnt;     // 每个 host 的样本数
    const char *log_file; // Optional log file path for data storage
} monitor_args_t;


/* 
 *  函数声明
*/
// Raw socket ICMP ping（成功返回 0）
int icmp_ping(const char *host, double *rtt_ms);

// 非阻塞 TCP 扫描
int scan_port(const char *host, int port);

/* Stats 更新函数 */
void statsUpdate(ping_stats_t *s, int success, double rtt);
void statsPrint(const ping_stats_t *s);

/* 多线程监控 */
int start_monitoring(host_entry_t *hosts, int host_count, int cnt, const char *log_file, pthread_t *thread_ids);
void *monitor_thread(void *arg) 
#endif
