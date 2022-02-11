# UDP-with-retry-exercise

## Get latency and throughput
First execute `./server <port> 0`

To run client `./client <server IP> <server port> <packet size in K> <packet count>`

client packet size range from 1 to 32

## Check reliability

To check that the server actually received the client's message, and when it drops them.
It also prints out cline't retry time to send the same message before it receives the server's acknowledgment

First execute `./server <port> 10 -d`

To run client `./client <server IP> <server port> 1 10`
