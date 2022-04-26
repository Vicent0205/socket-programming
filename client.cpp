/*
** final tcp 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h> 
#include <sys/ioctl.h>   
#include <linux/if.h> 
#include <netdb.h>
#include <iostream>
#include <unordered_map>
using namespace std;
#define PORT "9034" // 我們正在 listen 的 port
#define MAXDATASIZE 100 // 我們一次可以收到的最大位原組數（number of bytes）
// 取得 sockaddr，IPv4 或 IPv6：
void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int client()
{
  char argv[]="10.0.0.1";
  int sockfd, numbytes;
  char buf[MAXDATASIZE];
  struct addrinfo hints, *servinfo, *p;
  int rv;
  char s[INET6_ADDRSTRLEN];
/*
  if (argc != 2) {
    fprintf(stderr,"usage: client hostname\n");
    exit(1);
  }
*/
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if ((rv = getaddrinfo("10.0.0.1", PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  // 用迴圈取得全部的結果，並先連線到能成功連線的
   for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype,
      p->ai_protocol)) == -1) {
      perror("client: socket");
      continue;
    }

    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("client: connect");
      continue;
    }

    break;
  }

  if (p == NULL) {
    fprintf(stderr, "client: failed to connect\n");
    return 2;
  }

  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);

  //printf("client: connecting to %s\n", s);
  fd_set master;
  fd_set read_fds;
  int fdmax;
  
  FD_ZERO(&master);
  FD_ZERO(&read_fds);
  FD_SET(0,&master);
  FD_SET(0,&read_fds);
  FD_SET(sockfd,&read_fds);
  FD_SET(sockfd,&master);
  while(true)
{
  read_fds=master;
  int ret=select(sockfd+1,&read_fds,NULL,NULL,NULL);
  if(FD_ISSET(sockfd,&read_fds))
  {
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
      perror("recv");
      exit(1);
    }

    buf[numbytes] = '\0';
    std::cout<<buf<<std::endl;
  }
  if(FD_ISSET(0,&read_fds))
{
  char* message=new char[100];
  std::cin.getline(message,99);
  int len,bytes_sent;
  len=strlen(message);
  int bytes_send=send(sockfd,message,len,0);
  //std::cout<<"done  "<<bytes_send<<std::endl;
  
  //freeaddrinfo(servinfo); // 全部皆以這個 structure 完成
}

}
  close(sockfd);

  return 0;
}


int server()
{
  unordered_map<int,int>map;
  fd_set master; // master file descriptor 清單
  fd_set read_fds; // 給 select() 用的暫時 file descriptor 清單
  int fdmax; // 最大的 file descriptor 數目

  int listener; // listening socket descriptor
  int newfd; // 新接受的 accept() socket descriptor
  struct sockaddr_storage remoteaddr; // client address
  socklen_t addrlen;

  char buf[256]; // 儲存 client 資料的緩衝區
  int nbytes;

  char remoteIP[INET6_ADDRSTRLEN];

  int yes=1; // 供底下的 setsockopt() 設定 SO_REUSEADDR
  int i, j, rv;

  struct addrinfo hints, *ai, *p;

  FD_ZERO(&master); // 清除 master 與 temp sets
  FD_ZERO(&read_fds);

  // 給我們一個 socket，並且 bind 它
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
    fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
    exit(1);
  }

  for(p = ai; p != NULL; p = p->ai_next) {
    listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (listener < 0) {
      continue;
    }

    // 避開這個錯誤訊息："address already in use"
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
      close(listener);
      continue;
    }

    break;
  }

  // 若我們進入這個判斷式，則表示我們 bind() 失敗
  if (p == NULL) {
    fprintf(stderr, "selectserver: failed to bind\n");
    exit(2);
  }
  freeaddrinfo(ai); // all done with this

  // listen
  if (listen(listener, 10) == -1) {
    perror("listen");
    exit(3);
  }

  // 將 listener 新增到 master set
  FD_SET(listener, &master);
  FD_SET(0,&master);
  // 持續追蹤最大的 file descriptor
  fdmax = listener; // 到此為止，就是它了

  // 主要迴圈
  for( ; ; ) {
    read_fds = master; // 複製 master

    if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
      perror("select");
      exit(4);
    }

    // 在現存的連線中尋找需要讀取的資料
    for(i = 0; i <= fdmax; i++) {
      if (FD_ISSET(i, &read_fds)) { // 我們找到一個！！
// change
	if(i==0){
	  char* message=new char[100];
  	std::cin.getline(message,99);
  	int len,bytes_sent;
  	len=strlen(message);
	int des=message[4]-'0';
	//cout<<"des  "<<des<<endl;
	
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
	for(int p=1;p<=fdmax;p++)
	{
	  if(des==1) {cout<<messageSend<<endl;break;}
	  if(FD_ISSET(p,&master))
	  {
	    if(map[p]==des)
	    {
		if (send(p, messageSend, myIn+2, 0) == -1) {
                    perror("send");
                  }
	    }
	  }
	}
		
	}

        else if (i == listener) {
          // handle new connections
          addrlen = sizeof remoteaddr;
          newfd = accept(listener,
            (struct sockaddr *)&remoteaddr,
            &addrlen);

          if (newfd == -1) {
            perror("accept");
          } else {
            FD_SET(newfd, &master); // 新增到 master set
            if (newfd > fdmax) { // 持續追蹤最大的 fd
              fdmax = newfd;
            }
		
           
              inet_ntop(remoteaddr.ss_family,
                get_in_addr((struct sockaddr*)&remoteaddr),
                remoteIP, INET6_ADDRSTRLEN);
              
		//cout<<"routerIp"<<remoteIP<<endl;
	       if(remoteIP[7]=='1') map[newfd]=1;
    		else if(remoteIP[7]=='2') 
		{map[newfd]=2;}
    		else if(remoteIP[7]=='3') map[newfd]=3;
    		else if(remoteIP[7]=='4') map[newfd]=4;

          }

        } else {
          // 處理來自 client 的資料
          if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
            // got error or connection closed by client
            if (nbytes == 0) {
              // 關閉連線
              //printf("selectserver: socket %d hung up\n", i);
            } else {
              perror("recv");
            }
            close(i); // bye!
            FD_CLR(i, &master); // 從 master set 中移除

          } else {
            // 我們從 client 收到一些資料
		//cout<<"send i "<<i<<endl; 
		int des=buf[4]-'0';
		//cout<<"des  "<<des<<endl;
		int from=map[i];
		if(des==1)
		 {
			for(int sIndex=6;sIndex<nbytes;sIndex++)
			cout<<buf[sIndex];
			cout<<" From h"<<map[i]<<endl;
		}
		else
	{
            for(j = 0; j <= fdmax; j++) {
              // 送給大家！
              if (FD_ISSET(j, &master)) {
                // 不用送給 listener 跟我們自己
		char bufSend[256];
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
                if (map[j]==des) {
                  if (send(j, bufSend, nbytes+2, 0) == -1) {
                    perror("send");
                  }
                }
              }
            }
	}
          }
        } // END handle data from client
      } // END got new incoming connection
    } // END looping through file descriptors
  } // END for( ; ; )--and you thought it would never end!

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
