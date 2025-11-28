// ipipfree.ipdb from ipip.net

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include "ipdb-c/ipdb.h"
#include "maxminddb.h"


#define MAXLEN 1024

int debug = 0;
int ipv6 = 0;
char mmdbfilename[MAXLEN];

#include "shmcount.c"

ipdb_reader *reader;		// ipip.net ipdb
MMDB_s mmdb;			// db-ip mmdb

void find(char *ip, char *result, int len)
{
	char *p;
	inc_lookup_count();
	if (debug >= 2)
		printf("find: %s\n", ip);
	p = ip;
	while (*p && ((*p == '.') || (*p == ':')
		      || (*p >= '0' && *p <= '9')
		      || (*p >= 'a' && *p <= 'z')
		      || (*p >= 'A' && *p <= 'Z')
		      || (*p >= 'A' && *p <= 'Z')))
		p++;
	*p = 0;
	if (debug >= 2)
		printf("find: %s\n", ip);
	int err = ipdb_reader_find(reader, ip, "CN", result);
	if (err == 0)
		return;

	result[0] = 0;

// ipip.net not found, using db-ip, for ipv6

	int gai_error, mmdb_error;
	MMDB_lookup_result_s mmdbresult = MMDB_lookup_string(&mmdb, ip, &gai_error, &mmdb_error);

	if (gai_error != 0) {
		if (debug >= 2)
			printf("Error from getaddrinfo for %s - %s\n", ip, gai_strerror(gai_error));
		return;
	}

	if (MMDB_SUCCESS != mmdb_error) {
		if (debug >= 2)
			printf("Got an error from libmaxminddb: %s\n\n", MMDB_strerror(mmdb_error));
		return;
	}

	MMDB_entry_data_s entry_data;
	char country[MAXLEN];
	char subdivision[MAXLEN];
	char city[MAXLEN];
	strcpy(country, "未知");
	strcpy(subdivision, "未知");
	strcpy(city, "未知");
//step 1, get country

	int status = MMDB_get_value(&mmdbresult.entry, &entry_data,
				    "country", "names", "zh-CN", NULL);
	if (status != MMDB_SUCCESS)
		status = MMDB_get_value(&mmdbresult.entry, &entry_data, "country", "names", "en", NULL);

	if ((status == MMDB_SUCCESS) && entry_data.has_data) {
		if (entry_data.type == MMDB_DATA_TYPE_UTF8_STRING) {
			int n;
			n = entry_data.data_size > MAXLEN - 1 ? MAXLEN - 1 : entry_data.data_size;
			memcpy(country, entry_data.utf8_string, n);
			country[n] = 0;
		}
	}
// step 2. get subdivisions
	status = MMDB_get_value(&mmdbresult.entry, &entry_data, "subdivisions", "names", "zh-CN", NULL);
	if (status != MMDB_SUCCESS)
		status = MMDB_get_value(&mmdbresult.entry, &entry_data, "subdivisions", "0", "names", "en", NULL);
	if ((status == MMDB_SUCCESS) && entry_data.has_data) {
		if (entry_data.type == MMDB_DATA_TYPE_UTF8_STRING) {
			int n;
			n = entry_data.data_size > MAXLEN - 1 ? MAXLEN - 1 : entry_data.data_size;
			memcpy(subdivision, entry_data.utf8_string, n);
			subdivision[n] = 0;
		}
	}
// step3. city
	status = MMDB_get_value(&mmdbresult.entry, &entry_data, "city", "names", "zh-CN", NULL);
	if (status != MMDB_SUCCESS)
		status = MMDB_get_value(&mmdbresult.entry, &entry_data, "city", "names", "en", NULL);
	if ((status == MMDB_SUCCESS) && entry_data.has_data) {
		if (entry_data.type == MMDB_DATA_TYPE_UTF8_STRING) {
			int n;
			n = entry_data.data_size > MAXLEN - 1 ? MAXLEN - 1 : entry_data.data_size;
			memcpy(city, entry_data.utf8_string, n);
			city[n] = 0;
		}
	}
	sprintf(result, "%s\t\%s\t%s", country, subdivision, city);
	return;
}

void usage(void)
{
	printf("Usage:\n");
	printf("   ipdesc [ -d debug_level ] -m mmdbfile_name\n");
	printf("        -d debug, level 1: print socket op, 2: print msg\n");
	printf("        -m mmdbfile_name\n");
	exit(0);
}

int main(int argc, char *argv[])
{
	int c;
	char buf[MAXLEN], result[MAXLEN];
	while ((c = getopt(argc, argv, "d:m:h")) != EOF)
		switch (c) {
		case 'd':
			debug = atoi(optarg);;
			break;
		case 'm':
			strncpy(mmdbfilename, optarg, MAXLEN - 1);
			break;
		case 'h':
			usage();

		};
	(void)signal(SIGCLD, SIG_IGN);
	(void)signal(SIGHUP, SIG_IGN);
	setvbuf(stdout, NULL, _IONBF, 0);

	initshm(1);

	int err = ipdb_reader_new("ipipfree.ipdb", &reader);
	if (err) {
		printf("ipdb_reader_new ipipfree.ipdb error %d\n", err);
		exit(-1);
	}

	int status = MMDB_open(mmdbfilename, MMDB_MODE_MMAP, &mmdb);

	if (status != MMDB_SUCCESS) {
		printf("Can't open %s - %s\n", mmdbfilename, MMDB_strerror(status));

		if (MMDB_IO_ERROR == status) {
			printf("    IO error: %s\n", strerror(errno));
		}
		exit(-1);
	}
	while (fgets(buf, MAXLEN, stdin)) {
		if (buf[strlen(buf) - 1] == '\n')
			buf[strlen(buf) - 1] = 0;
		find(buf, result, 128);
		if (result[0])
			printf("%s	%s\n", buf, result);
		else
			printf("%s	未知	未知	未知\n", buf);
	}
}
