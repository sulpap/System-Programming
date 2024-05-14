#!/bin/bash

# general version

# check if the number of arguments provided is correct
if [ "$#" -lt 1 ]; then
    echo "Usage: $0 <file1.txt> <file2.txt> ... <fileN.txt>"
    exit 1
fi

# iterate over each file argument
for file in "$@"; do
    # check if file exists and is readable
    if [ ! -f "$file" ]; then
        echo "Error: File '$file' not found"
        continue
    fi
    
    # read each line from the file and execute it as a command
    while IFS= read -r line; do
        # execute the command
        eval "$line"
    done < "$file"
done

exit 0

# #!/bin/bash

#jobCommander version -- Run make and paste the txt files you want to test multijob with in the bin. Then run ./multijob.sh (outside bin)

# # check if the number of arguments provided is correct
# if [ "$#" -lt 1 ]; then
#     echo "Usage: $0 <file1.txt> <file2.txt> ... <fileN.txt>"
#     exit 1
# fi

# # check if the executor exists
# if [ ! -f "bin/jobExecutorServer.txt" ]; then
#     echo "JobExecutor not found. Run make and paste the txt files you want to test multijob with in the bin. Then try again."
#     exit 1
# fi

# cd bin

# # iterate over each file argument
# for file in "$@"; do
#     # check if file exists and is readable
#     if [ ! -f "$file" ]; then
#         echo "Error: File '$file' not found"
#         continue
#     fi
    
#     # read each line from the file and execute it as a command
#     while IFS= read -r line; do
#         # execute the command
#         ./jobCommander issueJob "$line"
#     done < "$file"
# done

# exit 0
