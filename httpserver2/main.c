#include "thread_epoll.h"
#include "log.h"
#include "http.h"

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
#include <mqueue.h>

char LOGBUF[10240];
struct tm *pt;
mqd_t mqdes;
char htmlpath[100];

int main()
{
    freopen("config.ini","r",stdin);

    int threads = 3;
    int port = 23333;
    scanf("port = %d\nthreads = %d\n",&port,&threads);
    scanf("htmlpath = %s\n",htmlpath);
    //printf("port = %d\nthreads = %d\nhtmlpath = %s\n",port,threads,htmlpath);

    int listenfd;
    mqdes =  mq_open("/abc",O_CREAT | O_RDWR,0666,NULL);
    if(mqdes == (mqd_t)-1)
    {
        pt = get_time();
        memset(LOGBUF,0,sizeof(LOGBUF));
        sprintf(LOGBUF,"Error:%02d:%02d:%02d : mq_open failed %s\n",
            pt->tm_hour,pt->tm_min,pt->tm_sec,strerror(errno));
        save_log(LOGBUF);
        exit(EXIT_FAILURE);
    }
    mqd_t *tmp = (mqd_t *) malloc(sizeof(mqd_t));
    *tmp = mqdes;
    pthread_t ptid;
    if((pthread_create(&ptid,NULL,log_thread,(void *)tmp)) < 0)
    {
        pt = get_time();
        memset(LOGBUF,0,sizeof(LOGBUF));
        sprintf(LOGBUF,"Error:%02d:%02d:%02d : log pthread_create failed %s\n",
            pt->tm_hour,pt->tm_min,pt->tm_sec,strerror(errno));
        save_log(LOGBUF);
        exit(EXIT_FAILURE);
    }

    listenfd = socket_create(port);

    int i,conn,epollfd[threads];
    pthread_t tid[threads];


    for (i = 0; i < threads; i++)
            epollfd[i] = epoll_create1(0);

	for(i=0;i<threads;i++)
	{
		if(pthread_create(&tid[i],NULL,thread_routine,(void *)&epollfd[i])<0)
        {
            pt = get_time();
            memset(LOGBUF,0,sizeof(LOGBUF));
            sprintf(LOGBUF,"Error:%02d:%02d:%02d : pthread_create failed %s\n", 
                pt->tm_hour,pt->tm_min,pt->tm_sec,strerror(errno));
        	mq_send(mqdes,LOGBUF,strlen(LOGBUF),1);
            exit(EXIT_FAILURE);
        }
	}
	while (1)
    {
        for (i = 0; i < threads; i++)
        {
            conn = tcp_accept(listenfd);
            addfd(epollfd[i],conn);
        }

    }
}

