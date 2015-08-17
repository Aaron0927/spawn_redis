#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/socket.h>
#include <linux/in.h>
#include <string.h>

int flag = 0;

int fs_open(char *ip, unsigned short port)
{
    int fd; 
    int recbytes;
    int sin_size;
    struct sockaddr_in s_add, c_add; 

    printf("Open file!\r\n");
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1)
    {
        printf("open fail! \r\n");
        return -1;
    }

    bzero(&s_add, sizeof(struct sockaddr_in)); // 清空结构体
    s_add.sin_family = AF_INET; // 采用IPv4网络协议
    s_add.sin_addr.s_addr= inet_addr(ip); 
    s_add.sin_port = htons(port); 

    if(-1 == connect(fd, (struct sockaddr *)(&s_add), sizeof(struct sockaddr)))
    {
        printf("open fail!\r\n");
        return -1;
    }

	char rcv[32] ={0};
	if (read(fd, rcv, 32) == -1) 
	{
        printf("read error!\r\n");
    } else {
		if (strcmp(rcv, "zcf_ok") == 0) {
			printf("read OK! RCV = %s\n", rcv);
		} else {
			close(fd);

			flag = 1;		//printf("s = %d \n", s);
			printf("a new redis-server has up! RCP = %s\n", rcv);
		}
	}

    printf("open ok!\r\n");                    
    return fd;
}

void fs_read(int fd, char *fname)
{
    char buffer[1024] = "get ";
    char rcv[1024] = {0};
    strcat(buffer, fname);
    strcat(buffer, " \r\n");
    printf("read : %s", buffer);
    send(fd, buffer, sizeof(buffer), 0);

    if (read(fd, rcv, 1024) == -1) 
    {
        printf("read error!\n");
        return;
    }
    printf("read OK!\n");
    printf("RCV : %s\n", rcv);
    return;

}

void fs_write(int fd, char *fname, char *buf)
{
    char buffer[1024] = "set ";
    char rcv[1024] = {0};
    strcat(buffer, fname);
    strcat(buffer, " \"");
    strcat(buffer, buf);
    strcat(buffer, "\"\n");
    printf("write : %s", buffer);
    send(fd, buffer, sizeof(buffer), 0);
    if (read(fd, rcv, 1024) == -1) 
    {
        printf("write error!\n");
        return;
    }
    printf("write OK!\n");
    printf("RCV : %s\n", rcv);
}

void fs_close(int fd)
{
    close(fd);
    printf("file close OK!\n");
}


int main()
{
    int fd = fs_open("192.168.182.12", 6379);
    
    getchar();
    fs_close(fd);
    if (flag == 1)
	fs_open("192.168.182.13", 6379);
    getchar();
    return 0;
}
