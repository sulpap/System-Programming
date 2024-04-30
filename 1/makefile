# make:
# 	gcc -o my_program main.c jobCommander.c jobExecutorServer.c -Wall

# clean:
# 	rm my_program
# 	rm jobExecutorServer_file
# 	rm myfifo1
# 	rm myfifo2

# # rm -f $(OBJS) $(OUT) $(FILES)

SOURCE = issueJob.c jobCommander.c jobExecutorServer.c list.c main.c
OBJS = issueJob.o jobCommander.o jobExecutorServer.o list.o main.o
HEADER = myheaders.h list.h
OUT = jobCommander
FILES = jobExecutorServer_file myfifo1 myfifo2
CC = gcc
FLAGS = -c

all: $(OBJS)
	$(CC) $(OBJS) -o $(OUT)

main.o : main.c
	$(CC) $(FLAGS) main.c

issueJob.o : issueJob.c
	$(CC) $(FLAGS) issueJob.c

jobCommander.o : jobCommander.c
	$(CC) $(FLAGS) jobCommander.c

jobExecutorServer.o : jobExecutorServer.c
	$(CC) $(FLAGS) jobExecutorServer.c

list.o : list.c
	$(CC) $(FLAGS) list.c

build : $(SOURCE)
	$(CC) -o $(OUT) $(SOURCE)

clean : 
	rm -f $(OBJS) $(OUT) $(FILES)