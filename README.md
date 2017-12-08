# IP地址信息查询服务

本查询服务使用 http://ipip.net 提供的IP地址数据库免费版

感谢北京天特信科技有限公司

程序有两种方式提供服务：

## 独立程序运行

```
cd /usr/src
git clone https://github.com/bg6cq/ipdesc
cd ipdesc
make
自行下载 http://ipip.net 的免费版 17monipdb.dat
./ipdesc 90
```
上面的90是提供服务的tcp端口，访问 http://server_ip/x.x.x.x 即可返回x.x.x.x的地址信息

## docker 服务运行
```
docker pull bg6cq/ipdesc
docker run -d -p 90:80 ipdesc
```
上面的90是提供服务的tcp端口，访问 http://server_ip/x.x.x.x 即可返回x.x.x.x的地址信息
