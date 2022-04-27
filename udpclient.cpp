/*
** broadcaster.c -- 一個類似 talker.c 的 datagram "client"，
** 差異在於這個可以廣播
*/
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
#include <iostream>
#include <signal.h> 
#include <sys/ioctl.h>   
#include <linux/if.h>  
#define SERVERPORT 4950 // 所要連線的 port
#define MYPORT "4950" // 使用者所要連線的 port
#define MAXBUFLEN 100

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

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
int main()
{
	char szIP[16] = {0};
	GetLocalIP(szIP);
  char ipNum=szIP[7];
	//printf("IP [%s]\n", szIP);
	//printf("i=%c \n",szIP[7]);
  int sockfd;
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_in their_addr; // 連線者的位址資訊
  struct sockaddr_storage recv_addr;
  socklen_t addr_len;
  struct hostent *he;
  int rv;
  char buf[MAXBUFLEN];
  char s[INET6_ADDRSTRLEN];
  int numbytes;
  int broadcast = 1;
  //char broadcast = '1'; // 如果上面這行不能用的話，改用這行

  /*if (argc != 3) {
    fprintf(stderr,"usage: broadcaster hostname message\n");
    exit(1);
  }*/

  if ((he=gethostbyname("10.255.255.255")) == NULL) { // 取得 host 資訊
    perror("gethostbyname");
    exit(1);
  }

  /*if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    perror("socket");
    exit(1);
  }*/

 //create socket and bind
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
 // printf("listener: waiting to recvfrom...\n");

  // 這個 call 就是要讓 sockfd 可以送廣播封包
  if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast,
    sizeof broadcast) == -1) {
    perror("setsockopt (SO_BROADCAST)");
    exit(1);
  }

  their_addr.sin_family = AF_INET; // host byte order
  their_addr.sin_port = htons(SERVERPORT); // short, network byte order
  their_addr.sin_addr = *((struct in_addr *)he->h_addr);
  //their_addr.sin_addr.s_addr=htonl(INADDR_BROADCAST);
  memset(their_addr.sin_zero, '\0', sizeof their_addr.sin_zero);
  
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
 /* char test[]="hihi";
  numbytes = sendto(sockfd, test, strlen(test), 0,
    	(struct sockaddr *)&their_addr, sizeof their_addr);*/
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
    	(struct sockaddr *)&their_addr, sizeof their_addr)) == -1) {

    	perror("talker: sendto");
    	exit(1);
  	}
  	//freeaddrinfo(servinfo);
  	//printf("talker: sent %d bytes to %s\n", int(strlen(message)), message);
        delete []message;
  
    }
    else if(FD_ISSET(sockfd,&read_fds))
    {
	addr_len=sizeof recv_addr;
  	if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, 
        (struct sockaddr *)&recv_addr, &addr_len)) == -1) {

    	perror("recvfrom");
    	exit(1);
     	}

  	

  	inet_ntop(recv_addr.ss_family,

  	get_in_addr((struct sockaddr *)&recv_addr), s, sizeof s);

  	//printf("listener: packet is %d bytes long\n", numbytes);

  	buf[numbytes] = '\0';
	if(s[7]!=ipNum)
  	printf("%s \n", buf);
     	}

  }
  close(sockfd);
  return 0;
  /*char test[]="hiihi";
  if ((numbytes=sendto(sockfd,test, strlen(test), 0,
          (struct sockaddr *)&recv_addr, sizeof recv_addr)) == -1) {
    perror("sendto");
    exit(1);
    }

  printf("sent %d bytes to %s\n", numbytes,
      inet_ntoa(their_addr.sin_addr));

  close(sockfd);

  return 0;*/
}
