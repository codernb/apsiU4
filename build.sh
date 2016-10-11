#!/bin/bash

gcc tcp_listener.c -o tcp_listener
gcc worker.c -o worker

./tcp_listener & ./worker &

result="$(echo xxx | nc localhost 8500)"

if [ $result = "XXX" ]; then
    echo "Success"
else
    echo "Error: $result"
fi

kill %1 %2
