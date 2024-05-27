#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
struct hostent* foundhost;
struct in_addr myaddress;

/* IPV dot-number into  binary form (network byte order) */
inet_aton(argv[1], &myaddress);

foundhost=gethostbyaddr((const char*)&myaddress, sizeof(myaddress), AF_INET);

if (foundhost!=NULL){
	printf("IP-address:%s Resolved to: %s\n", argv[1],foundhost->h_name);
	exit(0);
	}
else	{ 
	printf("IP-address:%s could not be resolved\n",argv[1]);
	exit(1);
	}
}
