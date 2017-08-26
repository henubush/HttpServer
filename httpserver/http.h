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

//得到http 请求中 GET后面的字符串
void get_http_command(char *http_msg, char *command);
//根据用户在GET中的请求，生成相应的回复内容
int make_http_content(const char *command, char **content);
//根据扩展名返回文件类型描述
const char *get_filetype(const char *filename) ;
// 得到文件内容
int  get_file_content(const char *file_name, char **content);


#endif