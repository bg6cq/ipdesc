all: ipdescd ipiptest

ipdescd: ipdescd.c
	gcc -o ipdescd -std=gnu99 -Wall ipdescd.c ipdb-c/ipdb.c -ljson-c -lmaxminddb

ipiptest: ipiptest.c
	gcc -o ipiptest -std=gnu99 -g -Wall ipiptest.c ipdb-c/ipdb.c -ljson-c
	./ipiptest

docker:  ipdescd 17monipdb.datx
	gcc -static -o ipdescd -Wall ipdescd.c ipip.c
	docker build -t ipdesc .

indent: ipdescd.c ipiptest.c
	indent ipdescd.c ipiptest.c -nbad -bap -nbc -bbo -hnl -br -brs -c33 -cd33 -ncdb -ce -ci4 \
		-cli0 -d0 -di1 -nfc1 -i8 -ip0 -l160 -lp -npcs -nprs -npsl -sai \
		-saf -saw -ncs -nsc -sob -nfca -cp33 -ss -ts8 -il1
