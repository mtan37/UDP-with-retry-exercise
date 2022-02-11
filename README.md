# UDP-with-retry-exercise

## Usage
```
./server <port> <message_drop_percentage> [-d]
-d - turn on to print server debug message

./client <server IP> <server port> <packet size(in KB)> <packet count> [-R]
-R - turn on to tell client that we are trying to test reliability... a increasing sequence of 0s will be sent
```


## Get latency and throughput
First execute `./server <port> 0`

To run client `./client <server IP> <server port> <packet size in K> <packet count>`

client packet size range from 1 to 32

## Check reliability

To check that the server actually received the client's message, and when it drops them.
It also prints out cline't retry time to send the same message before it receives the server's acknowledgment

First execute `./server <port> 10 -d`

To run client `./client <server IP> <server port> 1 10`
