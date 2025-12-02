#include "monitor.h"
#include "data_analysis.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define DEFAULT_LOG_FILE "monitor_records.log"
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

// Helper to append a new host to the config file
void append_host_to_config(const char *host, const char *ports) {
    FILE *fp = fopen("host_config.txt", "a");
    fprintf(fp, "%s %s\n", host, ports);
    fclose(fp);
    printf("Successfully added host: %s\n", host);
}
// Helper to show current configuration
void show_config() {
    FILE *fp = fopen("host_config.txt", "r");
    printf("--- Current Host Configuration ---\n");
    char line[256];
    int i = 1;
    while (fgets(line, sizeof(line), fp)) {
        if (strlen(line) > 1) {
            printf("%d. %s", i, line);
            i++;
        }
    }
    printf("----------------------------------\n");
    fclose(fp);
}
void print_help() {
    printf("\n--- Available Commands ---\n");
    printf("  ping <host>            : Ping a host (e.g., ping google.com)\n");
    printf("  scan <host> <port>     : Check a specific port (e.g., scan google.com 80)\n");
    printf("  scan <host>            : Scan common ports (e.g., scan google.com)\n");
    printf("  monitor [count]        : Monitor hosts from config file (default 10 times)\n");
    printf("  report                 : Generate summary report from logs\n");
    printf("  stats [all]            : Show statistics (default: last 10, 'all': full history)\n");
    printf("  add <host> <ports>     : Add a new host (e.g., add 10.0.0.1 80)\n");
    printf("  show                   : Show current list of monitored hosts\n");
    printf("  exit                   : Exit the program\n");
    printf("--------------------------\n");
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
            handle_command_view(cmd);
            char host[100];
            int port;
            // Check how many arguments were parsed
            int args = sscanf(cmd, "scan %s %d", host, &port);
            if (args == 2) {
                //Specific Port
                printf("Scanning %s : %d...\n", host, port);
                int ans = scan_port(host, port);
                if (ans == 1) printf(">> Port %d is OPEN\n", port);
                else if (ans == 2) printf(">> Port %d is CLOSED\n", port);
                else printf(">> Port %d status is UNKNOWN/TIMEOUT\n", port);
            } else if (args == 1) {
                // No Port Specified -> Scan Common Ports
                printf("No port specified. Scanning common ports for %s...\n", host);
                printf("This may take a few seconds depending on network speed.\n");
                printf("---------------------------------------------------\n");
                // List of most common TCP ports to check
                int common_ports[] = {21, 22, 23, 25, 53, 80, 110, 135, 139, 143, 443, 445, 993, 995, 3306, 3389, 8080};
                int num_ports = 17;
                int ans_cnt = 0;
                for (int i = 0; i < num_ports; i++) {
                    int p = common_ports[i];
                    printf("Checking %d...\r", p); 
                    fflush(stdout); 
                    int ans = scan_port(host, p);
                    if (ans == 1) {
                        printf("  [+] Port %-5d is OPEN \n", p);
                        ans_cnt++;
                    }
                }
                printf("                                   \r");
                if (ans_cnt == 0) printf("Scan complete. No common open ports found.\n");
                else printf("Scan complete. Found %d open ports.\n", ans_cnt);
            } else {
                printf("Usage:\n");
                printf("  scan <host> <port>  : Check specific port\n");
                printf("  scan <host>         : Check common ports\n");
            }
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
            size_t cnt = dataload(DEFAULT_LOG_FILE, records, 1000);
            if (cnt == 0) printf("No records in %s\n", DEFAULT_LOG_FILE);
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
            size_t cnt = dataload(DEFAULT_LOG_FILE, records, 1000);
            if (cnt == 0) printf("No records in %s\n", DEFAULT_LOG_FILE);
            else {
                int show_all = (strstr(cmd, "all") != NULL);
                printf("Total number of record is: %zu\n", cnt);
                size_t start_index = 0;
                if (show_all) {
                    printf("Detailed Statistics (FULL HISTORY):\n");
                    start_index = 0; // Start from beginning
                } else {
                    printf("Detailed Statistics (Last 10 records):\n");
                    // Default:show only last 10
                    start_index = (cnt > 10) ? (cnt - 10) : 0;
                }
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
        } else if (strncmp(cmd, "add", 3) == 0) {
            handle_command_view(cmd);
            char host[100];
            char ports[100];
            if (sscanf(cmd, "add %s %s", host, ports) == 2) {
                append_host_to_config(host, ports);
            } else {
                printf("Usage: add <host> <ports>\n");
                printf("Example: add 192.168.1.5 22,80\n");
            }
        } else if (strncmp(cmd, "show", 4) == 0) {
            handle_command_view(cmd);
            show_config();
        }
         else if (strncmp(cmd, "exit", 4) == 0) {
            printf("BYE BYE!!\n");
            break;
        } else {
            if (strlen(cmd) > 1) {
                printf(">> Unknown command: %s", cmd);
                printf("Please check the available commands below:\n");
                print_help();
            }
        }
    }
}
