FROM scratch
ADD 17monipdb.dat /
ADD ipdescd /
CMD ["/ipdescd","-f"]

EXPOSE 80

LABEL maintainer="james@ustc.edu.cn"
LABEL ipip.net="20180101"
