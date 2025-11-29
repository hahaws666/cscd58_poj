#include "monitor.h"
#include "data_analysis.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_LOG_FILE "monitor_records.log"
#define DEFAULT_LOG_FILE_STATS "monitor_records_stat.log"
#define DEFAULT_LOG_FILE_REPORT "monitor_records_report.log"
#define MX 65535
#define SIZE 512
// Read a given file one by one line and then record each line's info to hostname what is returned is how many hosts are loaded
// hopefully fixed at 2025/11/28
int read_host_config(const char *file, host_entry_t *hosts, int mx) {
    FILE *fp = fopen(file, "r");
    char line[SIZE];
    int cnt = 0;
    while (cnt < mx && fgets(line, SIZE, fp)) {
        host_entry_t *host = &hosts[cnt];
        // https://www.geeksforgeeks.org/c/strchr-in-c/
        char *start_pos = strchr(line, ' ');
        *start_pos = '\0';
        printf("debug noe copy to host name\n");
        strncpy(host->hostname, line, sizeof(host->hostname) - 1);
        host->hostname[sizeof(host->hostname) - 1] = '\0';
        char *tok = strtok(start_pos + 1, ",");
        int port_cnt = 0;
        while (tok && port_cnt < 32) {
            int port = atoi(tok);
            host->ports[port_cnt++] = port;
            tok = strtok(NULL, ",");
        }
        host->port_cnt = port_cnt;
        cnt++;
    }
    fclose(fp);
    return cnt;
}

int main() {
    // 50 should be enough for this one..
    char cmd[50];
    while(true){
        printf("Network Monitor\n");
        printf("Commands:\n");
        fgets(cmd, sizeof(cmd), stdin);
        // ping case
        if (strncmp(cmd, "ping", 4) == 0) {
            char host[50];
            sscanf(cmd, "ping %s", host);
            double rtt;
            if (icmp_ping(host, &rtt) == 0) {
                printf("RTT = %.2f ms\n", rtt);
            }
            else printf("Ping failed, please check your connection\n");
        } // scan port case
        else if (strncmp(cmd, "scan", 4) == 0) {
            char host[50];
            int port;
            sscanf(cmd, "scan %s %d", host, &port);
            int ans = scan_port(host, port);
        } // monitor case
        else if (strncmp(cmd, "monitor", 7) == 0) {
            // read host config file
            host_entry_t hosts[100];
            int cnt = read_host_config("host_config.txt", hosts, 100);
            // as said the markdown file the default is 10
            // user is able to passed in another param to change this value
            int default_cnt = 10;
            int tmp;
            if (sscanf(cmd, "monotor %d", &tmp) == 1) default_cnt = tmp;
            // 每个线程的记录
            // using一些C69知识
            pthread_t thread_ids[100];
            for (int i = 0; i < cnt; i++) {
                monitor_args_t *ans_args = malloc(sizeof(monitor_args_t));
                ans_args->host = &hosts[i];
                ans_args->cnt = default_cnt;
                ans_args->log_file = DEFAULT_LOG_FILE;
                // 初始化
                hosts[i].ping_stats.total_sent = 0.0;
                hosts[i].ping_stats.total_received = 0;
                hosts[i].ping_stats.last_rtt = 0.0;
                hosts[i].ping_stats.mn_rtt = 1e9;
                hosts[i].ping_stats.mx_rtt = 0.0;
                hosts[i].ping_stats.sum_rtt = 0.0;
                hosts[i].ping_stats.loss_rate = 0.0;
                // 创建线程
                pthread_create(&thread_ids[i], NULL, monitor_thread, ans_args);
            }
            // 等全部结束。。。
            for (int i = 0; i < threads_created; i++) {
                pthread_join(thread_ids[i], NULL);
            }
            printf("All monitoring completed.\n");
        } else if (strcmp(cmd, "report") == 0) {            
            monitor_record_t records[1000];
            size_t cnt = dataload(DEFAULT_LOG_FILE_REPORT, records, 1000);
            if (cnt == 0) printf("No records found in %s\n", DEFAULT_LOG_FILE_REPORT);
            else {
                ping_stats_t s;
                datareport(records, cnt, &s);
                printf("Debuging the monitor report....\n");
                printf("Total number of record is: %zu\n", cnt);
                printf("Detailed stats of report coming below.....\n");
                statsPrint(&s);
            }
        } else if (strcmp(cmd, "stats") == 0) {            
            monitor_record_t records[1000];
            size_t cnt = dataload(DEFAULT_LOG_FILE_STATS, records, 1000);
            if (cnt == 0) printf("No records found in %s\n", DEFAULT_LOG_FILE_STATS);
            else {
                printf("Now we are at the stats part\n");
                printf("Total number of record is: %zu\n\n", cnt);
                for (size_t i = 0; i < cnt && i < 10; i++) {
                    struct tm *tm_info = localtime(&records[i].timestamp);
                    char time_str[64];
                    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
                    printf("[%s] %s: RTT=%.2f ms, Status=%s\n", time_str, records[i].hostname, records[i].rtt_ms, records[i].ping_success ? "OK" : "FAIL");
                }
                ping_stats_t s;
                datareport(records, cnt, &s);
                printf("\n");
                statsPrint(&s);
            }

        } else if (strcmp(cmd, "exit") == 0) {
            break;
        }
    }
}
