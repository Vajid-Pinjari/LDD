#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
int main(int argc,char *argv[])
{
	int fd;
	char str[20];
	fd=open("/dev/KV_driver",O_WRONLY);
	if(fd<2)
	{
		perror("open");
		return 0;
	}else perror("open");
	if(*argv[1]=='I')
	{
	if(write(fd,argv[1],strlen(argv[1]))<0)
	{
		perror("write");
		return 0;
	}
	perror("write");
	close(fd);
	fd=0;

	fd=open("/dev/KV_driver",O_RDONLY);
        if(fd<=2)
        {
                perror("open");
                return 0;
        }else perror("open");

	if(read(fd,str,sizeof(str))<0)
	{
		perror("read");
		return 0;
	}
	else perror("read");
	printf("coming from driver %d : %s\n",fd,str);
	}
	else
	{
		if(write(fd,argv[1],strlen(argv[1]))<0)
		{
			perror("write");
			return 0;
		}
		char str[100];
		close(fd);
		fd=0;
		fd=open("/dev/KV_driver",O_RDONLY);
		if(fd<=2)
		{
			perror("open");
			return 0;
		}
		else perror("open");
		if(read(fd,str,sizeof(str)-1)<0)
		{
			perror("read");
			return 0;
		}
		printf("Data : %s\n",str);
	}
}
