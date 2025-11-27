# Network Monitor CLI

This project provides a simple network monitoring CLI written in C. It supports ICMP ping latency measurement, TCP port scanning, and multi-threaded monitoring for predefined hosts.

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

## Build

```bash
cd /home/mininet/project/cscd58_poj
make
```

The resulting binary is produced at `bin/monitor`.

## Run

Root privileges are required because raw sockets are used:

```bash
sudo ./bin/monitor
```

Available CLI commands:

- `ping <host>`: send ICMP echo requests and report RTT.
- `scan <host> <port>`: perform a non-blocking TCP port scan.
- `monitor [count]`: start multi-thread monitoring; optional `count` sets sample iterations (default 10).
  - Automatically saves monitoring records to `monitor_records.log`
  - Displays uptime percentage upon completion
- `report [file]`: generate statistical report from log file (default: `monitor_records.log`)
  - Shows total records, min/max/avg RTT, packet loss rate
- `stats [file]`: show detailed statistics including recent records (default: `monitor_records.log`)
  - Displays timestamped records and aggregated statistics
- `exit`: quit the CLI.

Examples:

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

## Data Storage

Monitoring data is automatically saved to `monitor_records.log` (or custom file path) during monitoring sessions. Each record includes:
- Timestamp
- Hostname
- Ping success/failure status
- RTT (Round-Trip Time)
- Port scan results for configured ports

Use the `report` or `stats` commands to analyze historical data.

## Testing

A standalone test program is available for the data management module:

```bash
cd /home/mininet/project/cscd58_poj
gcc -Wall -Wextra -std=c11 -Isrc -Iinclude tests/data_analysis_test.c src/data_analysis.c src/stats.c -o build/data_analysis_test
./build/data_analysis_test
```

This test verifies:
- Historical data storage and loading
- Statistical report generation
- Uptime tracking
- Threshold-based alerting

## Project Structure

```
cscd58_poj/
├── src/
│   ├── main.c           # CLI entry point
│   ├── monitor.c        # Multi-threaded monitoring
│   ├── monitor.h        # Core data structures
│   ├── icmp.c           # ICMP ping implementation
│   ├── tcp_scan.c       # TCP port scanning
│   ├── stats.c          # Statistics computation
│   ├── data_analysis.c  # Data management & reporting
│   └── data_analysis.h  # Data management API
├── include/
├── tests/
│   └── data_analysis_test.c
├── build/               # Compiled objects
├── bin/                 # Executable binary
└── Makefile
```

## Clean

```bash
make clean
```

