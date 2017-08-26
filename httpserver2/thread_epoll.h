#ifndef _THREAD_EPOLL_H_
#define _THREAD_EPOLL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <netinet/in.h>
#include <assert.h>
#include <time.h>

int socket_create(int port);
int tcp_accept(int sockfd);
void addfd(int epollfd,int fd);
void delfd(int epollfd,int fd);
void *thread_routine(void *arg);
void get_ip_port(char *res,int conn);

#endif
