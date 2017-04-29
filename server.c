#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h> 
#include <sys/select.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <string.h>
#include <arpa/inet.h>


#define MAX_CLIENT 10
#define PORT_NUM 10086
#define RECV_MSG_LEN 100
#define MAX_NAME_LEN 16
#define SEND_MSG_LEN (RECV_MSG_LEN+MAX_NAME_LEN+10)

#define IPADDRESS "127.0.0.1"
#define PORT 10086


int client_index(const int client_status[MAX_CLIENT], int client)
{
	int i;
	for (i=0;i<MAX_CLIENT;++i)
	{
		if(client_status[i]==client) return i;
	}
	return -1;
}
int set_status(int  * client_status, int client, char online)
{
	int new_status = online ? client : -1;
	int old_status = online ? -1 : client;
	int i;
	for (i=0;i<MAX_CLIENT;++i)
	{
		if(client_status[i]==old_status)
		{
			client_status[i] = new_status;
			break;
		}
	}
	return i;
}


void chat_msg(char * sys_msg, char (*client_names)[MAX_NAME_LEN], int client_i, char * client_msg)
{
	char * name = client_names[client_i];
	sprintf(sys_msg,"%s:\n%s", name, client_msg);
}

void client_setname(char (* client_names)[MAX_NAME_LEN], int client_i, const char * msg)
{
	printf("setting name: %s\n", msg);
	strncpy(client_names[client_i], msg, MAX_NAME_LEN);
	client_names[client_i][MAX_NAME_LEN-1] = 0;
}

void broadcast(const int client_status[MAX_CLIENT], int expt, char * msg)
{
	int i;
	for (i=0;i<MAX_CLIENT;++i)
	{
		if (client_status[i] != -1 && client_status[i] != expt)
		{
			write(client_status[i], msg, SEND_MSG_LEN);
		}
	}
}

int main(int argc, char * argv[])
{
	int client_status[MAX_CLIENT]; // -1:no client, pos:client fds
	char client_names[MAX_CLIENT][MAX_NAME_LEN];

	int server_sockfd, client_sockfd;
	unsigned int server_len, client_len;
	
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;
	fd_set fdslist, testfds;

	server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_address.sin_port = htons(PORT);
	server_len = sizeof(server_address);

	bind(server_sockfd, (struct sockaddr *) &server_address, server_len);
	listen(server_sockfd, MAX_CLIENT);

	FD_ZERO(&fdslist);
	FD_SET(server_sockfd, &fdslist);

	int i;
	for(i=0;i<MAX_CLIENT;++i) client_status[i] = -1;
	char sys_msg[SEND_MSG_LEN];
	char client_msg[RECV_MSG_LEN];

	printf("server waiting\n");
	while(1)
	{
		int fd, result, client_i;
		int nread; // for counting
		testfds = fdslist;

		result = select(FD_SETSIZE, &testfds, (fd_set *)0,(fd_set *)0,(struct timeval *) 0);
		if(result < 1)
		{
			perror("server down\n");
			exit(1);
		}

		for(fd = 0;fd < FD_SETSIZE; fd++)
		{
			if(FD_ISSET(fd, &testfds))
			{
				if(fd == server_sockfd)
				{
					client_len = sizeof(client_address);
					client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);
					FD_SET(client_sockfd, &fdslist);
					printf("adding client on fd %d\n", client_sockfd);
					//strcpy(sys_msg, "Please set a username:");
					//write(client_sockfd, sys_msg, 30);
				}
				else
				{
					client_i = client_index(client_status, fd);
					printf("from client %d\n", client_i);
					ioctl(fd, FIONREAD, &nread);
					//ioctl(fd, I_NREAD, &nread);
					printf("nread=%d\n",nread);
					if(nread == 0)
					{	// Leave chat
						close(fd);
						FD_CLR(fd, &fdslist);
						if (client_i >= 0)
						{
							sprintf(sys_msg, "%s has left the chatroom.", client_names[client_i]);
							printf("%s\n",sys_msg);
							set_status(client_status, fd, 0);
							broadcast(client_status, -1, sys_msg);
						}
					}
					else
					{
						read(fd, client_msg, nread);
						if (client_i < 0)
						{	// Join chat
							client_i = set_status(client_status, fd, 1);
							client_setname(client_names, client_i, client_msg);
							sprintf(sys_msg, "%s has joined the chatroom\n", client_names[client_i]);
							printf("%s\n",sys_msg);
							broadcast(client_status, -1, sys_msg);
						}
						else
						{	// new message received
							//chat_msg(sys_msg, client_status, client_names, fd, client_msg);
							sprintf(sys_msg,"%s:\n%s", client_names[client_i], client_msg);
							printf("%s\n",sys_msg);
							broadcast(client_status, -1, sys_msg);
						}
					}
				}
			}
		}
	}


	return 0;
}

