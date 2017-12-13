// 17monipdb.dat from ipip.net

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

#include "ipip.h"

#define MAXEVENTS 64

#define MAXLEN 1024

int port = 80;
int fork_and_do = 0;
int debug = 0;
int ipv6 = 0;

char *http_head =
    "HTTP/1.0 200 OK\r\nConnection: close\r\nContent-Type: text/html; charset=UTF-8\r\nServer: web server by james@ustc.edu.cn, data from ipip.net\r\n\r\n";

void respond(int cfd, char *mesg)
{
	char buf[MAXLEN], *p = mesg;
	char result[128];
	int len = 0;

	if (debug)
		printf("From Client(fd %d):\n%s##END\n", cfd, mesg);
	buf[0] = 0;
	if (memcmp(p, "GET /", 5) == 0) {
		if (memcmp(p + 5, "favicon.ico", 11) == 0)
			len = snprintf(buf, MAXLEN, "HTTP/1.0 404 OK\r\nConnection: close\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n");
		else if (*(p + 5) == ' ') {	//   GET /, show ip and desc
			struct sockaddr_storage in_addr;
			socklen_t in_len = sizeof(in_addr);
			char hbuf[INET6_ADDRSTRLEN];

			getpeername(cfd, (struct sockaddr *)&in_addr, &in_len);
			if (in_addr.ss_family == AF_INET6) {
				struct sockaddr_in6 *r = (struct sockaddr_in6 *)&in_addr;
				inet_ntop(AF_INET6, &r->sin6_addr, hbuf, sizeof(hbuf));
				len = snprintf(buf, MAXLEN, "%s%s IPv6\r\n", http_head, hbuf);
			} else if (in_addr.ss_family == AF_INET) {
				struct sockaddr_in *r = (struct sockaddr_in *)&in_addr;
				inet_ntop(AF_INET, &r->sin_addr, hbuf, sizeof(hbuf));
				find(hbuf, result, 128);
				len = snprintf(buf, MAXLEN, "%s%s %s\r\n", http_head, hbuf, result);
			}
		} else if (*(p + 5) >= '0' && *(p + 5) <= '9') {	// GET /IP, show ip desc
			find(p + 5, result, 128);
			if(result[0])
				len = snprintf(buf, MAXLEN, "%s%s\r\n", http_head, result);
			else
				len = snprintf(buf, MAXLEN, "%sNULL\r\n", http_head);
		} else
			len = snprintf(buf, MAXLEN,
				       "%s%s\r\n", http_head,
				       "使用方式: <br>http://serverip/ 显示本机IP地址和信息<br>http://serverip/IP地址 显示IP地址的信息<p>"
				       "IP地址数据库来自<a href=http://ipip.net>http://ipip.net</a>免费版，最后更新时间20170704<br>"
				       "感谢北京天特信科技有限公司<br>https://github.com/bg6cq/ipdesc<br>james@ustc.edu.cn 2017.12.09");
	}
	if (debug)
		printf("Send to Client(fd %d):\n%s##END\n", cfd, buf);
	write(cfd, buf, len);
}

int set_socket_non_blocking(int fd)
{
	int flags;
	flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0)
		return -1;
	flags |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) < 0)
		return -1;
	return 0;
}

void set_socket_keepalive(int fd)
{
	int keepalive = 1;	// 开启keepalive属性
	int keepidle = 5;	// 如该连接在60秒内没有任何数据往来,则进行探测
	int keepinterval = 5;	// 探测时发包的时间间隔为5 秒
	int keepcount = 3;	// 探测尝试的次数。如果第1次探测包就收到响应了,则后2次的不再发
	setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive, sizeof(keepalive));
	setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, (void *)&keepidle, sizeof(keepidle));
	setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, (void *)&keepinterval, sizeof(keepinterval));
	setsockopt(fd, SOL_TCP, TCP_KEEPCNT, (void *)&keepcount, sizeof(keepcount));
}

void usage(void)
{
	printf("Usage:\n");
	printf("   ipdescd [ -d ] [ -f ] [ -6 ] [ tcp_port ]\n");
	printf("        -d debug\n");
	printf("        -f fork and do\n");
	printf("        -6 support ipv6\n");
	printf("        default port is 80\n");
	exit(0);
}

int bind_and_listen(void)
{
	int listenfd;
	int enable = 1;

	if (ipv6)
		listenfd = socket(AF_INET6, SOCK_STREAM, 0);
	else
		listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd < 0) {
		perror("socket");
		exit(-1);
	}
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
		perror("setsockopt(SO_REUSEADDR)");
		exit(-1);
	}
	if (ipv6) {
		static struct sockaddr_in6 serv_addr6;
		memset(&serv_addr6, 0, sizeof(serv_addr6));
		serv_addr6.sin6_family = AF_INET6;
		serv_addr6.sin6_port = htons(port);
		if (bind(listenfd, (struct sockaddr *)&serv_addr6, sizeof(serv_addr6)) < 0) {
			perror("bind");
			exit(-1);
		}
	} else {
		static struct sockaddr_in serv_addr;
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		serv_addr.sin_port = htons(port);
		if (bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
			perror("bind");
			exit(-1);
		}
	}
	if (set_socket_non_blocking(listenfd) < 0) {
		perror("set_socket_non_blocking");
		exit(-1);
	}
	if (listen(listenfd, 64) < 0) {
		perror("listen");
		exit(-1);
	}
	return listenfd;
}

