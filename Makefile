ipdescd: ipdescd.c ipip.c ipip.h
	gcc -static -o ipdescd -Wall ipdescd.c ipip.c

docker:  ipdescd 17monipdb.dat
	docker build -t ipdesc .
