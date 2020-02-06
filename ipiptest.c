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

#include "ipdb-c/ipdb.h"

#define MAXLEN 1024

ipdb_reader *reader;

void test(char *ip)
{
	char body[512];
	char tmp[64];
	char *lang[2];
	lang[0] = "CN";
	lang[1] = "EN";
	printf("IP %s\n", ip);
	for (int i = 0; i < 2; ++i) {
		int err = ipdb_reader_find(reader, ip, lang[i], body);
		printf("%s find err: %d\n", lang[i], err);
		if (err)
			continue;
		printf("%s\n", body);
		int f = 0, p1 = 0, p2 = -1;
		do {
			if (*(body + p1) == '\t' || !*(body + p1)) {
				strncpy(tmp, body + p2 + 1, (size_t) p1 - p2);
				tmp[p1 - p2] = 0;
				printf("%d: %s: %s\n", f + 1, reader->meta->fields[f], tmp);
				p2 = p1;
				++f;
			}
		} while (*(body + p1++));
	}
}

int main(int argc, char *argv[])
{
	int err = ipdb_reader_new("ipipfree.ipdb", &reader);
	printf("new ipdb reader err: %d\n", err);
	if (err)
		exit(-1);

	printf("ipdb build time: %li\n", reader->meta->build_time);
	printf("ipdb ipv4 support: %i\n", ipdb_reader_is_ipv4_support(reader));
	printf("ipdb ipv6 support: %i\n", ipdb_reader_is_ipv6_support(reader));
	printf("ipdb language: ");
	for (int i = 0; i < reader->meta->language_length; ++i) {
		printf("%s ", reader->meta->language[i].name);
	}
	printf("\n");
	printf("ipdb fields: ");
	for (int i = 0; i < reader->meta->fields_length; ++i) {
		printf("%s ", reader->meta->fields[i]);
	}
	printf("\n");

	test("255.255.255.255");
	test("8.8.8.8");
	test("8.8.8.256");
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
		char body[512];
		uip = rand() << 2;
		sprintf(buf, "%u.%u.%u.%u", (uip >> 24) & 0xff, (uip >> 16) & 0xff, (uip >> 8) & 0xff, uip & 0xff);
		ipdb_reader_find(reader, buf, "CN", body);
	}
	gettimeofday(&end, NULL);
	usems = 1000.0 * (end.tv_sec - st.tv_sec) + (end.tv_usec - st.tv_usec) / 1000.0;
	printf("5M lookup time: %.2f(ms)\n", usems);

	ipdb_reader_free(&reader);
	return 0;
}
