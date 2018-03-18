// 17monipdb.datx from ipip.net

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include "ipip.h"

#define MAXLEN 1024

void test(char *ip)
{
	char buf[MAXLEN];
	find(ip, buf, 128);
	printf("IP %s: %s\n", ip, buf);
}

int main(int argc, char *argv[])
{
	if (init("17monipdb.datx") != 1) {
		printf("init 17monipdb.datx error");
		exit(-1);
	}
	test("255.255.255.255");
	test("8.8.8.8");
	test("202.38.63.255");
	test("202.38.64.0");
	test("202.38.95.255");
	test("202.38.96.0");
	printf("\ndo 500*10000 lookup test:\n");

	struct timeval st, end;
	float usems;
	gettimeofday(&st, NULL);
	uint32_t uip;
	char buf[MAXLEN];
	int i;
	for (i = 0; i < 5000000; i++) {
		uip = rand() << 2;
		find_u(uip, buf, 128);
	}
	gettimeofday(&end, NULL);
	usems = 1000.0 * (end.tv_sec - st.tv_sec) + (end.tv_usec - st.tv_usec) / 1000.0;
	printf("5M lookup time: %.2f(ms)\n", usems);
}
