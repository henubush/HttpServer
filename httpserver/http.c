#include "http.h"
#include "log.h"

//http 消息头
#define HEAD "HTTP/1.0 200 OK\n\
Connection: Keep-Alive\n\
Content-Type: %s\n\
Transfer-Encoding: chunked\n\
Accept-Ranges:bytes\n\
Content-Length:%d\n\n"
//http 消息尾
#define TAIL "\n\n"

extern char LOGBUF[10240];
extern struct tm *pt;
extern mqd_t mqdes;
extern pthread_mutex_t mqd_mutex;

//得到http 请求中 GET后面的字符串
void get_http_command(char *http_msg, char *command)
{
	char *p_end = http_msg;
	char *p_start = http_msg;
	while (*p_start) //GET /
	{
		if (*p_start == '/')
		{
			break;
		}
		p_start++;
	}
	p_start++;
	p_end = strchr(http_msg, '\n');
	while (p_end != p_start)
	{
		if (*p_end == ' ')
		{
			break;
		}
		p_end--;
	}
	strncpy(command, p_start, p_end - p_start);
}

//根据用户在GET中的请求，生成相应的回复内容
int make_http_content(const char *command, char **content)
{
	char *file_buf;
	int file_length;
	char headbuf[1024];
	
	file_length = get_file_content(command, &file_buf);
	if (file_length == 0)
	{
		return 0;
	}
	//printf("file_length = %d\n", file_length);

	memset(headbuf, 0, sizeof(headbuf));
	sprintf(headbuf, HEAD,get_filetype(command), file_length); //设置消息头
	int iheadlen = strlen(headbuf); //得到消息头长度
	int itaillen = strlen(TAIL); //得到消息尾长度
	int isumlen = iheadlen + file_length + itaillen; //得到消息总长度
	*content = (char *) malloc(isumlen); //根据消息总长度，动态分配内存
	if(*content==NULL)
	{
		pt = get_time();
		memset(LOGBUF,0,sizeof(LOGBUF));
		sprintf(LOGBUF,"Warning:%d:%d:%d : %s,%d malloc failed %s\n",
			pt->tm_hour,pt->tm_min,pt->tm_sec, __FILE__, __LINE__, strerror(errno));
        mq_send(mqdes,LOGBUF,strlen(LOGBUF),0);
		
	}
	char *tmp = *content;
	memcpy(tmp, headbuf, iheadlen); //安装消息头
	memcpy(&tmp[iheadlen], file_buf, file_length); //安装消息体
	memcpy(&tmp[iheadlen] + file_length, TAIL, itaillen); //安装消息尾
	//printf("headbuf:\n%s", headbuf);
	if (file_buf)
	{
		free(file_buf);
	}
	return isumlen; //返回消息总长度
}

int  get_file_content(const char *file_name, char **content) // 得到文件内容
{
	int  file_length = 0;
	FILE *fp = NULL;
	if(file_name == NULL)
	{
		return file_length;
	}
	fp = fopen(file_name, "rb");
	if(fp == NULL)
	{
		pt = get_time();
		memset(LOGBUF,0,sizeof(LOGBUF));
		sprintf(LOGBUF,"Warning : %02d:%02d:%02d : file name: %s,%s,%d:open file failture %s \n",
			pt->tm_hour,pt->tm_min,pt->tm_sec,file_name, __FILE__, __LINE__,strerror(errno));
        mq_send(mqdes,LOGBUF,strlen(LOGBUF),0);
		return get_file_content("www/404.html", content);
	}
	fseek(fp, 0, SEEK_END);
	file_length = ftell(fp);
	rewind(fp);

	*content = (char *) malloc(file_length);
	if(*content == NULL)
	{
		pt = get_time();
		memset(LOGBUF,0,sizeof(LOGBUF));
		sprintf(LOGBUF,"Warning:%02d:%02d:%02d : %s,%d:malloc failture %s \n",
			pt->tm_hour,pt->tm_min,pt->tm_sec, __FILE__, __LINE__, strerror(errno));
        mq_send(mqdes,LOGBUF,strlen(LOGBUF),0);
		return 0;
	}

	fread(*content, file_length, 1, fp);
	fclose(fp);
	return file_length;
}

const char *get_filetype(const char *filename) //根据扩展名返回文件类型描述
{
	//得到文件扩展名
	char sExt[32];
	const char *p_start=filename;
	memset(sExt, 0, sizeof(sExt));
	while(*p_start)
	{
		if (*p_start == '.')
		{
			p_start++;
			strncpy(sExt, p_start, sizeof(sExt));
			break;
		}
		p_start++;
	}
	//根据扩展名返回相应描述
	if (strncmp(sExt, "bmp", 3) == 0)
		return "image/bmp";
	if (strncmp(sExt, "gif", 3) == 0)
		return "image/gif";
	if (strncmp(sExt, "ico", 3) == 0)
		return "image/x-icon";
	if (strncmp(sExt, "jpg", 3) == 0)
		return "image/jpeg";
	if (strncmp(sExt, "avi", 3) == 0)
		return "video/avi";
	if (strncmp(sExt, "css", 3) == 0)
		return "text/css";
	if (strncmp(sExt, "dll", 3) == 0)
		return "application/x-msdownload";
	if (strncmp(sExt, "exe", 3) == 0)
		return "application/x-msdownload";
	if (strncmp(sExt, "dtd", 3) == 0)
		return "text/xml";
	if (strncmp(sExt, "mp3", 3) == 0)
		return "audio/mp3";
	if (strncmp(sExt, "mpg", 3) == 0)
		return "video/mpg";
	if (strncmp(sExt, "png", 3) == 0)
		return "image/png";
	if (strncmp(sExt, "ppt", 3) == 0)
		return "application/vnd.ms-powerpoint";
	if (strncmp(sExt, "xls", 3) == 0)
		return "application/vnd.ms-excel";
	if (strncmp(sExt, "doc", 3) == 0)
		return "application/msword";
	if (strncmp(sExt, "mp4", 3) == 0)
		return "video/mpeg4";
	if (strncmp(sExt, "ppt", 3) == 0)
		return "application/x-ppt";
	if (strncmp(sExt, "wma", 3) == 0)
		return "audio/x-ms-wma";
	if (strncmp(sExt, "wmv", 3) == 0)
		return "video/x-ms-wmv";
	
	return "text/html";
}
