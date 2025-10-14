FLAGS := -Wall -Wextra
LIBS := -lcrypto -lssl

server:conn.c main.c
	gcc ${FLAGS} ${LIBS} main.c conn.c -o server 
