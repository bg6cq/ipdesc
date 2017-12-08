// 17monipdb from ipip.net

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

#define ERROR 1
#define LOG   2

int port=80;

void Log(char *msg)
{
	printf("%s\n",msg);
	exit(1);
}

//client connection
void respond(int cfd)
{
	char *p, mesg[MAXLEN], buf[MAXLEN];
	int len;

	len=recv(cfd, mesg, MAXLEN-1, 0);

	if (len<=0)    // receive error
		Log("recv() error");
	mesg[len]=0;
//	printf("%s", mesg);
	p=mesg;
	if(*p=='G' && *(p+1)=='E' && *(p+2)=='T' && *(p+3)==' ' && *(p+4)=='/' && *(p+5)>='0' && *(p+5)<='9')  {
    		char result[128];
    		find(p+5, result, 128);
    		len = snprintf(buf,MAXLEN,"HTTP/1.0 200 OK\r\nConnection: close\r\nContent-Type: text/html; charset=UTF-8\r\nServer: web server by james@ustc.edu.cn, data from ipip.net\r\n\r\n%s\r\n",result);
//		printf("%s",buf);
		write (cfd, buf, len);
	} else {
    		len = snprintf(buf,MAXLEN,"HTTP/1.0 200 OK\r\nConnection: close\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n%s\r\n",
		"使用方式: /地址<br>IP地址数据库来自http://ipip.net免费版，最后更新时间20170704<br>"
		"感谢北京天特信科技有限公司");
		write (cfd, buf, len);
	}
	close(cfd);
}


int main(int argc, char* argv[])
{
	int listenfd;
	static struct sockaddr_in serv_addr;
	if(argc>1) 
		port = atoi(argv[1]);
	if(port < 0 || port >60000)
		Log("Invalid port number try [1,60000]");
	(void)signal(SIGCLD, SIG_IGN); 
	(void)signal(SIGHUP, SIG_IGN); 
	printf("Server started at port: %d\n", port);
    	init("17monipdb.dat");
	if((listenfd = socket(AF_INET, SOCK_STREAM,0)) <0)
		Log("system call socket");

	int enable = 1;
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
		    Log("setsockopt(SO_REUSEADDR) failed");
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);
	if(bind(listenfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr)) <0)
		Log("system call bind");
	if(listen(listenfd,64) <0)
		Log("system call listen");

	// ACCEPT connections
	while (1)
	{
		struct sockaddr_in clientaddr;
		socklen_t addrlen;
		int cfd;
		addrlen = sizeof(clientaddr);
		cfd = accept (listenfd, (struct sockaddr *) &clientaddr, &addrlen);

		if (cfd>=0)
		{	int pid;
			pid = fork();
			if ( pid==0 )
			{
				respond(cfd);
				exit(0);
			}
			else close(cfd);
		}
	}

	return 0;
}

