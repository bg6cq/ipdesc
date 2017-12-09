// 17monipdb.dat from ipip.net

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
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
#include <arpa/inet.h>
#include "ipip.h"

#define MAXEVENTS 64

#define MAXLEN 1024

int port = 80;

int debug = 0;

void Log(char *msg)
{
	printf("%s\n", msg);
	exit(1);
}

void respond(int cfd, char *mesg)
{
	char *p, buf[MAXLEN];
	int len;

	if (debug)
		printf("From Client(fd %d): %s##\n", cfd, mesg);
	p = mesg;
	if (memcmp(p, "GET /favicon.ico", 16) == 0)
		len = snprintf(buf, MAXLEN, "HTTP/1.0 404 OK\r\nConnection: close\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n");
	else if (*p == 'G' && *(p + 1) == 'E' && *(p + 2) == 'T' && *(p + 3) == ' ' && *(p + 4) == '/' && *(p + 5) >= '0' && *(p + 5) <= '9') {
		char result[128];
		find(p + 5, result, 128);
		len =
		    snprintf(buf, MAXLEN,
			     "HTTP/1.0 200 OK\r\nConnection: close\r\nContent-Type: text/html; charset=UTF-8\r\nServer: web server by james@ustc.edu.cn, data from ipip.net\r\n\r\n%s\r\n",
			     result);
	} else
		len = snprintf(buf, MAXLEN, "HTTP/1.0 200 OK\r\nConnection: close\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n%s\r\n",
			       "使用方式: /IP地址<br>IP地址数据库来自<a href=http://ipip.net>http://ipip.net</a>免费版，最后更新时间20170704<br>"
			       "感谢北京天特信科技有限公司<br>https://github.com/bg6cq/ipdesc<br>james@ustc.edu.cn 2017.12.09");
	if (debug)
		printf("Send to Client(fd %d): %s##\n", cfd, buf);
	write(cfd, buf, len);
}

void set_socket_non_blocking(int fd)
{
	int flags;
	flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0)
		Log("fcntl getfl error");
	flags |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) < 0)
		Log("fcntl setfl error");
}

void usage(void)
{
	printf("Usage:\n");
	printf("   ipdescd [ -d ] [ tcp_port ]\n");
	printf("        default port is 80\n");
	exit(0);
}

int main(int argc, char *argv[])
{
	int listenfd, efd;
	int enable = 1;
	struct epoll_event event, *events;
	static struct sockaddr_in serv_addr;

	int c;
	while ((c = getopt(argc, argv, "dh")) != EOF)
		switch (c) {
		case 'd':
			debug = 1;
			break;
		case 'h':
			usage();
		};
	if (optind == argc - 1)
		port = atoi(argv[optind]);
	if (port < 0 || port > 65535)
		Log("Invalid port number try [1,65535]");

	(void)signal(SIGCLD, SIG_IGN);
	(void)signal(SIGHUP, SIG_IGN);
	setvbuf(stdout, NULL, _IONBF, 0);

	printf("web server started at port: %d\n", port);

	if (init("17monipdb.dat") != 1)
		Log("init 17monipdb.dat error");
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		Log("socket error");
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
		Log("setsockopt(SO_REUSEADDR) error");
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);
	if (bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		Log("bind error");
	set_socket_non_blocking(listenfd);
	if (listen(listenfd, 64) < 0)
		Log("listen error");
	if ((efd = epoll_create1(0)) < 0)
		Log("epoll_create1 error");
	event.data.fd = listenfd;
	event.events = EPOLLIN | EPOLLET;
	if (epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &event) < 0)
		Log("epoll ctl_add of listenfd");
	/* Buffer where events are returned */
	events = calloc(MAXEVENTS, sizeof event);

	// Event Loop 
	while (1) {
		int n, i;
		n = epoll_wait(efd, events, MAXEVENTS, -1);
		for (i = 0; i < n; i++) {
			if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) {
				/* An error has occured on this fd, or the socket is not
				 * ready for reading (why were we notified then?) */
				printf("epoll error, unknow event\n");
				close(events[i].data.fd);
			} else if (listenfd == events[i].data.fd) {
				/* notification on the listening socket, which
				 * means one or more incoming connections. */
				while (1) {
					struct sockaddr in_addr;
					socklen_t in_len;
					int infd;
					in_len = sizeof in_addr;
					infd = accept(listenfd, &in_addr, &in_len);
					if (infd == -1) {
						if ((errno == EAGAIN) || (errno == EWOULDBLOCK))	/*  all incoming connections processed. */
							break;
						else
							Log("accept new client error");
					}
					if (debug) {
						char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
						if (getnameinfo(&in_addr, in_len, hbuf, sizeof hbuf, sbuf, sizeof sbuf, NI_NUMERICHOST | NI_NUMERICSERV) == 0) {
							printf("new connection on fd %d " "(host=%s, port=%s)\n", infd, hbuf, sbuf);
						}
					}

					/* set the incoming socket non-blocking and add it to the list of fds to monitor. */
					set_socket_non_blocking(infd);
					event.data.fd = infd;
					event.events = EPOLLIN | EPOLLET;
					if (epoll_ctl(efd, EPOLL_CTL_ADD, infd, &event) < 0)
						Log("epoll_ctl add new client error");
				}
			} else if (events[i].events & EPOLLIN) {
				/* new data on the fd waiting to be read.
				 *
				 * We only read the first packet, for normal http client, it's OK */
				ssize_t count;
				char buf[MAXLEN];

				count = read(events[i].data.fd, buf, MAXLEN - 1);
				if (count > 0) {
					buf[count] = 0;
					respond(events[i].data.fd, buf);
				}
				if (debug)
					printf("close fd %d\n", events[i].data.fd);
				close(events[i].data.fd);
			}
		}
	}
}
