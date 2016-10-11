#!/bin/bash

gcc tcp_listener.c -o tcp_listener.out
gcc worker.c -o worker.out

./tcp_listener.out & ./worker.out &

result="$(echo xxx | nc localhost 8500)"

if [ $result = "XXX" ]; then
    echo "Success"
else
    echo "Error: $result"
fi

kill %1 %2
