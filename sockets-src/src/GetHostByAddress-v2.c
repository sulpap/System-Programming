#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
struct hostent* esu;
struct in_addr myaddress;

inet_aton(argv[1], &myaddress);
esu=gethostbyaddr((const char*)&myaddress, sizeof(myaddress), AF_INET);
if (esu!=NULL){
	printf("IP address %s Resolved to Name: %s\n", argv[1],esu->h_name);
	exit(0);
	}
else	{ 
	printf("IP address %s could not be resolved\n",argv[1]);
	exit(1);
	}
}





#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

int main()
{
    struct sockaddr_in sa;    /* input */
    socklen_t len;         /* input */
    char hbuf[NI_MAXHOST];

    memset(&sa, 0, sizeof(struct sockaddr_in));

    /* For IPv4*/
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = inet_addr("195.134.67.202");
    len = sizeof(struct sockaddr_in);

    if (getnameinfo((struct sockaddr *) &sa, len, hbuf, sizeof(hbuf), 
        NULL, 0, NI_NAMEREQD)) {
        printf("could not resolve hostname\n");
    }
    else {
        printf("host=%s\n", hbuf);
    }

    return 0;                                                  
}
