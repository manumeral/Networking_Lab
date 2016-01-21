/*
** client.c -- a stream socket client demo
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define PORT "3490" // the port client will be connecting to
// get sockaddr, IPv4 or IPv6:
#define PACKET_SIZE 128
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) 
	{
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
int main(int argc, char *argv[])
{
	int sockfd, numbytes,temp;
	char buf[PACKET_SIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	if (argc != 2) 
	{
		fprintf(stderr,"usage: client hostname\n");
		exit(1);
	}
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) 
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
		p->ai_protocol)) == -1) 
		{
			perror("client: socket");
			continue;
		}
		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
		{
			close(sockfd);
			perror("client: connect");
			continue;
		}
		break;
	}
	if (p == NULL) 
	{
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),s, sizeof s);
	printf("client: connecting to %s\n", s);
	freeaddrinfo(servinfo); // all done with this structure

	int count , count_of_packets ;
	count=count_of_packets=0;
	FILE * fd = fopen("received.txt","wb");
	while(1)
	{
		numbytes = recv(sockfd, buf, PACKET_SIZE, 0);
		if (numbytes == -1) 
		{
			perror("recv");
			exit(1);
		}
		if (send(sockfd, "0", 1, 0) == -1)
		{
			perror("send ack");
		}
		count_of_packets++;
		count = 0 ;
		while(count < PACKET_SIZE-1)
		{
			
			if(buf[count]==EOF)
			{
				break;	
			}
			else
			{
				fputc(buf[count++],fd);
			}
		}
		printf("client: received packet #%d\n",count_of_packets);	
	
		if(buf[PACKET_SIZE-1] == '0')
		{
			break ;
		}
	}
	fclose(fd);
	close(sockfd);
	return 0;	
}
