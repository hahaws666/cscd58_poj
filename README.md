# Network Monitor Project

This is Our CSCD58 project, Network Monitoring System. It is a simple network monitoring written in C and play with command line prompt. It has some key features such ping latency, port scanning, and multi-threaded monitoring.

## Group Members
- Shuang Wu 1008838135 shuaang.wu@mail.utoronto.ca
- Andrew Li 1008837993 qj.li@mail.utoronto.ca
- Delun Sun 1007925391 delun.sun@mail.utoronto.ca

## Some important functionalities

### Monitoring Capabilities
- Ping: ICMP ping just like ping command based on raw socket
- Port scanning: We will scan the tcp port to see if it is available
- Late measurement: Our measurement is based on RTT tracking with all kind of stats

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
sudo ./monitor
```

An important thing is sudo is required here

## Examples to play around with this:
**We can use ping**:
```bash
ping google.com
```
Example Output:
```bash

```

**We can use scan port**
```bash
scan google.com 22
```
Example output:

**We can monitor the hosts from a config file with all targetes and all results will be stored in a log file**
```bash
monitor # defualt is 10 times, you can customize it
monitor 5
```

Example output:

**generate summary report from monitor log**
```bash
report
```

**show detailed stats**
```bash
stats
```

## File Structures
We can compile all by MAKEFILE management and all header and c files are stored in `src` directory

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
https://sites.uclouvain.be/SystInfo/usr/include/netinet/ip_icmp.h.html
https://pubs.opengroup.org/onlinepubs/009604499/functions/sendto.html
https://en.cppreference.com/w/c/chrono/timespec
https://man7.org/linux/man-pages/man3/sscanf.3.html
https://stackoverflow.com/questions/5582211/what-does-define-gnu-source-imply 