#include <netdb.h> 	
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void main(int argc, char **argv){
int 	i=0; 
char 	hostname[50], symbolicip[50];
struct 	hostent *mymachine; 
struct  in_addr **addr_list;

if (argc!=2 ) {printf("Usage: GetHostByName-p18 host-name\n"); exit(0);}

if ( (mymachine=gethostbyname(argv[1])) == NULL)
	printf("Could not resolved Name:  %s\n",argv[1]);
else	{
	printf("Name To Be  Resolved: %s\n", mymachine->h_name);
	printf("Name Length in Bytes: %d\n", mymachine->h_length);
	if ( mymachine->h_aliases != NULL) 
		for (i=0; mymachine->h_aliases[i]!=NULL; i++){
			printf("Alternative Name: %s\n",mymachine->h_aliases[i]);
			}
	addr_list = (struct in_addr **) mymachine->h_addr_list;
	for(i = 0; addr_list[i] != NULL; i++) {
        	strcpy(symbolicip , inet_ntoa(*addr_list[i]) );
		printf("%s resolved to %s \n",mymachine->h_name,symbolicip);
		}
	}
}
