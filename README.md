This repository consists of two projects:
1. jobs_management
2. multithread_network

Jobs_management is a client-server program, where the client (commander) and the server (executor) communicate via named pipes. It undertakes the execution of tasks received from an input file.
- To run:
  make
  cd bin
  ./jobCommander <command_name> <arguments>

Multithread_network is a multithreaded network application, an extension of jobs_management.
- To run, open at least 2 terminals:
  - terminal 1:
    make
    cd bin
    ./jobExecutorServer [portNum] [bufferSize] [threadPoolSize]
    (ex ./jobExecutorServer 7856 5 8)

  - terminal 2(+):
    cd bin
    /jobCommander [serverName] [portNum] [jobCommanderInputCommand]
    (ex ./jobCommander linux18 7856 issueJob ./progDelay 15)
  
