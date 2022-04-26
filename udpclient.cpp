#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include<iostream>
#include <signal.h> 
#include <sys/ioctl.h>   
#include <linux/if.h>
#define MYPORT "4950" // 使用者所要連線的 port
#define MAXBUFLEN 100
#define SERVERPORT "4950" // 使用者所要連線的 port

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
// udpSever

int server()
{
  int sockfd;
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage addrList[5];
  int addrSizeList[5];
  int rv;
  int numbytes;
  struct sockaddr_storage their_addr;
  char buf[MAXBUFLEN];
  socklen_t addr_len;
  char s[INET6_ADDRSTRLEN];

  memset(&hints, 0, sizeof hints);

  hints.ai_family = AF_UNSPEC; // 設定 AF_INET 以強制使用 IPv4
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE; // 使用我的 IP

  if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  // 用迴圈來找出全部的結果，並 bind 到首先找到能 bind 的
  for(p = servinfo; p != NULL; p = p->ai_next) {

    if ((sockfd = socket(p->ai_family, p->ai_socktype,
      p->ai_protocol)) == -1) {
      perror("listener: socket");
      continue;
    }

    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("listener: bind");
      continue;
    }

    break;
  }

  if (p == NULL) {
    fprintf(stderr, "listener: failed to bind socket\n");
    return 2;
  }

  freeaddrinfo(servinfo);
  //printf("listener: waiting to recvfrom...\n");
  addr_len = sizeof their_addr;
  
//change
  fd_set master;
  fd_set read_fds;
  int fdmax;
  FD_ZERO(&master);
  FD_ZERO(&read_fds);
  FD_SET(0,&master);
  FD_SET(0,&read_fds);
  FD_SET(sockfd,&read_fds);
  FD_SET(sockfd,&master);
//change done
  while(true)
{ 
  read_fds=master;
  int ret=select(sockfd+1,&read_fds,NULL,NULL,NULL);
  if(FD_ISSET(sockfd,&read_fds))
  {
  if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, 
        (struct sockaddr *)&their_addr, &addr_len)) == -1) {

    perror("recvfrom");
    exit(1);
     }

  //printf("listener: got packet from %s\n",

  inet_ntop(their_addr.ss_family,

  get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);

  //printf("listener: packet is %d bytes long\n", numbytes);
  int from=s[7]-'0';
  if(buf[0]=='H')
  {
  addrList[from]=their_addr;
  addrSizeList[from]=addr_len;
  }
  else {
  int des=buf[4]-'0';
  
  buf[numbytes] = '\0';

  //printf("listener: packet contains \"%s\"\n", buf);
  char bufSend[100];
  int nbytes=numbytes;
  for(int sIndex=6;sIndex<nbytes;sIndex++)
		bufSend[sIndex-6]=buf[sIndex];
		bufSend[nbytes-6]=' ';
		bufSend[nbytes-5]='F';
		bufSend[nbytes-4]='r';
		bufSend[nbytes-3]='o';
		bufSend[nbytes-2]='m';
		bufSend[nbytes-1]=' ';
		bufSend[nbytes]='h';
		bufSend[nbytes+1]=from+'0';
		bufSend[nbytes+2]='\0';
  if(from!=des)
    {
	if(des==1) std::cout<<bufSend<<std::endl;
	else 
	{
	     if ((numbytes = sendto(sockfd, bufSend, strlen(bufSend)+1, 0,
    (sockaddr*)&addrList[des], sizeof(addrList[des]))) == -1) {

    perror(" sever:sendto");
    exit(1);
  		}

  	//freeaddrinfo(servinfo);
  	//printf("sever: sent %d bytes to %s\n", int(strlen(bufSend)), bufSend);

	}
    }
   }

  }
 else if(FD_ISSET(0,&read_fds))
  {
	char* message=new char[100];
  	std::cin.getline(message,99);
  	int len,bytes_sent;
  	len=strlen(message);
	int des=message[4]-'0';
	//std::cout<<"des  "<<des<<std::endl;
	int from =1;
	char*messageSend=new char[100];
	int myIn;
	for( myIn=6;myIn<len;myIn++)
		messageSend[myIn-6]=message[myIn];

		messageSend[myIn-6]=' ';
		messageSend[myIn-5]='F';
		messageSend[myIn-4]='r';
		messageSend[myIn-3]='o';
		messageSend[myIn-2]='m';
		messageSend[myIn-1]=' ';
		messageSend[myIn]='h';
		messageSend[myIn+1]=1+'0';
		messageSend[myIn+2]='\0';
	delete []message;
	  if(from!=des)
    {
	//if(des==1) std::cout<<bufSend<<std::endl;
	
	     if ((numbytes = sendto(sockfd, messageSend, strlen(messageSend)+1, 0,
    (sockaddr*)&addrList[des], sizeof(addrList[des]))) == -1) {

    perror(" sever:sendto");
    exit(1);
  		}

  	//freeaddrinfo(servinfo);
  	//printf("sever: sent %d bytes to %s\n", int(strlen(messageSend)), messageSend);

	delete []messageSend;
    }

  }

}
  close(sockfd);

  return 0;
}

