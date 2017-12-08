FROM scratch
ADD ipdescd 17monipdb.dat /
CMD ["/ipdescd"]

EXPOSE 80

LABEL maintainer="james@ustc.edu.cn"
LABEL ipip.net="20170704"
