// 17monipdb.dat from ipip.net

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "ipip.h"

#define MAXLEN 1024

int port = 80;

int debug = 0;

void Log(char *msg)
{
	printf("%s\n", msg);
	exit(1);
}

void respond(int cfd)
{
	char *p, mesg[MAXLEN], buf[MAXLEN];
	int len;

	len = recv(cfd, mesg, MAXLEN - 1, 0);
	if (len <= 0)		// receive error
		Log("recv() error");
	mesg[len] = 0;
	if (debug)
		printf("%s", mesg);
	p = mesg;
	if (*p == 'G' && *(p + 1) == 'E' && *(p + 2) == 'T' && *(p + 3) == ' ' && *(p + 4) == '/' && *(p + 5) >= '0' && *(p + 5) <= '9') {
		char result[128];
		find(p + 5, result, 128);
		len =
		    snprintf(buf, MAXLEN,
			     "HTTP/1.0 200 OK\r\nConnection: close\r\nContent-Type: text/html; charset=UTF-8\r\nServer: web server by james@ustc.edu.cn, data from ipip.net\r\n\r\n%s\r\n",
			     result);
	} else
		len = snprintf(buf, MAXLEN, "HTTP/1.0 200 OK\r\nConnection: close\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n%s\r\n",
			       "使用方式: /IP地址<br>IP地址数据库来自<a href=http://ipip.net>http://ipip.net</a>免费版，最后更新时间20170704<br>"
			       "感谢北京天特信科技有限公司");
	if (debug)
		printf("%s", buf);
	write(cfd, buf, len);
	close(cfd);
	exit(0);
}

int main(int argc, char *argv[])
{
	int listenfd;
	int enable = 1;
	static struct sockaddr_in serv_addr;

	if (argc > 1)
		port = atoi(argv[1]);
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
		Log("setsockopt(SO_REUSEADDR) failed");
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);
	if (bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		Log("bind error");
	if (listen(listenfd, 64) < 0)
		Log("listen error");

	// ACCEPT connections
	while (1) {
		struct sockaddr_in clientaddr;
		socklen_t addrlen;
		int cfd;
		addrlen = sizeof(clientaddr);
		cfd = accept(listenfd, (struct sockaddr *)&clientaddr, &addrlen);

		if (cfd >= 0) {
			int pid;
			pid = fork();
			if (pid == 0)
				respond(cfd);
			if (pid > 0) {
				if (debug)
					printf("parent close client fd\n");
				close(cfd);
			}
		} else if (debug)
			printf("accept return < 0\n");
	}
}
