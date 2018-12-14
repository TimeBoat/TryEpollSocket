/*0000: 需要显示 1111:创建目录2222:返回上一级目录5555:文件名字大小8888:文件内容*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>

#define IP "127.0.0.1"
#define PORT 4445

int main()
{
	int ret = 0;
	char *buf = NULL;
	int sockid = 0;
	int addlen = 0;
	int len = 0;
	long int size = 0;
	FILE *fp = NULL;
	struct sockaddr_in soaddr;
	struct stat st_buf;
	
	memset(&st_buf,0,sizeof(st_buf));
	chdir("/home/xth/xth");
	buf = malloc(505);
	if(!buf)
	{
		printf("malloc error\n");
		return 1;
	}
	sockid = socket(AF_INET,SOCK_STREAM,0);
	if (0 > sockid)
	{
		perror("socket error");
		return 1;
	}
	
	soaddr.sin_family = AF_INET;
	soaddr.sin_port = htons(PORT);
	soaddr.sin_addr.s_addr = inet_addr(IP);
	addlen = sizeof(soaddr);
	ret = connect(sockid,(struct sockaddr *)&soaddr,addlen);
	if(0 > ret)
	{
		perror("connect error");
		return 1;
	}
	
	while(1)
	{
		memset(buf,0,505);
		ret = recv(sockid,buf,505,0);
		if(0 > ret)
		{
			perror("recv error");
			exit;
		}
		if(buf[501] == '0'||buf[502] == '0'||buf[503] == '0'||buf[504] == '0')
		{
			printf("%s\n",buf);
			if(strstr(buf,"input"))
			{
				memset(buf,0,505);
				gets(buf);
				printf("buf:%s\n",buf);
				ret = send(sockid,buf,505,0);
				if(0 > ret)
				{
					perror("resend error");
					exit;
				}
				if((!memcmp(buf,"q\0",3))||(!memcmp(buf,"Q\0",3)))
				{
					exit;
					return 1;
				}
			}
			else if(strstr(buf,"no such"))
			{
				continue;
			}
		}
		else if(buf[501] == '1'||buf[502] == '1'||buf[503] == '1'||buf[504] == '1')
		{
			ret = mkdir(buf,0777);
			if (0 > ret)
			{
				perror("mkdir error");
				exit;
			}
			chdir(buf);
		}
		else if(buf[501] == '2'||buf[502] == '2'||buf[503] == '2'||buf[504] == '2')
		{
			chdir("./..");
		}
		else if(buf[501] == '5'||buf[502] == '5'||buf[503] == '5'||buf[504] == '5')
		{
			sscanf(buf+300,"%ld",&size);
			printf("size:%ld\n",size);
			fp = fopen(buf,"a");
			if(!fp)
			{
				perror("fopen error");
			}
			stat(buf,&st_buf);
			memset(buf,0,505);
			sprintf(buf,"%ld",st_buf.st_size);
			ret = send(sockid,buf,505,0);
			if (0 > ret)
			{
				perror("send size error");
				exit;
			}
		}
		else if(buf[501] == '8'||buf[502] == '8'||buf[503] == '8'||buf[504] == '8')
		{
			if(size <= st_buf.st_size+505+len)
			{
				ret = fwrite(buf,size-st_buf.st_size-len,1,fp);
				if (0 > ret)
				{
					perror("fwrite error");
					return;
				}
				len = 0;
				size = 0;
				memset(&st_buf,0,sizeof(st_buf));
				fclose(fp);
				continue;
			}
			ret = fwrite(buf,500,1,fp);
			if (0 > ret)
			{
				perror("fwrite error");
				return;
			}
			len += 500;
		}
	}
}
