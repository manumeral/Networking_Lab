#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <sys/param.h> // MAXPATHLEN is defined here
#include <errno.h> // To check the value of errno : To check for path name length related issues
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
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
#define COMMAND_SIZE 512
#define FALSE 0
#define TRUE 1
int file_select();
extern  int alphasort();
char* pathname;
void put_file(char*,char *,int);
void get_file(char *,char * ,int );
bool check(char *,char *);
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
  char command[COMMAND_SIZE];
  char receive[512] ;
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
      //Check Command;
      
      while(1)
      {
        int count_of_comm_pack=0;
        int iter = 0 ;
        int c = 0 ;
        command[0]='\0';
        while(1)
        {
          if ( (recv(new_fd, buf,sizeof(buf), 0)) == -1) 
          {
            perror("recv");
            exit(1);
          }
          if (send(new_fd, "0", 1, 0) == -1)
          {
            perror("send ack");
          }
          count_of_comm_pack++;
          c = 0 ;
          while(c < PACKET_SIZE-1)
          {
            // printf("%d ",buf[c]);
            if(buf[c]=='\0')
            {
              command[iter]='\0';
              break;  
            }
            else
            {
              command[iter++]=buf[c++];
            }
          }
          printf("\n%d client: received packet #%d\n",buf[PACKET_SIZE-1],count_of_comm_pack); 
         
          if(buf[PACKET_SIZE-1] == '0')
          {
            break ;
          }
        }
        printf("OUT\n");
        
        if(strcmp(command,"quit") != 0)
        { 
          if( check(command,"ls") )
          {
            int find = 0 ;
            int find_r = 1 ;
            while(!isspace(receive[0]=command[find++]));
            while(isspace(receive[0]=command[find++]));
            while(command[find] != '\0' && !isspace(receive[find_r++]=command[find++]));
            receive[find_r]='\0';
            list(receive,buf,new_fd);
          }
          else if(check(command,"put"))
          {
            int find = 0 ;
            int find_r = 1 ;
            while(!isspace(receive[0]=command[find++]));
            while(isspace(receive[0]=command[find++]));
            while(command[find] != '\0' && !isspace(receive[find_r++]=command[find++]));
            receive[find_r]='\0';
            get_file(receive,buf,new_fd); 
          }
          else if(check(command,"get"))
          {
            int find = 0 ;
            int find_r = 1 ;
            while(!isspace(receive[0]=command[find++]));
            while(isspace(receive[0]=command[find++]));
            while(command[find] != '\0' && !isspace(receive[find_r++]=command[find++]));
            receive[find_r]='\0';
            put_file(receive,buf,new_fd);
          }
          else if(check(command,"cd"))
          {
            int find = 0 ;
            int find_r = 1 ;
            while(!isspace(receive[0]=command[find++]));
            while(isspace(receive[0]=command[find++]));
            while(command[find] != '\0' && !isspace(receive[find_r++]=command[find++]));
            receive[find_r]='\0';
            chdir(receive);
          }
        }
        else
        {
          printf("Attempting Connection Close.");
          break;
        }//Command Confirmed
      }
      printf("Connection Closed\n");
      close(new_fd);
      exit(0);
      
    }
    close(new_fd); // parent doesn't need this
  }
  return 0;
}
void put_file(char *file_name,char *buf,int new_fd)
{
  // printf("In put_file\n");
  FILE *fd=fopen(file_name,"r");
  int count_of_packets = 0 ;
  int count = 0,temp,numbytes;
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
        buf[count-1]='\0';
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
  printf("File Transferred !\n");
  fclose(fd);
}
void get_file(char *file_name,char * buf,int sockfd)
{
//TRANSFER STARTS
  // printf("In get_file\n");
  int count , count_of_packets ;
  count=count_of_packets=0;
  FILE * fd = fopen(file_name,"w");
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
    while(count < PACKET_SIZE-1)
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
    }
    printf("client: received packet #%d\n",count_of_packets); 
  
    if(buf[PACKET_SIZE-1] == '0')
    {
      break ;
    }
  }
  fclose(fd);
////TRANSFER ENDS
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
void list(void)
{
  int count,i,choice,j;
  choice = 1 ;
  struct dirent **files;
  pathname = (char *)(malloc( MAXPATHLEN * sizeof(char) ) ) ;
  char *new_path = (char *)malloc(sizeof(char) * MAXPATHLEN );
  if (getcwd(pathname, MAXPATHLEN) == NULL )
  {
    int errsv = errno;
      switch(errsv)
      {
        case(ERANGE) :  printf("Too Long pathname\n");
                break ;
      case(EACCES) :  printf("Permission denied\n");
                break ;
        case(EFAULT) :  printf("Memory points to bad address\n");
                break ;
        case(EINVAL) :  printf("The size argument is zero and buf is not a null pointer\n");
                break ;
        case(ENOMEM) :  printf("Out of memory\n");
                break ;
        case(ENOENT) :  printf("The current working directory has been unlinked\n");
                break ;
        default :   printf("Error getting pathname\n");
              break; 
      }
    exit(0);
  }
  if( strcmp(buf,"cwd") == 0 )
  {
    printf("Current Working Directory = %s\n",pathname);
    return ;
  }
  else
  {

  }
  count = scandir(pathname, &files, file_select, alphasort);
  // printf("0.\tExit\n");
  printf("1.\t..(parent directory)\n");
  for (i=2; i<=count+1 ; i++ )
  {
    struct stat sb;
    char temp[MAXPATHLEN];
    temp[0] = '\0';
    strcat(temp,pathname);
    strcat(temp,"/\0");
    strcat(temp,files[i-2]->d_name);
    printf("%d.\t%s",i,files[i-2]->d_name);
    if (stat(temp, &sb) == -1)\
    {
       perror("stat");
       printf("\n");
       exit(EXIT_FAILURE);
    }
    else
    {
      switch (sb.st_mode & S_IFMT) 
      {
        case S_IFBLK:  printf("(block device)\n");
                      break;
        case S_IFCHR:  printf("(character device)\n");
                      break;
        case S_IFDIR:  printf("(directory)\n");
                      break;
        case S_IFIFO:  printf("(FIFO/pipe)\n");
                      break;
        case S_IFLNK:  printf("(symlink)\n");
                      break;
        case S_IFREG:  printf("(regular file)\n");
                      break;
        case S_IFSOCK: printf("(socket)\n");
                      break;
        default:       printf("(unknown?)\n"); 
                      break;
    }
  }
} 



int file_select(struct direct *entry)
{
  if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0))
    return (FALSE);
  else
    return (TRUE);
}