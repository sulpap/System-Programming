#
# Makefile 
#
# Type  make    to compile all the programs
# in the chapter 
#
all: inet_str_server inet_str_client \
	inet_dgr_server inet_dgr_client \
	rls rlsd prs prsref \
	GetHostByName-p18 \
	GetHostByAddress-p20\
	ByteOrder-p16

clean: 
	rm -f inet_str_server \
	inet_str_client int_dgr_server inet_dgr_client \
	./rls ./rlsd ./prs ./prsref \
	GetHostByName-p18 \
	GetHostByAddress-p20 \
	ByteOrder-p16

inet_dgr_server: inet_dgr_server.c 
	gcc -o inet_dgr_server inet_dgr_server.c

inet_dgr_client: inet_dgr_client.c 
	gcc -o inet_dgr_client inet_dgr_client.c

inet_str_server: inet_str_server.c
	gcc -o inet_str_server inet_str_server.c

inet_str_client: inet_str_client.c
	gcc -o inet_str_client inet_str_client.c

rlsd: rlsd.c 
	gcc rlsd.c -o ./rlsd 

rls: rls.c 
	gcc rls.c -o ./rls 

prsref: prsref.c 
	gcc prsref.c -o ./prsref

prs: prs.c 
	gcc prs.c -o ./prs

GetHostByName-p18: GetHostByName-p18.c
	gcc GetHostByName-p18.c -o GetHostByName-p18

GetHostByAddress-p20: GetHostByAddress-p20.c
	gcc GetHostByAddress-p20.c -o GetHostByAddress-p20

ByteOrder-p16: ByteOrder-p16.c
	gcc ByteOrder-p16.c -o ByteOrder-p16
