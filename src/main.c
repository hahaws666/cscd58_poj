#include "monitor.h"
#include "data_analysis.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define DEFAULT_LOG_FILE "monitor_records.log"
#define DEFAULT_LOG_FILE_STATS "monitor_records.log"
#define DEFAULT_LOG_FILE_REPORT "monitor_records.log"
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
        // printf("222222222my design is intend to seperate host and port by a single space\n");
        char *start_pos = strchr(line, ' ');
        *start_pos = '\0';
        // printf("debug noe copy to host name\n");
        strncpy(host->hostname, line, sizeof(host->hostname) - 1);
        host->hostname[sizeof(host->hostname) - 1] = '\0';
        char *tok = strtok(start_pos + 1, ",");
        int port_cnt = 0;
        while (tok && port_cnt < 32) {
            int port = atoi(tok);
            host->ports[port_cnt++] = port;
            tok = strtok(NULL, ",");
        }
        host->port_count = port_cnt;
        cnt++;
    }
    fclose(fp);
    return cnt;
}

void print_help() {
    printf("\n--- Available Commands ---\n");
    printf("  ping <host>            : Ping a host (e.g., ping google.com)\n");
    printf("  scan <host> <port>     : Check if a port is open (e.g., scan google.com 80)\n");
    printf("  monitor [count]        : Monitor hosts from config file (default 10 times)\n");
    printf("  report                 : Generate summary report from logs\n");
    printf("  stats                  : Show detailed statistics from logs\n");
    printf("  exit                   : Exit the program\n");
    printf("--------------------------\n\n");
}

void handle_command_view(const char *cmd) {
    system("clear");
    printf(">> Executing: %s", cmd);
    printf("---------------------------------------------------\n");
}

