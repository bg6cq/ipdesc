# IP地址信息查询服务

ipdesc 程序基于GPL v3发布。

本查询服务IPv4信息使用 http://ipip.net 提供的IP地址数据库免费版, 感谢北京天特信科技有限公司([http://ipip.net](http://ipip.net))。

IPv6信息使用 <a href='https://db-ip.com'>IP Geolocation by DB-IP</a> https://db-ip.com/db/download/ip-to-city-lite


特点：

* 仅仅一个进程，占用内存约9MB，启动后不再读写任何文件，每秒钟可以响应超1万次查询
* 使用epoll高效接口，单进程支持超1万并发连接（需要使用ulimit -n 10240设置单进程可打开的文件数）
* 启动完成后自动 chroot 并降权到 nobody（默认开启，可用 -u root 关闭），运行期无法访问文件系统，降低被攻陷后的提权风险

演示站点（请单击如下URL测试）：

* 帮助信息 [http://202.38.95.17:90/help](http://202.38.95.17:90/help)
* 查询IP信息 [http://202.38.95.17:90/202.38.64.1](http://202.38.95.17:90/202.38.64.1) IPv6: [http://[2001:da8:d800:95::17]:90/202.38.64.1](http://[2001:da8:d800:95::17]:90/202.38.64.1)

命令行：
```
Usage:
   ipdescd [ -d debug_level ] [ -f ] [ -6 ] [ -u user ] -m mmdbfilename [ tcp_port ]
        -d debug
        -f fork and do
        -6 support ipv6
        -u user: 绑定端口后 chroot 并 setuid 到该用户（默认 nobody；-u root 关闭沙箱）
        -m mmdbfilename
        default port is 80
```
注意：程序必须在包含 ipipfree.ipdb 与 mmdbfilename 的目录下启动（chroot 以此为根）；
默认会在启动后把自身限制在空 chroot 中并无权访问任何文件，如需旧行为可加 -u root。
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

./ipdescd -f -m dbip-city-lite-2020-02.mmdb 90
```

默认需要以root启动以绑定80端口（或使用 -u root 且以非root运行在1024以上端口）。
程序会在绑定端口后执行 chroot + setuid(nobody)，进入事件循环后不再有root权限、也不再能访问任何文件。

如果需要查看运行的调试输出，可以使用

```
./ipdescd -f -d 90 -m dbip-city-lite-2020-02.mmdb

```

上面的90是提供服务的tcp端口，访问 http://server_ip:90/x.x.x.x 即可返回x.x.x.x的地址信息

## 沙箱说明

程序启动流程：加载 ipipfree.ipdb 与 mmdb 数据库 → 绑定监听端口 → **chroot 到启动目录并降权到 nobody** → 进入事件循环。

* 数据库与共享内存、/dev/null、监听socket均在 chroot 前就绪，运行期不依赖任何文件，因此可以安全地限制在空 chroot 中。
* 启动后无法读写任何文件、无法 setuid/提权（能力已清零），即使查询逻辑被攻陷也无法访问宿主文件系统或执行特权操作。
* 必须以 root 启动才能进行 chroot/setuid；若以非 root 启动且未加 -u root，程序会报错退出（拒绝悄悄以当前用户继续运行）。
* 若要关闭沙箱（例如在容器内已由容器隔离保护、或需要以特权运行），使用 `-u root`。
* 若想降权到指定用户而非 nobody，使用 `-u 用户名`（该用户名必须存在）。

## 命令行批量查询

ipdesc 从标准输入读取一行一行的IP，输出地址信息
