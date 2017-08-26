#include "thread_epoll.h"
#include "http.h"
#include "log.h"

#define MAX_PTHREAD 10
#define MAX_EVENT_SIZE 10240

extern char LOGBUF[10240];
extern struct tm *pt;
extern mqd_t mqdes;
extern char htmlpath[100];

void *thread_routine(void *arg)
{
	int epollffd = *(int *)arg;
	struct epoll_event events[MAX_EVENT_SIZE];
	while(1)
	{
		int nready = epoll_wait( epollffd, events, MAX_EVENT_SIZE, -1);
		if(nready == -1)
		{
			if(errno == EINTR)//中断
				continue;
			pt = get_time();
	        memset(LOGBUF,0,sizeof(LOGBUF));
	        sprintf(LOGBUF,"Warning:%02d:%02d:%02d : %s,%d epoll_wait error %s\n",
	            pt->tm_hour,pt->tm_min,pt->tm_sec, __FILE__, __LINE__, strerror(errno));
        	mq_send(mqdes,LOGBUF,strlen(LOGBUF),1);
	        //exit(EXIT_FAILURE);
	        continue;
		}
		if(nready==0)
			continue;
		int conn;
		for (int i = 0;i < nready;i++ )
		{
			int sockfd = events[i].data.fd;
			if(events[i].events & EPOLLIN)
			{
				char buf[1024];
				memset(buf, 0, sizeof(buf));
				int rc = recv(sockfd, buf, sizeof(buf)-1, 0);
				if(rc < 0)
				{
					pt = get_time();
			        memset(LOGBUF,0,sizeof(LOGBUF));
			        sprintf(LOGBUF,"Warning:%02d:%02d:%02d : %s,%d recv error %s\n",
			            pt->tm_hour,pt->tm_min,pt->tm_sec, __FILE__, __LINE__, strerror(errno));
		        	mq_send(mqdes,LOGBUF,strlen(LOGBUF),0);
					
					close(sockfd);
					delfd(epollffd,sockfd);
					continue;
				}
				else if(rc == 0)
				{
			        pt = get_time();
			    	char ip_port[20];
				    get_ip_port((char *)&ip_port,sockfd);
					memset(LOGBUF,0,sizeof(LOGBUF));
					sprintf(LOGBUF,"Info:%02d:%02d:%02d : %s,disconnect %s\n", 
						pt->tm_hour,pt->tm_min,pt->tm_sec,ip_port,strerror(errno));
				    mq_send(mqdes,LOGBUF,strlen(LOGBUF),2);

					close(sockfd);
					delfd(epollffd,sockfd);
					continue;
				}
				/*char command[1024];
				memset(command, 0, sizeof(command));
				get_http_command(buf, command); //得到http 请求中 GET后面的字符串*/

		        /*pt = get_time();
			    char ip_port[20];
			    get_ip_port(&ip_port,sockfd);
				memset(LOGBUF,0,sizeof(LOGBUF));
				sprintf(LOGBUF,"Info:%02d:%02d:%02d : %s,Get %s %s\n", 
					pt->tm_hour,pt->tm_min,pt->tm_sec,ip_port,command,strerror(errno));
			    mq_send(mqdes,LOGBUF,strlen(LOGBUF),2);*/

				/*if(command[0] == 0)
					sprintf(command,"index.html");

				int flag = 0;
				if(strlen(command)>=3)
				{
					for(int i=0;i<strlen(command)-3;i++)
					{
						if(command[i]=='.'&&command[i+1]=='.'&&command[i+2]=='/')
							flag = 1;
					}
				}
				if(flag == 1)
				{
					sprintf(command,"404.html");
				}
				//printf("%s\n",command);
				char path[1024];
				sprintf(path,"%s%s",htmlpath,command);
				
				char *content = NULL;
				int ilen = make_http_content(path, &content); //根据用户在GET中的请求，生成相应的回复内容
				if (ilen > 0)*/
				{
					sprintf(buf,"HTTP/1.0 200 OK\nConnection: Keep-Alive\nContent-Type: text/html\nTransfer-Encoding: chunked\nAccept-Ranges:bytes\nContent-Length:91\n\n<html><head><title>test page</title></head><body>Hello world!</body></html>\n\n");
					int ilen = strlen(buf);
					rc = send(sockfd, buf, ilen, 0); //将回复的内容发送给client端socket
					if(rc<0)
					{
						pt = get_time();
			        	memset(LOGBUF,0,sizeof(LOGBUF));
			        	sprintf(LOGBUF,"Warning:%02d:%02d:%02d : %s,%d send error %s\n",
			            	pt->tm_hour,pt->tm_min,pt->tm_sec, __FILE__, __LINE__, strerror(errno));
			        	mq_send(mqdes,LOGBUF,strlen(LOGBUF),0);

						close(sockfd);
						delfd(epollffd,sockfd);	
					}
				}
				//free(content);
			}
		}
	}
	return NULL;
}

