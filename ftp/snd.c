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

void sreach(int sockidusr);
void filecopy(int sockidusr,char *name);
void dircopy(int sockidusr,char *name);
void download(int sockidusr);
void *snd(void *p);
void * acc(void *p);

int n = 10;
pthread_t pth[10];
int id_new[10] = {0};
int sockid = 0;
//static int i = 0;
int main()
{
	
	struct sockaddr_in soaddr;
	struct stat st_buf;
	pthread_t pth_acc;
	char buf[100] = "\0";
	int ret = 0;
	int addlen = 0;
	
	sockid = socket(AF_INET,SOCK_STREAM,0);
	if (0 > sockid)
	{
		perror("1 sockid error");
		return 1;
	}
	
	soaddr.sin_family = AF_INET;
	soaddr.sin_port = htons(PORT);
	soaddr.sin_addr.s_addr = INADDR_ANY;
	addlen = sizeof(soaddr);
	ret = bind(sockid,(struct sockaddr *)&soaddr,addlen);
	if (0 > ret)
	{
		perror("bind error");
		exit;
	}
	
	ret = listen(sockid,n);
	if (0 > ret)
	{
		perror("listen error");
		exit;
	}
	
	ret = pthread_create(&pth_acc,NULL,acc,NULL);
	if (0 > ret)
	{
		perror("0 pthread_create error");
		exit;
	}
	pthread_join(pth_acc,NULL);
	close(sockid);
	exit;
}

void * acc(void *p)
{
	int fl = 0;
	int ret = 0;
	int tmp = 0;
	struct sockaddr_in *add[10];
	int addlen = sizeof(struct sockaddr_in); 
	//sleep(20);
	while(10 > fl)
	{   add[fl] = malloc(sizeof(struct sockaddr_in)); 
		id_new[fl] = accept(sockid,(struct sockaddr*)add[fl],&addlen);
		if (0 > id_new[fl])
		{
			perror("access error");
			exit;
		}
		tmp = fl;
		ret = pthread_create(&pth[fl],NULL,snd,&tmp);
		printf("fl =%d\n",fl);
		fl ++;
	}
	while(fl >= 0)
	{
		pthread_join(pth[fl],NULL);
		fl--;
	}
}

void *snd(void *p)
{
	char *buf = malloc(505);
	int i = *(int *)p;
	printf("i = %d\n",i);
	sleep(20);
	struct stat st_buf;
	FILE * fp = NULL;
	int ret = 0;
	if (!buf)
	{
		//perror("malloc [%d] buf error",i);
		exit;
	}
	char *name = malloc(256);
	if (!name)
	{
		//perror("malloc [%d] name error",i);
		exit;
	}
	chdir("/home/xth");
	//sleep(15);
	while(1)
	{   
		fp = popen("ls /home/xth","r");
		chdir("/home/xth");
		if (!fp)
		{
			//perror("[%d]popen error",i);
			exit;
		}
		fstat(fp,&st_buf);
		while(!feof(fp))
		{
			ret = fread(buf,500,1,fp);
			if (0 > ret)
			{
				perror("fread error");
			}
			memcpy(buf+500,"0000",5);
			ret = send(id_new[i],buf,505,0);
			if (0 > ret)
			{
				perror("send error");
			}
			memset(buf,0,505);
		}
		fclose(fp);
		memcpy(buf,"input:\tdownload:D,sreach dir:S,quit:Q",50);
		memcpy(buf+500,"0000",5);
		ret = send(id_new[i],buf,505,0);
		if (0 > ret)
		{
			perror("help send error");
		}
		memset(buf,505,0);
		ret = recv(id_new[i],buf,505,0);
		printf("recv buf:%s\n",buf);
		if(buf[0] == 'd'||buf[0] == 'D')
		{
			download(id_new[i]);
		}
		if(buf[0] == 's'||buf[0] == 'S')
		{
			sreach(id_new[i]);
		}
		if(buf[0] == 'q'||buf[0] == 'Q')
		{
			break;
		}
	}
	if (buf)
	{
		free(buf);
		buf = NULL;
	}
	exit;
}