//udp client
int client()
{
  int sockfd;
  struct addrinfo hints, *servinfo, *p;
  int rv;
  int numbytes;
  char buf[MAXBUFLEN];
  char s[INET6_ADDRSTRLEN];
  struct sockaddr_storage their_addr;
  socklen_t addr_len;

  /*if (argc != 3) {
    fprintf(stderr,"usage: talker hostname message\n");
    exit(1);
  }*/

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;

  if ((rv = getaddrinfo("10.0.0.1", SERVERPORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  // 用迴圈找出全部的結果，並產生一個 socket
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype,
      p->ai_protocol)) == -1) {
      perror("talker: socket");
      continue;
    }

    break;
  }

  if (p == NULL) {
    fprintf(stderr, "talker: failed to bind socket\n");
    return 2;
  }

  //change
  fd_set master;
  fd_set read_fds;
  int fdmax;
  FD_ZERO(&master);
  FD_ZERO(&read_fds);
  FD_SET(0,&master);
  FD_SET(0,&read_fds);
  FD_SET(sockfd,&read_fds);
  FD_SET(sockfd,&master);
 char test[]="HHHH";

//change done

  if ((numbytes = sendto(sockfd, test, strlen(test), 0,
    p->ai_addr, p->ai_addrlen)) == -1) {

    perror("talker: sendto");
    exit(1);
  }

  freeaddrinfo(servinfo);
  //printf("talker: sent %d bytes to %s\n", numbytes, test);
  while(true)
  { 
    read_fds=master;
    int ret=select(sockfd+1,&read_fds,NULL,NULL,NULL);
    if(FD_ISSET(0,&read_fds))
    {
	  char* message=new char[100];
  	std::cin.getline(message,99);
  	int len,bytes_sent;
  	len=strlen(message);
	if ((numbytes = sendto(sockfd, message, strlen(message), 0,
    	p->ai_addr, p->ai_addrlen)) == -1) {

    	perror("talker: sendto");
    	exit(1);
  	}
  	//freeaddrinfo(servinfo);
  	//printf("talker: sent %d bytes to %s\n", int(strlen(message)), message);
        delete []message;
  
    }
    else if(FD_ISSET(sockfd,&read_fds))
    {
  	if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, 
        (struct sockaddr *)&their_addr, &addr_len)) == -1) {

    	perror("recvfrom");
    	exit(1);
     	}

  	//printf("listener: got packet from %s\n",

  	inet_ntop(their_addr.ss_family,

  	get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);

  	printf(" %s \n", buf);
     	}

  }
  close(sockfd);
  return 0;
}

int GetLocalIP(char *ip)
{
	int  MAXINTERFACES = 16;
	int fd, intrface = 0;
	struct ifreq buf[MAXINTERFACES]; ///if.h   
	struct ifconf ifc; ///if.h   
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0) //socket.h   
	{
		ifc.ifc_len = sizeof buf;
		ifc.ifc_buf = (caddr_t)buf;
		if (!ioctl(fd, SIOCGIFCONF, (char *)&ifc)) //ioctl.h   
		{
			intrface = ifc.ifc_len / sizeof (struct ifreq);
			while (intrface-- > 0)
			{
				if (!(ioctl(fd, SIOCGIFADDR, (char *)&buf[intrface])))
				{
					sprintf(ip, "%s", inet_ntoa(((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr));
					//printf("ip:%s\n", ip);
					break;
				}
 
			}
		}
		close(fd);
	}
	return 0;
}


int main(void)
{
  char szIP[16] = {0};
	GetLocalIP(szIP);
  if(szIP[7]=='1') server();
  else client();
}
