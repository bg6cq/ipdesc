all: ipdescd ipiptest

ipdescd: ipdescd.c ipip.c ipip.h
	gcc -o ipdescd -Wall ipdescd.c ipip.c

ipiptest: ipiptest.c ipip.c ipip.h
	gcc -o ipiptest -g -Wall ipiptest.c ipip.c
	./ipiptest

docker:  ipdescd 17monipdb.datx
	gcc -static -o ipdescd -Wall ipdescd.c ipip.c
	docker build -t ipdesc .

indent: ipdescd.c ipip.c ipip.h ipiptest.c
	indent ipdescd.c ipip.c ipip.h ipiptest.c -nbad -bap -nbc -bbo -hnl -br -brs -c33 -cd33 -ncdb -ce -ci4 \
		-cli0 -d0 -di1 -nfc1 -i8 -ip0 -l160 -lp -npcs -nprs -npsl -sai \
		-saf -saw -ncs -nsc -sob -nfca -cp33 -ss -ts8 -il1
