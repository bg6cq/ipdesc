# IP地址信息查询服务

ipdesc 程序基于GPL v3发布。

本查询服务IPv4信息使用 http://ipip.net 提供的IP地址数据库免费版, 感谢北京天特信科技有限公司([http://ipip.net](http://ipip.net))。

IPv6信息使用 <a href='https://db-ip.com'>IP Geolocation by DB-IP</a> https://db-ip.com/db/download/ip-to-city-lite


特点：

* 仅仅一个进程，占用内存约9MB，启动后不再读写任何文件，每秒钟可以响应超1万次查询
* 使用epoll高效接口，单进程支持超1万并发连接（需要使用ulimit -n 10240设置单进程可打开的文件数）

演示站点（请单击如下URL测试）：

* 帮助信息 [http://202.38.95.17:90/help](http://202.38.95.17:90/help)
* 查询IP信息 [http://202.38.95.17:90/202.38.64.1](http://202.38.95.17:90/202.38.64.1) IPv6: [http://[2001:da8:d800:95::17]:90/202.38.64.1](http://[2001:da8:d800:95::17]:90/202.38.64.1)

命令行：
```
Usage:
   ipdescd [ -d ] [ -f ] [ -6 ] -m mmdbfilename [ tcp_port ]
        -d debug
        -f fork and do
        -6 support ipv6
	-m mmdbfilename
        default port is 80
```
程序有两种方式提供服务：

## docker 服务运行

```
docker pull bg6cq/ipdesc
docker run -d -p 90:80 --name ipdesc bg6cq/ipdesc
```
如果需要查看运行的调试输出，可以使用
```
docker run -it -p 90:80 --name ipdesc bg6cq/ipdesc /ipdescd -f -d
```

上面的90是提供服务的tcp端口，访问 http://server_ip:90/x.x.x.x 即可返回x.x.x.x的地址信息

不带x.x.x.x参数，会显示简单帮助和致谢信息。

更多关于docker信息，请参见 [https://hub.docker.com/r/bg6cq/ipdesc/](https://hub.docker.com/r/bg6cq/ipdesc/)

## 独立进程运行

```
cd /usr/src
git clone https://github.com/bg6cq/ipdesc
cd ipdesc
make

自行下载 https://www.ipip.net/product/client.html 的免费版 ipipfree.ipdb 以及 https://db-ip.com/db/download/ip-to-city-lite 到当前目录

./ipdescd -f 90 -m dbip-city-lite-2020-02.mmdb
```

如果需要查看运行的调试输出，可以使用

```
./ipdescd -f -d 90 -m dbip-city-lite-2020-02.mmdb

```

上面的90是提供服务的tcp端口，访问 http://server_ip:90/x.x.x.x 即可返回x.x.x.x的地址信息


## 命令行批量查询

ipdesc 从标准输入读取一行一行的IP，输出地址信息
