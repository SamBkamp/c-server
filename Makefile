FLAGS := -Wall -Wextra -ggdb
LIBS := -lcrypto -lssl -pthread

server:conn.c main.c parse.c
	gcc ${FLAGS} ${LIBS} main.c conn.c parse.c -o server
