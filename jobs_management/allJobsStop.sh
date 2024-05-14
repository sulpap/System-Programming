#!/bin/bash

# check if the number of arguments provided is correct
if [ "$#" -ne 0 ]; then
    echo "Usage: ./allJobsStop.sh"
    exit 1
fi

# check if the executor exists
if [ ! -f "bin/jobExecutorServer.txt" ]; then
    echo "JobExecutor not found"
    exit 1
fi

cd bin

# stop running jobs
./jobCommander poll running | while read -r line; do
    jobId=$(echo "$line" | grep -oP 'job_\K\d+')
    if [ -n "$jobId" ]; then
        ./jobCommander stop "job_$jobId"
    fi
done

# stop queued jobs
./jobCommander poll queued | while read -r line; do
    jobId=$(echo "$line" | grep -oP 'job_\K\d+')
    if [ -n "$jobId" ]; then
        ./jobCommander stop "job_$jobId"
    fi
done

exit 0