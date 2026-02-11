# SNIFFING

SNIFFING is a TCP/IP network sniffer written in C using the libpcap library.  
It captures network traffic on the host machine and records metrics related to visited websites, including the time spent on specific domains.

A CLI client allowing users to manage monitored hostnames and retrieve statistics.

## Requirements

- GCC (or compatible C compiler)
- libpcap
- Make

## Build & Run

### Start the Server

```bash
make run
```

### Start the Client

````bash
make run_client`

### Run Tests

```bash
make test
````

## Client Commands

Once the client is running, you can use the interactive prompt:

```bash
sniffing>
```

### Add a Hostname to Monitor

```bash
hostname add www.instagram.com
```

### List Monitored Hostnames

```bash
hostname list
```

### Display Metrics

```bash
stats
```

### Stop the Server

```bash
server stop
```

### Start the Server

```bash
server start
```
