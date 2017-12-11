ipdescd: ipdescd.c ipip.c ipip.h
	gcc -o ipdescd -Wall ipdescd.c ipip.c

docker:  ipdescd 17monipdb.dat
	gcc -static -o ipdescd -Wall ipdescd.c ipip.c
	docker build -t ipdesc .

indent: ipdescd.c ipip.c ipip.h
	indent ipdescd.c ipip.c ipip.h  -nbad -bap -nbc -bbo -hnl -br -brs -c33 -cd33 -ncdb -ce -ci4 \
		-cli0 -d0 -di1 -nfc1 -i8 -ip0 -l160 -lp -npcs -nprs -npsl -sai \
		-saf -saw -ncs -nsc -sob -nfca -cp33 -ss -ts8 -il1
