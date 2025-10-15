FLAGS := -Wall -Wextra -ggdb
LIBS := -lcrypto -lssl

server:conn.c main.c parse.c
	gcc ${FLAGS} ${LIBS} main.c conn.c parse.c -o server
