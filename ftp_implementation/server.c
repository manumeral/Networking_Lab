#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#define PORT "3490" 
#define BACKLOG 10 
#define PACKET_SIZE 128 
void sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) 
	{
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
int main(void)
{
	int sockfd, new_fd; 
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; 
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	for(p = servinfo; p != NULL; p = p->ai_next) 
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) 
		{
			perror("server: socket");
			continue;
		}
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1) 
		{
			perror("setsockopt");
			exit(1);
		}
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
		{
			close(sockfd);
			perror("server: bind");
			continue;
		}
		break;
	}
	if (p == NULL) 
	{
		fprintf(stderr, "server: failed to bind\n");
		return 2;
	}
	freeaddrinfo(servinfo); 
	if (listen(sockfd, BACKLOG) == -1) 
	{
		perror("listen");
		exit(1);
	}
	sa.sa_handler = sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) 
	{
		perror("sigaction");
		exit(1);
	}
	char buf[PACKET_SIZE];
	int numbytes,temp;
	printf("server: waiting for connections...\n");
	while(1) 
	{ 
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) 
		{
			perror("accept");
			continue;
		}
		inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr *)&their_addr),s, sizeof s);
		printf("server: got connection from %s\n", s);
		if (!fork()) 
		{
		 // this is the child process
			close(sockfd); // child doesn't need the listener
			FILE *fd=fopen("send.txt","rb");
			int count_of_packets = 0 ;
			int count = 0;
			bool eof_flag = false ;
			while(1)
			{
				count  = 0 ;
				eof_flag = false;
				//Preparing a Packet
				while( count < (PACKET_SIZE-1) )
				{
					if(!feof(fd))
						buf[count++]=fgetc(fd);
					else
					{
						eof_flag = true ;
						break;
					}
				}
				//Packet is being Sent
				count_of_packets++;
				if(eof_flag == false)
				{
					buf[PACKET_SIZE-1]='1';// To indicate that one more packet will be sent 
				}
				else
				{
					buf[PACKET_SIZE-1]='0';	
				}
				temp=send(new_fd, buf, PACKET_SIZE, 0);
				if(temp == -1)
					perror("send");
				if ((numbytes = recv(new_fd, buf, sizeof(buf), 0)) == -1) 
				{
					perror("recv");
					exit(1);
				}
				if(buf[0]=='0')
					printf("Packet #%d sent \n",count_of_packets);
				//Packet Sent and Acknowledged
				if(eof_flag == true)
					break;
			}
			printf("File Transferred !\n");fclose(fd);
			close(new_fd);
			exit(0);
			
		}
		close(new_fd); // parent doesn't need this
	}
	return 0;
}