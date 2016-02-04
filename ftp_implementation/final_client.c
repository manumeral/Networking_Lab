/*
** client.c -- a stream socket client demo
*/
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
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
#define COMMAND_SIZE 512
void get_file(char *,char *,int );
void put_file(char *,char *,int );
bool check(char *,char *);
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
	char acknowledgement;
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
		if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) 
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
//CONNECTION ESTABLISHED
	int count , count_of_packets ;
	count=count_of_packets=0;
	char command[COMMAND_SIZE];
	char receive[512];
	printf("Connected To Server\n");
//SENDING COMMAND
	do
	{
		int iter = 1 ;
		int count_of_comm_pack = 0 ;
		bool eof_command = 	false ;
		printf("\nMyftp>");
		while(!isalpha(command[0]=getchar())); 
		while((command[iter++]=getchar()) != '\n');
		command[iter-1]='\0';
		iter = 0 ;	
		char name[100],file_name[128];
		name[0]='\0';
		sprintf(name,".swp_transfer%lld\0",(long long int)(getpid()));
		char cat_command[128];
		cat_command[0] = '\0';
		strcat(cat_command,"cat ");
		strcat(cat_command,name);
		if( !check(command,"lls") && !check(command,"lpwd") )
		{
			while(!eof_command)
			{
				count = 0 ; 
				while( (count < PACKET_SIZE-1) && (iter < 256) )
				{
					if(command[iter] != '\0' )
						buf[count++]=command[iter++];
					else
					{
						buf[count++]=command[iter++];
						eof_command = true ;
						break;
					}	
				}
				if(eof_command)
				{
					buf[PACKET_SIZE-1] = '0';// NO MORE PACKETS COMING
				}
				else
				{
					buf[PACKET_SIZE-1] = '1';//MORE PACKETS COMING
				}
				// printf("%s %s\n",command,buf);
				if (send(sockfd, buf , sizeof(buf) , 0) == -1)
				{
					perror("send");
				}
				if ( recv(sockfd, &acknowledgement, sizeof(acknowledgement), 0) == -1) 
				{
					perror("recv ack");
					exit(1);
				}
				if(acknowledgement == '1')
				{
					count_of_comm_pack++;
					printf("Command Packet #%d Sent\n",count_of_comm_pack);
				}
				if(eof_command)
				{
					printf("Command Transfer Completed !\n");
				}
			}
		}
		if(!check(command,"mget") && !check(command,"mput") && !check(command,"put") && !(check(command,"get")) && !check(command,"cd") )
		{
			if(check(command,"lls")||check(command,"lpwd"))
			{
				system(&command[1]);	
			}
			else
			{
				get_file(name,buf,sockfd);
				system(cat_command);
			}
		}
		else if(check(command,"put"))
		{
			int find = 0 ;
			int find_r = 1 ;
			while(!isspace(receive[0]=command[find++]));
			while(isspace(receive[0]=command[find++]));
			while(command[find] != '\0' && !isspace(receive[find_r++]=command[find++]));
			receive[find_r]='\0';
			put_file(receive,buf,sockfd);
		}
		else if(check(command,"get"))
		{
			int find = 0 ;
			int find_r = 1 ;
			while(!isspace(receive[0]=command[find++]));
			while(isspace(receive[0]=command[find++]));
			while(command[find] != '\0' && !isspace(receive[find_r++]=command[find++]));
			receive[find_r]='\0';
			get_file(receive,buf,sockfd);	
		}
		else if(check(command,"mput"))
		{
			int find = 0 ;
			int find_r = 1 ;
			while(!isspace(receive[0]=command[find++]));
			while(command[find]!='\0')
			{
				find_r = 1 ;
				while(isspace(receive[0]=command[find++]));
				while(!isspace(receive[find_r++]=command[find]))
					if(command[find++] == '\0')
						break;
				if(receive[find_r-1] != '\0')
					receive[find_r-1] = '\0';
				else
				{	
					receive[find_r-1] = '\0';
					command[find] = '\0' ;
				}
				put_file(receive,buf,sockfd);
			}
		}
		else if(check(command,"mget"))
		{
			int find = 0 ;
			int find_r = 1 ;
			while(!isspace(receive[0]=command[find++]));
			while(command[find]!='\0')
			{
				find_r = 1 ;
				while(isspace(receive[0]=command[find++]));
				while(!isspace(receive[find_r++]=command[find]))
					if(command[find++] == '\0')
						break;
				if(receive[find_r-1] != '\0')
					receive[find_r-1] = '\0';
				else
				{	
					receive[find_r-1] = '\0';
					command[find] = '\0' ;
				}
				get_file(receive,buf,sockfd);
			}
		}
	}while(strcmp(command,"quit") != 0);

	close(sockfd);
	return 0;	
}
void put_file(char *file_name,char *buf,int new_fd)
{
	// printf("In put_file\n");
	FILE *fd=fopen(file_name,"rb");
	int count_of_packets = 0 ;
	int count = 0,temp,numbytes;
	bool eof_flag = false ;
	while(1)
	{
		count  = 0 ;
		eof_flag = false;
		//Preparing a Packet
		temp=fread(buf,sizeof(char),PACKET_SIZE-1,fd);
		count_of_packets++;
		if(temp<PACKET_SIZE-1)
		{
			printf("Packet #%d sent \n",count_of_packets);
			buf[temp]='0';
		}
		else
		{
			buf[temp]='1';
		}
		//Packet is being Sent
		numbytes=send(new_fd, buf, temp+1, 0);
		if(numbytes == -1)
			perror("send");
		if (recv(new_fd, buf, sizeof(buf), 0) == -1) 
		{
			perror("recv");
			exit(1);
		}
		if(buf[temp]=='0')
			break;
		//Packet Sent and Acknowledged
	}
	printf("File Transferred !\n");
	fclose(fd);
}
void get_file(char *file_name,char * buf,int sockfd)
{
//TRANSFER STARTS
	// printf("In get_file\n");
	int count , count_of_packets ;
	count=count_of_packets=0;
	FILE * fd = fopen(file_name,"wb");
	while(1)
	{
		int numbytes = recv(sockfd, buf, PACKET_SIZE, 0);
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
		fwrite(buf,sizeof(char),numbytes-1,fd);
//		printf("client: received packet #%d\n",count_of_packets);	
		
		printf("%d\n",numbytes);
		if(buf[numbytes-1]=='0')
		{
			break ;
		}
		/*while(count < PACKET_SIZE-1)
		{
			
			if(buf[count]=='\0')
			{
				// fputc(buf[count++],fd);
				break;	
			}
			else
			{
				fputc(buf[count++],fd);
			}
		}*/
		/*printf("client: received packet #%d\n",count_of_packets);	
		
		printf("%d\n",numbytes);*/
	}
	fclose(fd);
////
}
bool check(char * f1, char * f2)
{
	int iter = 0 ;
	while((f1[iter]==f2[iter]) && (f2[iter] != '\0') && (f1[iter] != '\0') )
	{
		iter++;
	}
	if( f2[iter]=='\0' )
	{
		return true;
	}
	else
	{
		return false;
	}
}