int socket_create(int port)
{
	struct sockaddr_in servaddr;
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	int listenfd;
	if((listenfd = socket(PF_INET,SOCK_STREAM,0)) < 0)
	{
		pt = get_time();
        memset(LOGBUF,0,sizeof(LOGBUF));
        sprintf(LOGBUF,"Error:%02d:%02d:%02d : %s,%d socket error %s\n",
            pt->tm_hour,pt->tm_min,pt->tm_sec, __FILE__, __LINE__, strerror(errno));
        mq_send(mqdes,LOGBUF,strlen(LOGBUF),1);
        exit(EXIT_FAILURE);
	}
	int on=1;
	if((setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on))) < 0)
	{
		pt = get_time();
		memset(LOGBUF,0,sizeof(LOGBUF));
		sprintf(LOGBUF,"Error:%02d:%02d:%02d : %s,%d setsockopt failed %s\n", 
			pt->tm_hour,pt->tm_min,pt->tm_sec, __FILE__, __LINE__, strerror(errno));
        mq_send(mqdes,LOGBUF,strlen(LOGBUF),1);
		exit(EXIT_FAILURE);
	}
	if((bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr))) < 0)
	{
		pt = get_time();
		memset(LOGBUF,0,sizeof(LOGBUF));
		sprintf(LOGBUF,"Error:%02d:%02d:%02d : %s,%d bind failed %s\n", 
			pt->tm_hour,pt->tm_min,pt->tm_sec, __FILE__, __LINE__, strerror(errno));
        mq_send(mqdes,LOGBUF,strlen(LOGBUF),1);
		exit(EXIT_FAILURE);
	}
	if((listen(listenfd,SOMAXCONN)) < 0)
	{
		pt = get_time();
		memset(LOGBUF,0,sizeof(LOGBUF));
		sprintf(LOGBUF,"Error:%02d:%02d:%02d : %s,%d listen failed %s\n", 
			pt->tm_hour,pt->tm_min,pt->tm_sec, __FILE__, __LINE__, strerror(errno));
        mq_send(mqdes,LOGBUF,strlen(LOGBUF),1);
		exit(EXIT_FAILURE);
	}
	return listenfd;
}

int tcp_accept(int sockfd)
{

    struct sockaddr_in client_addr;
    socklen_t length = sizeof(client_addr);
    int connect;

    if((connect = accept(sockfd, (struct sockaddr *) &client_addr, &length)) < 0)
    {
        pt = get_time();
		memset(LOGBUF,0,sizeof(LOGBUF));
		sprintf(LOGBUF,"Error:%02d:%02d:%02d : %s,%d accept failed %s\n", 
			pt->tm_hour,pt->tm_min,pt->tm_sec, __FILE__, __LINE__, strerror(errno));
        mq_send(mqdes,LOGBUF,strlen(LOGBUF),1);
		//exit(EXIT_FAILURE);
    }
    pt = get_time();
    char ip_port[20];
    get_ip_port((char *)&ip_port,connect);
//    printf("%s\n",ip_port );
	memset(LOGBUF,0,sizeof(LOGBUF));
	sprintf(LOGBUF,"Info:%02d:%02d:%02d : %s,accept %s\n", 
		pt->tm_hour,pt->tm_min,pt->tm_sec,ip_port, strerror(errno));
    mq_send(mqdes,LOGBUF,strlen(LOGBUF),2);

    return connect;
}

void addfd( int epollfd, int fd )
{
	struct epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN;
	epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &event );
}
void delfd(int epollfd,int fd)
{
    struct epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN ;
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd,  &event);
}
void get_ip_port(char *res,int conn)
{
    char guest_ip[20];
    struct sockaddr_in guest;
    socklen_t guest_len = sizeof(guest);
    getpeername(conn, (struct sockaddr *)&guest, &guest_len);
    inet_ntop(AF_INET, &guest.sin_addr, guest_ip, sizeof(guest_ip));
    sprintf(res,"%s:%d", inet_ntoa(guest.sin_addr) , ntohs(guest.sin_port));
}
