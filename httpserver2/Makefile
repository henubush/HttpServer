# Digital UNIX 4.0 compilation flags:
CFLAGS=-Wall -g -c $(DEBUGFLAGS)
RTFLAGS=-lrt
CC:=gcc
# Solaris 2.5 compilation flags:
#CFLAGS=-D_POSIX_C_SOURCE=199506 -D_REENTRANT -Xa -lpthread -g $(DEBUGFLAGS)
#RTFLAGS=-lposix4

SOURCES=log.c http.c thread_epoll.c  main.c
PROGRAMS=$(SOURCES:.c=.o)

EXEC = main
start:$(PROGRAMS)
	$(CC) -g -o  $(EXEC) $(PROGRAMS) -lpthread -lrt
http:
	${CC} ${CFLAGS} ${RTFLAGS}  http.c -lrt
thread_epoll:
	${CC} ${CFLAGS} ${RTFLAGS} http.o thread_epoll.c -lrt
log:
	${CC} ${CFLAGS} ${RTFLAGS} log.c -lrt
main:
	${CC} ${CFLAGS} ${RTFLAGS} main.c
	${CC} -g ${CFLAGS} ${RTFLAGS} log.o http.o thread_epoll.o main.o -o main -pthread -lrt 
.c.o:
	$(CC) -g -o $@ -c $<
clean:
	@rm -rf *.o main
recompile:	clean all

