#include "log.h"

pthread_mutex_t mqd_mutex;

void  *log_thread(void *mqid)
{

	static int year=0,month =0,day = 0;
	static int filecnt = 1;

	struct mq_attr attr;
	ssize_t nr;
	char *buf;
	mqd_t mqdes = *((mqd_t *) mqid);
	free((mqd_t *)mqid);
	mq_getattr(mqdes, &attr);
	buf =malloc(attr.mq_msgsize);

	while(1)
	{
		struct tm *pt;
	    pt=get_time();
	    if(year!=pt->tm_year+1900 || month!=pt->tm_mon || day!=pt->tm_mday)
	    {
	    	year=pt->tm_year+1900;
	    	month=pt->tm_mon;
	    	day=pt->tm_mday;
	    	filecnt = 1;
	    }
		char name[1024];
		sprintf(name,"%04d%02d%02d_%02dlog.txt",year,month,day,filecnt);

		FILE *fp = fopen(name,"a+");
		fseek(fp, 0L, SEEK_END);  
	    int filesize = ftell(fp); 
	    if(filesize >= 1024*1024*50)//50MB
	    {
	    	fclose(fp); 
	    	filecnt++;
	    	sprintf(name,"%04d%02d%02d_%02dlog.txt",year,month,day,filecnt);
	    	fp = fopen(name,"a+");
	    }
		
		mq_getattr(mqdes, &attr);
		int cnt = attr.mq_curmsgs;
		while(cnt--)
		{ 
			memset(buf,0,sizeof(buf));
           	int len = mq_receive(mqdes, buf, attr.mq_msgsize, NULL);
           	buf[len] = '\0';
           	fputs(buf,fp);  
		}
		fclose(fp); 
	}
	free(buf);
}


void save_log(char *buf)
{
	static int year=0,month =0,day = 0;
	static int filecnt = 1;
    struct tm *pt;
    pt=get_time();
    if(year!=pt->tm_year+1900 || month!=pt->tm_mon || day!=pt->tm_mday)
    {
    	year=pt->tm_year+1900;
    	month=pt->tm_mon;
    	day=pt->tm_mday;
    	filecnt = 1;
    }

	char name[1024];
	sprintf(name,"%04d%02d%02d_%02dlog.txt",year,month,day,filecnt);

	FILE *fp = fopen(name,"a+");
	fseek(fp, 0L, SEEK_END);  
    int filesize = ftell(fp); 
    if(filesize >= 1024*1024*50)
    {
    	fclose(fp); 
    	filecnt++;
    	sprintf(name,"%04d%02d%02d_%02dlog.txt",year,month,day,filecnt);
    	fp = fopen(name,"a+");
    }
	fputs(buf,fp);
    free(buf);  
}

struct tm* get_time()
{
	time_t t;
	struct tm *pt;
	time(&t);
	pt=localtime(&t);
	return pt;
}