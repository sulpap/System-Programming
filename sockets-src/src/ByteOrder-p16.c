#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

int main(){
	uint16_t nhost = 0xD04C, nnetwork;
	unsigned char *p;

	p=(unsigned char *)&nhost;
	printf("%x %x \n", *p, *(p+1));
	nnetwork=htons(nhost);
	p=(unsigned char *)&nnetwork;
	printf("%x %x \n", *p, *(p+1));
	exit(1);
}