int main(int argc, char *argv[])
{
	int listenfd, efd;
	struct epoll_event event, *events;

	int c;
	while ((c = getopt(argc, argv, "df6h")) != EOF)
		switch (c) {
		case 'd':
			debug = 1;
			break;
		case 'f':
			fork_and_do = 1;
			break;
		case '6':
			ipv6 = 1;
			break;
		case 'h':
			usage();
		};
	if (optind == argc - 1)
		port = atoi(argv[optind]);
	if (port < 0 || port > 65535) {
		printf("Invalid port number %d, please try [1,65535]", port);
		exit(-1);
	}

	(void)signal(SIGCLD, SIG_IGN);
	(void)signal(SIGHUP, SIG_IGN);
	setvbuf(stdout, NULL, _IONBF, 0);

	if (fork_and_do) {
		if (debug)
			printf("I am parent, pid: %d\n", getpid());
		while (1) {
			int pid = fork();
			if (pid == 0)	// child do the job
				break;
			else {
				if (debug)
					printf("I am parent, waiting for child...\n");
				wait(NULL);
			}
			if (debug)
				printf("child exit? I will restart it.\n");
			sleep(2);
		}
		if (debug)
			printf("I am child, I am doing the job\n");
	}
	printf("web server started at port: %d, my pid: %d\n", port, getpid());

	if (init("17monipdb.dat") != 1) {
		printf("init 17monipdb.dat error");
		exit(-1);
	}
	listenfd = bind_and_listen();
	if ((efd = epoll_create1(0)) < 0) {
		perror("epoll_create1");
		exit(-1);
	}
	event.data.fd = listenfd;
	event.events = EPOLLIN | EPOLLET;
	if (epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &event) < 0) {
		perror("epoll_ctl_add of listenfd");
		exit(-1);
	}
	/* Buffer where events are returned */
	events = calloc(MAXEVENTS, sizeof event);
	if (events == NULL) {
		perror("calloc memory");
		exit(-1);
	}
	// Event Loop 
	while (1) {
		int n, i;
		n = epoll_wait(efd, events, MAXEVENTS, -1);
		for (i = 0; i < n; i++) {
			if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) {
				/* An error has occured on this fd, or the socket is not
				 * ready for reading (why were we notified then?) */
				printf("epollerr or epollhup event of fd %d\n", events[i].data.fd);
				close(events[i].data.fd);
				continue;
			}
			if (!(events[i].events & EPOLLIN)) {
				printf("unknow event of fd %d\n", events[i].data.fd);
				close(events[i].data.fd);
				continue;
			}
			if (listenfd == events[i].data.fd) {
				/* notification on the listening socket, which
				 * means one or more incoming connections. */
				while (1) {
					int infd;
					infd = accept(listenfd, NULL, 0);
					if (infd == -1) {
						if ((errno == EAGAIN) || (errno == EWOULDBLOCK))	/*  all incoming connections processed. */
							break;
						else {
							perror("accept new client");
							continue;
						}
					}
					if (debug) {
						struct sockaddr_storage in_addr;
						socklen_t in_len = sizeof(in_addr);
						char hbuf[INET6_ADDRSTRLEN];

						getpeername(infd, (struct sockaddr *)&in_addr, &in_len);
						if (in_addr.ss_family == AF_INET6) {
							struct sockaddr_in6 *r = (struct sockaddr_in6 *)&in_addr;
							inet_ntop(AF_INET6, &r->sin6_addr, hbuf, sizeof(hbuf));
							printf("new connection on fd %d " "(host=%s, port=%d)\n", infd, hbuf, ntohs(r->sin6_port));
						} else if (in_addr.ss_family == AF_INET) {
							struct sockaddr_in *r = (struct sockaddr_in *)&in_addr;
							inet_ntop(AF_INET, &r->sin_addr, hbuf, sizeof(hbuf));
							printf("new connection on fd %d " "(host=%s, port=%d)\n", infd, hbuf, ntohs(r->sin_port));
						}
					}

					/* set the incoming socket non-blocking and add it to the list of fds to monitor. */
					if (set_socket_non_blocking(infd) < 0) {
						perror("set_socket_non_blocking of new client");
						close(infd);
						continue;
					}
					set_socket_keepalive(infd);
					event.data.fd = infd;
					event.events = EPOLLIN | EPOLLET;
					if (epoll_ctl(efd, EPOLL_CTL_ADD, infd, &event) < 0) {
						perror("epoll_ctl_add new client");
						close(infd);
					}
				}
				continue;
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
					printf("close fd %d\n\n", events[i].data.fd);
				close(events[i].data.fd);
			}
		}
	}
}
