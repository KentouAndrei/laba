#!/bin/bash

while true;

do

    sleep $((RANDOM % 6 + 5))

    curl -i -X GET 127.0.0.1/compute > /dev/null 2>&1 &

    echo "HTTP request dispatched at $(date)"

done
