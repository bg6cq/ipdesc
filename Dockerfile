FROM scratch
ADD ipdescd /
ADD 17monipdb.dat /
CMD ["/ipdescd"]

EXPOSE 80

LABEL maintainer="james@ustc.edu.cn"
