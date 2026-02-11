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

`make run`

### Start the Client

`make run_client`

### Run Tests

`make test`

## Client Commands

Once the client is running, you can use the interactive prompt:
`sniffing>`

### Add a Hostname to Monitor

`sniffing> hostname add www.instagram.com`

### List Monitored Hostnames

`sniffing> hostname list`

### Display Metrics

`sniffing> stats`

### Stop the Server

`sniffing> server stop`

### Start the Server

`sniffing> server start`