void download(int sockidusr)
{
	int ret = 0;
	char *buf = malloc(505);
	struct stat st_buf;
	FILE * fp = NULL;
	if (!buf)
	{
		perror("malloc  buf error");
		exit;
	}
	char *name = malloc(256);
	if (!name)
	{
		perror("malloc  name error");
		exit;
	}
	memcpy(buf,"input:\twhich to download",30);
	memcpy(buf+500,"0000",5);
	ret = send(sockidusr,buf,505,0);
	if (0 > ret)
	{
		perror("download send error");
		exit;
	}
	memset(buf,0,505);
	ret = recv(sockidusr,buf,505,0);
	if (0 > ret)
	{
		perror("download send error");
		exit;
	}
	printf("download buf:%s\n",buf);
	ret = access(buf,F_OK);
	if (0 > ret)
	{
		memcpy(buf,"no such file",30);
		memcpy(buf+500,"0000",5);
		ret = send(sockidusr,buf,505,0);
		if (0 > ret)
		{
			perror("download send error");
			exit;
		}
	}
	else
	{
		stat(buf,&st_buf);
		if(S_IFDIR&st_buf.st_mode)
		{
			dircopy(sockidusr,buf);
		}
		else
		{
			filecopy(sockidusr,buf);
		}
		memcpy(buf,"download over!",30);
		memcpy(buf+500,"0000",5);
		ret = send(sockidusr,buf,505,0);
		if (0 > ret)
		{
			perror("download send error");
			exit;
		}
	}
	if (buf)
	{
		free(buf);
		buf = NULL;
	}
	return;
}

void dircopy(int sockidusr,char *p)
{
	
	int ret = 0;
	struct stat st_buf;
	char *buf = malloc(505);
	if (!buf)
	{
		printf("dircopy malloc error");
		exit;
	}
	char *name = malloc(256);
	if (!name)
	{
		printf("name malloc error");
		exit;
	}
	DIR *dir;
	struct dirent *d_buf;
	memset(name,0,256);
	strncpy(name,p,strlen(p));
	printf("dir name:%s\n",name);
	dir = opendir(name);
	if (!dir)
	{
		perror("opendir error");
		exit;
	}
	chdir(name);
	memset(buf,505,0);
	memcpy(buf,name,strlen(name));
	memcpy(buf+500,"1111",5);
	ret = send(sockidusr,buf,505,0);
	if (0 > ret)
	{
		perror("dircopy 1 send error");
		exit;
	}
	while(d_buf = readdir(dir))
	{
		if((0 == strcmp((*d_buf).d_name,"."))||(0 == strcmp((*d_buf).d_name,"..")))
		{
			continue;
		}
		else
		{
			if((*d_buf).d_type == 4)
			{
				dircopy(sockidusr,(*d_buf).d_name);
				
			}
			else
			{
				filecopy(sockidusr,(*d_buf).d_name);
			}
		}
	}
	memset(buf,505,0);
	memcpy(buf,"dircopy over!",strlen("dircopy over!"));
	memcpy(buf+500,"2222",5);
	ret = send(sockidusr,buf,505,0);
	if (0 > ret)
	{
		perror("dircopy 1 send error");
		exit;
	}
	chdir("./..");
	return;
}

void filecopy(int sockidusr,char *name)
{
	int ret = 0;
	struct stat st_buf;
	long int size= 0;
	long int fsize = 0;
	FILE *fp = NULL;
	char *buf = malloc(505);
	if (!buf)
	{
		printf("dircopy malloc error");
		exit;
	}
	stat(name,&st_buf);
	size = st_buf.st_size;
	memset(buf,0,505);
	memcpy(buf,name,strlen(name));
	//memcpy(buf+300,&(st_buf.st_size),sizeof(st_buf.st_size));
	sprintf(buf+300,"%ld",st_buf.st_size);
	memcpy(buf+500,"5555",5);
	ret = send(sockidusr,buf,505,0);
	if (0 > ret)
	{
		perror("filecopy error");
		return;
	}
	fp = fopen(name,"r");
	if(!fp)
	{
		perror("filecopy fopen error");
		return;
	}
	memset(buf,0,505);
	ret = recv(sockidusr,buf,505,0);
	if (0 > ret)
	{
		perror("filecopy recv error");
		return;
	}
	sprintf(buf,"%ld",fsize);
	fseek(fp,fsize,SEEK_SET);
	while(!feof(fp))
	{
		ret = fread(buf,500,1,fp);
		if (0 > ret)
		{
			perror("filecopy fread error");
			return ;
		}
		memcpy(buf+500,"8888",5);
		ret = send(sockidusr,buf,505,0);
		if (0 > ret)
		{
			perror("filecopy send file error");
			return;
		}
		memset(buf,0,505);
	}
}

