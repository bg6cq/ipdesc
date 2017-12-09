# IP地址信息查询服务

ipdesc 程序基于GPL v3发布。

本查询服务使用 http://ipip.net 提供的IP地址数据库免费版,

感谢北京天特信科技有限公司

特点：
* 仅仅一个进程，占用内存约20MB，启动后不再读写任何文件，每秒钟可以响应超1万次查询
* 使用epoll高效接口，单进程支持超1万并发连接（需要使用ulimit -n 10240设置单进程可打开的文件数）

演示站点（请单击如下URL测试）：

* 帮助信息 [http://210.45.224.10:90/](http://210.45.224.10:90/)
* 查询演示 [http://210.45.224.10:90/202.38.64.1](http://210.45.224.10:90/202.38.64.1)

程序有两种方式提供服务：

## docker 服务运行

```
docker pull bg6cq/ipdesc
docker run -d -p 90:80 --name ipdesc bg6cq/ipdesc
```

上面的90是提供服务的tcp端口，访问 http://server_ip:90/x.x.x.x 即可返回x.x.x.x的地址信息

不带x.x.x.x参数，会显示简单帮助和致谢信息。

更多关于docker信息，请参见 [https://hub.docker.com/r/bg6cq/ipdesc/](https://hub.docker.com/r/bg6cq/ipdesc/)

## 独立进程运行

独立进程运行是最快的方式，由于查询的网络开销占比多，独立进程比docker方式快不少。

```
cd /usr/src
git clone https://github.com/bg6cq/ipdesc
cd ipdesc
make
自行下载 http://ipip.net 的免费版 17monipdb.dat
./ipdesc 90
```
上面的90是提供服务的tcp端口，访问 http://server_ip:90/x.x.x.x 即可返回x.x.x.x的地址信息

