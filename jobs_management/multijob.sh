#!/bin/bash

# check if the number of arguments provided is correct
if [ "$#" -lt 1 ]; then
    echo "Usage: $0 <file1.txt> <file2.txt> ... <fileN.txt>"
    exit 1
fi

cd bin

# iterate over each file argument
for givenFile in "$@"; do
    # check if file exists and is readable
    file="../$givenFile"
    if [ ! -f "$file" ]; then
        echo "Error: File '$file' not found"
        continue
    fi

    # read each line from the file and execute it as a command
    while IFS= read -r line; do
        # execute the command
        ./jobCommander issueJob $line
    done < "$file"
done

exit 0
