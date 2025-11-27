# Network Monitor CLI

This project provides a simple network monitoring CLI written in C. It supports ICMP ping latency measurement, TCP port scanning, and multi-threaded monitoring for predefined hosts.

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
- `exit`: quit the CLI.

Example:

```bash
> monitor 3
```

## Clean

```bash
make clean
```