void sreach(int sockidusr)
{
	char *buf = NULL;
	FILE *fp = NULL;
	struct stat st_buf;
	int ret = 0;
	char cwd[50] = "\0";
	buf = malloc(505);
	if (!buf)
	{
		printf("sreach malloc error");
		exit;
	}
	memcpy(cwd,"ls ",4);
	fp = popen(cwd,"r");
	
	if (!fp)
	{
		perror("popen error");
		exit;
	}
	
	while(!feof(fp))
	{
		ret = fread(buf,500,1,fp);
		if (0 > ret)
		{
			perror("sreach fread error");
			return ;
		}
		memcpy(buf+500,"0000",5);
		ret = send(sockidusr,buf,505,0);
		if (0 > ret)
		{
			perror("sreach send ls error");
			return;
		}
		memset(buf,0,505);
	}
	fclose(fp);
	memset(buf,0,505);
	memcpy(buf,"input:\twhice do you want to sreach",50);
	memcpy(buf+500,"0000",5);
	ret = send(sockidusr,buf,505,0);
	if (0 > ret)
	{
		perror("sreach send 1 error");
		return ;
	}
	memset(buf,0,505);
	ret = recv(sockidusr,buf,505,0);
	if (0 > ret)
	{
		perror("sreach 1 recv error");
		return;
	}
	ret = access(buf,F_OK);
	if(0 > ret)
	{
		memset(buf,0,505);
		memcpy(buf,"no such dir",30);
		memcpy(buf+500,"0000",5);
		ret = send(sockidusr,buf,505,0);
		if (0 > ret)
		{
			perror("sreach send 1 error");
			return ;
		}
		return ;
	}
	stat(buf,&st_buf);
	if(!(st_buf.st_mode&S_IFDIR))
	{
		memset(buf,0,505);
		memcpy(buf,"no such dir",30);
		memcpy(buf+500,"0000",5);
		ret = send(sockidusr,buf,505,0);
		if (0 > ret)
		{
			perror("sreach send 1 error");
			return ;
		}
		return ;
	}
	chdir(buf);
	memcpy(cwd,"ls ",4);
	fp = popen(cwd,"r");
	
	if (!fp)
	{
		perror("popen error");
		exit;
	}
	
	while(!feof(fp))
	{
		ret = fread(buf,500,1,fp);
		if (0 > ret)
		{
			perror("sreach fread error");
			return ;
		}
		memcpy(buf+500,"0000",5);
		ret = send(sockidusr,buf,505,0);
		if (0 > ret)
		{
			perror("sreach send ls error");
			return;
		}
		memset(buf,0,505);
	}
	fclose(fp);
	memset(buf,0,505);
	memcpy(buf," input:\tdownload:D,sreach dir:S,quit:Q,return:R",50);
	memcpy(buf+500,"0000",5);
	ret = send(sockidusr,buf,505,0);
	printf("---------\n");
	if (0 > ret)
	{
		perror("help send error");
	}
	memset(buf,505,0);
	ret = recv(sockidusr,buf,505,0);
	if (0 > ret)
	{
		perror("sreach recv error");
		exit;
	}
	if(buf[0] == 'd'||buf[0] == 'D')
	{
		download(sockidusr);
	}
	if(buf[0] == 's'||buf[0] == 'S')
	{
		sreach(sockidusr);
		chdir("./..");
	}
	if(buf[0] == 'q'||buf[0] == 'Q')
	{
		exit;
	}
	if(buf[0] == 'r'||buf[0] == 'R')
	{
		chdir("./..");
		sreach(sockidusr);
	}
}
