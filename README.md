# Network Monitor Project

This is Our CSCD58 project, Network Monitoring System. It is a simple network monitoring written in C and play with command line prompt. It has some key features such ping latency, port scanning, and multi-threaded monitoring, and summary of report.

## Group Members
- Shuang Wu 1008838135 shuaang.wu@mail.utoronto.ca
- Andrew Li 1008837993 qj.li@mail.utoronto.ca
- Delun Sun 1007925391 delun.sun@mail.utoronto.ca

## Contribution
- Shuang Wu: 
- Andrew Li: Gives lots of great ideas in our proposal and implemented stats and report functions with shuang.
- Delun Sun: 

## Functionalities overview
- `ping`: sends an ICMP request based on raw socket and print result(similar to our ping)
- `scan`: do a tcp connection with given host and port and determine the status of port
- `monitor`: based on a multi-thread repeatdly ping and scan with a list given in `host_congfig.txt`, result is recorded in a log file
- `stats`: load a log file and print the summary of its monitored results.
- `report`: load a log file and calculate the ping's statistics

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
RTT = 52.98 ms
```

**We can use scan port**
```bash
scan google.com 22
```
Example output:
```bash
1111111111111 start of scan port!!
Port is open
```

**We can monitor the hosts from a config file with all targetes and all results will be stored in a log file**
```bash
monitor # defualt is 10 times, you can customize it
monitor 5
```

Example output log file:
```bash
1764545602,1.1.1.1,1,14.01,1,53:1
1764545602,www.google.com,1,50.76,1,80:1
1764545602,8.8.8.8,1,3.74,2,80:1,443:1
1764545602,192.168.56.1,1,3.86,2,22:1,8080:1
1764545605,1.1.1.1,1,1049.63,1,53:1
1764545605,www.google.com,1,1041.97,1,80:1
1764545606,8.8.8.8,1,1015.94,2,80:1,443:1
1764545607,1.1.1.1,1,19.40,1,53:1
1764545607,www.google.com,1,46.37,1,80:1
1764545607,192.168.56.1,1,1004.36,2,22:1,8080:1
1764545609,8.8.8.8,1,53.91,2,80:1,443:1
1764545610,1.1.1.1,1,1043.98,1,53:1
1764545610,www.google.com,1,1019.14,1,80:1
1764545612,1.1.1.1,1,19.83,1,53:1
1764545612,www.google.com,1,44.69,1,80:1
1764545611,192.168.56.1,1,1.78,2,22:1,8080:1
1764545612,8.8.8.8,1,99.55,2,80:1,443:1
1764545615,1.1.1.1,1,1029.66,1,53:1
1764545615,www.google.com,1,934.78,1,80:1
1764545615,8.8.8.8,1,25.99,2,80:1,443:1
1764545615,192.168.56.1,1,276.46,2,22:1,8080:1
1764545617,1.1.1.1,1,19.75,1,53:1
1764545617,www.google.com,1,20.11,1,80:1
1764545619,8.8.8.8,1,982.81,2,80:1,443:1
1764545620,1.1.1.1,1,1013.24,1,53:1
1764545620,www.google.com,1,1002.61,1,80:1
1764545619,192.168.56.1,1,1.46,2,22:1,8080:1
1764545622,1.1.1.1,1,19.91,1,53:1
1764545622,www.google.com,1,29.24,1,80:1
1764545622,8.8.8.8,1,24.93,2,80:1,443:1
1764545625,www.google.com,1,1037.29,1,80:1
1764545625,1.1.1.1,1,1051.93,1,53:1
1764545624,192.168.56.1,1,1031.96,2,22:1,8080:1
1764545626,8.8.8.8,1,1001.29,2,80:1,443:1
1764545628,192.168.56.1,1,1.48,2,22:1,8080:1
1764545629,8.8.8.8,1,18.29,2,80:1,443:1
1764545633,8.8.8.8,1,1003.89,2,80:1,443:1
1764545633,192.168.56.1,1,1010.78,2,22:1,8080:1
1764545637,192.168.56.1,1,1.80,2,22:1,8080:1
1764545642,192.168.56.1,1,907.65,2,22:1,8080:1
```
So each line represent a result, where splitting by comma we have different field, the first is the timestamp; the second is the ipaddress or host name; the third is the ping status, where 1 represents ping of success 0 represents failure; fourth is the RTT time in ms; fifth is the number of ports scanned, while followed by each port's status, 0 means unknown/timeout/error, 1 means open, 2 means closed.

**generate summary report from monitor log**
```bash
report
```
Example output:
```bash
Total record #: 40
Total sent: 40
Total received: 40
Loss rate: 0.00%
Last RTT: 907.65 ms
Min RTT: 1.46 ms
Max RTT: 1051.93 ms
Avg RTT: 474.51 ms
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
https://man7.org/linux/man-pages/man3/tm.3type.html