int main() {
    // 50 should be enough for this one..
    char cmd[50];
    system("clear");
    printf("Welcome to Network Monitor! Please play around with this\n");
    while(true){
        print_help();
        printf("Commands: ");
        fgets(cmd, 50, stdin);
        handle_command_view(cmd);
        // ping case
        if (strncmp(cmd, "ping", 4) == 0) {
            char host[50];
            sscanf(cmd, "ping %s", host);
            double rtt;
            if (icmp_ping(host, &rtt) == 0) {
                printf("RTT = %.2f ms\n", rtt);
            }
            else printf("Ping failed, please check your connection or the target host\n");
        } // scan port case
        else if (strncmp(cmd, "scan", 4) == 0) {
            char host[50];
            int port;
            sscanf(cmd, "scan %s %d", host, &port);
            int ans = scan_port(host, port);
            if (ans == 1) printf("Port is open\n");
            else if (ans == 2) printf("Port is closed\n");
            else printf("Port is timeout ot unknown or there is an error\n");
        } // monitor case
        else if (strncmp(cmd, "monitor", 7) == 0) {
            // read host config file
            host_entry_t hosts[100];
            int cnt = read_host_config("host_config.txt", hosts, 100);
            // as said the markdown file the default is 10
            // user is able to passed in another param to change this value
            int default_cnt = 10;
            int tmp;
            if (sscanf(cmd, "monitor %d", &tmp) == 1) default_cnt = tmp;
            //printf("the default cnt is %d", default_cnt);
            // 每个线程的记录
            // using一些C69知识
            pthread_t thread_ids[100];
            for (int i = 0; i < cnt; i++) {
                monitor_args_t *ans_args = malloc(sizeof(monitor_args_t));
                ans_args->host = &hosts[i];
                ans_args->cnt = default_cnt;
                ans_args->log_file = DEFAULT_LOG_FILE;
                // 初始化
                hosts[i].total_sent = 0.0;
                hosts[i].total_received = 0;
                hosts[i].last_rtt = 0.0;
                hosts[i].mn_rtt = 1e9;
                hosts[i].mx_rtt = 0.0;
                hosts[i].sum_rtt = 0.0;
                hosts[i].last_jitter = 0.0;
                hosts[i].sum_jitter = 0.0;
                hosts[i].mx_jitter = 0.0;
                hosts[i].loss_rate = 0.0;
                //创建线程
                pthread_create(&thread_ids[i], NULL, monitor_thread, ans_args);
            }
            //等全部结束。。。
            for (int i = 0; i < cnt; i++) {
                pthread_join(thread_ids[i], NULL);
            }
            printf("All monitoring completed.\n");
        } else if (strncmp(cmd, "report", 6) == 0) {            
            monitor_record_t records[1000];
            size_t cnt = dataload(DEFAULT_LOG_FILE_REPORT, records, 1000);
            if (cnt == 0) printf("No records in %s\n", DEFAULT_LOG_FILE_REPORT);
            else {
                int sent, received;
                double last_rtt, mn_rtt, mx_rtt, avg, loss_rate;
                datareport(records, cnt, &sent, &received, &last_rtt, &mn_rtt, &mx_rtt, &avg, &loss_rate);
                // printf("11111Debuing the monitor report....\n");
                printf("Total record #: %zu\n", cnt);
                printf("Total sent: %d\n", sent);
                printf("Total received: %d\n", received);
                printf("Loss rate: %.2f%%\n", loss_rate * 100.0);
                printf("Last RTT: %.2f ms\n", last_rtt);
                printf("Min RTT: %.2f ms\n", mn_rtt);
                printf("Max RTT: %.2f ms\n", mx_rtt);
                printf("Avg RTT: %.2f ms\n", avg);
            }
        } else if (strncmp(cmd, "stats", 5) == 0) {            
            monitor_record_t records[1000];
            size_t cnt = dataload(DEFAULT_LOG_FILE_STATS, records, 1000);
            if (cnt == 0) printf("No records in %s\n", DEFAULT_LOG_FILE_STATS);
            else {
                // printf("Now we areat the stats part\n");
                printf("Total number of record is: %zu\n", cnt);
                printf("Detailed Statistics (Last 10 records):\n");
                size_t start_index = (cnt > 10) ? (cnt - 10) : 0;
                for (size_t i = start_index; i < cnt; i++) {
                    // A usefule time struct in c
                    // https://man7.org/linux/man-pages/man3/tm.3type.html
                    struct tm *ans_time = localtime(&records[i].timestamp);
                    printf("Time is: %04d-%02d-%02d %02d:%02d:%02d, host is: %s, RTT: %.2f ms, status: %s\n", ans_time->tm_year + 1900, ans_time->tm_mon + 1, ans_time->tm_mday, ans_time->tm_hour, ans_time->tm_min, ans_time->tm_sec, records[i].hostname, records[i].rtt_ms, (records[i].ping ? "SUCESS!" : "FAIL!"));
                }
                int sent, received;
                double last_rtt, mn_rtt, mx_rtt, avg, loss_rate;
                datareport(records, cnt, &sent, &received, &last_rtt, &mn_rtt, &mx_rtt, &avg, &loss_rate);
                printf("\n");
                printf("Total sent: %d\n", sent);
                printf("Total received: %d\n", received);
                printf("Loss rate: %.2f%%\n", loss_rate * 100.0);
                printf("Last RTT: %.2f ms\n", last_rtt);
                printf("Min RTT: %.2f ms\n", mn_rtt);
                printf("Max RTT: %.2f ms\n", mx_rtt);
                printf("Avg RTT: %.2f ms\n", avg);
            }

        } else if (strncmp(cmd, "exit", 4) == 0) {
            printf("BYE BYE!!\n");
            break;
        } else if (strncmp(cmd, "help", 4) == 0) {
            system("clear"); 
            print_help();
        }else {
            if (strlen(cmd) > 1) {
                printf(">> Unknown command: %s", cmd);
                printf("Please check the available commands below:\n");
            }
        }
    }
}
