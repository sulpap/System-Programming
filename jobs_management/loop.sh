#!/bin/bash

# Check if the number of arguments provided is correct
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <number_of_times> <message>"
    exit 1
fi

num_times=$1
message=$2
for ((i=1; i<=$num_times; i++)); do
    echo "Loop iteration: $i message: $message"
    sleep 1
done
