#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h> 
#include <sys/epoll.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <string.h>
#include <arpa/inet.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_EVENTS 10
#define USER_MSG_LEN 100
#define MAX_NAME_LEN 16
#define SERV_MSG_LEN (USER_MSG_LEN+MAX_NAME_LEN+10)

#define IPADDRESS "host1.yujincheng.me"
#define PORT 10086
	
int main(int arc, char * argv[])
{
	char * strin;
	char username[MAX_NAME_LEN];
	printf("Set your name: ");
	//fgets(username, MAX_NAME_LEN+1, stdin);
	//strtok(username,"\n");
	strin = readline(NULL);
	strncpy(username, strin, MAX_NAME_LEN);
	//clear_stdin();
	free(strin);
	printf("Your name is: %s\n",username);
	
	int sockfd, ret, nfds;
	unsigned int client_len;

	
	struct sockaddr_in server_address;
	//struct sockaddr_in client_address;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr(IPADDRESS);
	server_address.sin_port = htons(PORT);
	client_len = sizeof(server_address);
	
	ret = connect(sockfd, (struct sockaddr *) &server_address, client_len);
	
	if(ret == -1)
	{
		printf("connection failed.\n");
		exit(1);
	}

	sleep(0.1);
	write(sockfd, username, MAX_NAME_LEN);
	
	int epollfd, i;
	epollfd = epoll_create(1);
	struct epoll_event ev, events[MAX_EVENTS];
	ev.events = EPOLLIN;
	ev.data.fd = sockfd;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &ev); ev.events = EPOLLIN;
	ev.data.fd = STDIN_FILENO;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev);
	
	int nread;
	char serv_msg[SERV_MSG_LEN];
	char user_msg[USER_MSG_LEN];
	for (;;)
	{
		nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
		for(i=0;i<nfds;++i)
		{
			if(events[i].data.fd == sockfd)
			{	// receive msg from server
				ioctl(sockfd, FIONREAD, &nread);
				printf("b\n");
				if (nread == 0)
				{	// server closed
					printf("Server has been closed.\n\n");
					close(sockfd);
					close(epollfd);
					exit(1);
				}
				read(sockfd, serv_msg, nread);
				printf("%s\n\n", serv_msg);
			}
			else if(events[i].data.fd == STDIN_FILENO)
			{	// new msg from user
				printf("start\n");
				strin = readline("send msg:");
				printf("end\n");
				strncpy(user_msg, strin, USER_MSG_LEN);
				free(strin);
				write(sockfd, user_msg, strlen(user_msg)+1);
			}
			else
			{
				printf(" unconsidered\n");
				exit(1);
			}

		}
	}
	return 0;
}
