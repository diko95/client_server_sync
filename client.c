#include<stdio.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>

#define RCVBUFSIZE 32
#define SNDBUFSIZE 100


void DieWithError(char *errorMessage)
{
 perror(errorMessage);
 exit(1);
}

int main(int argc,char *argv[])
{
 int sock;
 struct sockaddr_in echoServAddr;
 unsigned short echoServPort;
 char *servIP;
 char *echoString;
 char echoBuffer[RCVBUFSIZE];
 char dataBuffer[SNDBUFSIZE];
 unsigned int echoStringLen;
 int bytesRcvd,totalBytesRcvd;
 if((argc<3) || (argc<4))
 {
   fprintf(stderr,"Usage:%s <ServerIP> <Echo word> [<echo port>]\n",argv[0]);
   exit(1);
 }

 servIP = argv[1];
 echoString = argv[2];

 if(argc==4)
   echoServPort = atoi(argv[3]);
 else
   echoServPort = 7;
 if((sock = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP)) < 0)
    DieWithError("socket() failed");

 
 memset(&echoServAddr,0,sizeof(echoServAddr));
 echoServAddr.sin_family = AF_INET;
 echoServAddr.sin_addr.s_addr = inet_addr(servIP);
 echoServAddr.sin_port = htons(echoServPort);

 
  echoStringLen = sizeof(echoBuffer);
 //datastring_len=strlen(datastring);

 while(1)
 {
   if(connect(sock,(struct sockaddr *) &echoServAddr,sizeof(echoServAddr)) < 0)
       DieWithError("connect() failed");

  
   //printf("what do you want?");
   //fgets(echoBuffer,RCVBUFSIZE,stdin);
 
   if(send(sock,echoBuffer,echoStringLen,0)!=echoStringLen)
      DieWithError("send() sent a different no of bytes than expected");

   totalBytesRcvd = 0;
   printf("recieved current_time:");
   while(totalBytesRcvd < sizeof(dataBuffer) )
   {
      if((bytesRcvd = recv(sock, dataBuffer , RCVBUFSIZE - 1,0)) <= 0)
          DieWithError("recv() failed");
      totalBytesRcvd += bytesRcvd;
      dataBuffer[bytesRcvd] = '\0';
    
      printf(dataBuffer);
      sleep(1);
   }
 }
 //printf("/n");
 close(sock);
 exit(0);
}
