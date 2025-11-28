# Network Monitor CLI

This project provides a simple network monitoring CLI written in C. It supports ICMP ping latency measurement, TCP port scanning, and multi-threaded monitoring for predefined hosts.

## Group Members
- Shuang Wu 1008838135 shuaang.wu@mail.utoronto.ca
- Andrew Li 1008837993 qj.li@mail.utoronto.ca
- Delun Sun 1007925391 delun.sun@mail.utoronto.ca
## Features

### Monitoring Capabilities
- **Multi-host monitoring**: Concurrent monitoring of multiple hosts using threads
- **ICMP reachability testing**: Raw socket-based ICMP ping for latency measurement
- **TCP port scanning**: Non-blocking TCP port scanning to check service availability
- **Latency measurement**: Round-trip time (RTT) tracking with min/max/avg statistics
- **Packet loss calculation**: Automatic packet loss rate computation

### Data Management
- **Historical data storage**: Automatically persist monitoring records to log files during monitoring
- **Statistical reports**: Generate comprehensive reports with aggregated statistics (min/max/avg RTT, loss rate)
- **Uptime tracking**: Track host availability and calculate uptime percentage in real-time
- **Threshold-based alerts**: Configure latency, loss rate, and uptime thresholds to trigger alerts
- **Integrated CLI commands**: Access data analysis features directly through `report` and `stats` commands

## Compile and run

```bash
cd /home/mininet/project/cscd58_poj
make
sudo ./bin/monitor
```

sudo is required here

## Examples to play around with this:

```bash
> monitor 3
Starting multi-thread monitoring...
Starting monitoring of 8.8.8.8 with 3 samples
Monitoring thread started for 8.8.8.8 with 3 samples
ATTEMPT 3: [PING] 8.8.8.8 -> OK, 6.22 ms
ATTEMPT 3: [PORT 80] -> TIMEOUT
ATTEMPT 3: [PORT 443] -> OPEN
...
Monitoring of 8.8.8.8 completed (Uptime: 100.00%)

> report
=== Monitoring Report ===
Total records: 3
---- Ping Stats ----
Total sent:      3
Total received:  3
Loss rate:       0.00%
Last RTT:        6.46 ms
Min RTT:         6.22 ms
Max RTT:         8.40 ms
Avg RTT:         7.29 ms
---------------------

> stats monitor_records.log
=== Detailed Statistics ===
Total records: 3
...
```
## File Structures
We can compile all by MAKEFILE management and all header and c files are stored in `src` directory

## Data Storage

Monitoring data is automatically saved to `monitor_records.log` (or custom file path) during monitoring sessions. Each record includes:
- Timestamp
- Hostname
- Ping success/failure status
- RTT (Round-Trip Time)
- Port scan results for configured ports

Use the `report` or `stats` commands to analyze historical data.

## Clean the executable

```bash
make clean
```

## References
https://man7.org/linux/man-pages/man3/getaddrinfo.3.html
https://man7.org/linux/man-pages/man3/clock_gettime.3.html
https://pubs.opengroup.org/onlinepubs/009695099/functions/setsockopt.html
https://man7.org/linux/man-pages/man2/select.2.html
https://elixir.bootlin.com/linux/v6.11.6/source/drivers/tty/serial/8250/8250_port.c#L54