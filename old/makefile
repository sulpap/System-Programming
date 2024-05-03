SOURCE = issueJob.c jobCommander.c jobExecutorServer.c list.c main.c
OBJS = issueJob.o jobCommander.o jobExecutorServer.o list.o main.o
HEADER = jobs.h list.h
OUT = jobCommander
FILES = server_file myfifo myfifo2
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

rebuild : clean build

