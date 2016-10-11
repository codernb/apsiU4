#!/bin/bash

if [ "$EUID" -ne 0 ]
  then echo "Please run as root"
  exit
fi

echo "Compiling"
gcc tcp_listener.c -o tcp_listener.out
gcc worker.c -o worker.out

echo "Starting"
./tcp_listener.out &
echo "tcp_listener running"
./worker.out &
echo "worker running"

echo "Testing"
result="$(echo xxx | nc localhost 8500)"

if [ $result = "XXX" ]; then
    echo "Success"
else
    echo "Error: $result"
fi

kill %1 %2
