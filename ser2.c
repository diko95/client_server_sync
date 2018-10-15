#include<stdio.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/time.h>
#include<fcntl.h>
#define MAXPENDING 5
#define RCVBUFSIZE 32
#define SNDBUFSIZE 100

void DieWithError(char *errorMessage)
{
 perror(errorMessage);
 exit(1);
}

void HandleTCPClient(int clntSocket)
{
 char echoBuffer[RCVBUFSIZE];
 int recvMsgSize;
 char dataBuffer[SNDBUFSIZE];


 

 
 
 
 FILE *pipe;
 pipe = popen("date", "r");
 if (pipe == NULL)
     return -1;
 while (fgets(dataBuffer, sizeof(dataBuffer), pipe) != NULL)
     fprintf(stdout, "%s", dataBuffer);
 pclose(pipe);
 
 if((recvMsgSize = recv(clntSocket,echoBuffer,RCVBUFSIZE,0)) < 0)
     DieWithError("recv() failed ");


 while(recvMsgSize > 0)
 {
  if(send(clntSocket,dataBuffer,SNDBUFSIZE,0) != SNDBUFSIZE)
     DieWithError("send() failed");

   if((recvMsgSize = recv(clntSocket,echoBuffer,RCVBUFSIZE,0)) < 0)
       DieWithError("recv() failed");
 }
 close(clntSocket);
}

int CreateTCPServerSocket(unsigned short port)
{
 int sock;
 struct sockaddr_in echoServAddr;
 
 if((sock = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP)) < 0)
    DieWithError("socket() failed");
 
 memset(&echoServAddr,0,sizeof(echoServAddr));
 echoServAddr.sin_family = AF_INET;
 echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
 echoServAddr.sin_port = htons(port);
 
 if(bind(sock,(struct sockaddr *) &echoServAddr,sizeof(echoServAddr)) < 0)
    DieWithError("bind() failed");
 
 if(listen(sock,MAXPENDING) < 0)
    DieWithError("listen() failed");

 return sock;
}

int AcceptTCPConnection(int servSock)
{
 int clntSock;
 struct sockaddr_in echoClntAddr;
 unsigned int clntLen;

 clntLen = sizeof(echoClntAddr);

 if((clntSock = accept(servSock,(struct sockaddr *) &echoClntAddr,&clntLen)) < 0)
     DieWithError("accept() failed");

 printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));

 return clntSock;
}

   
int main(int argc,char *argv[])
{
 int *servSock;
 int maxDescriptor;
 fd_set sockSet;
 long timeout;
 struct timeval selTimeout;
 int running = 1;
 int noPorts;
 int port;
 unsigned short portNo;
 
 if(argc<3)
 {
   fprintf(stderr,"Usage:%s <Timeout (secs.)> <Port 1> ...\n",argv[0]);
   exit(1);
 }

 timeout = atol(argv[1]);
 noPorts = argc - 2;
 
 servSock = (int *) malloc(noPorts * sizeof(int));
 maxDescriptor = -1;

 for(port = 0; port < noPorts;port++)
 {
   portNo = atoi(argv[port + 2]);
   servSock[port] = CreateTCPServerSocket(portNo);
   
   if(servSock[port] > maxDescriptor)
      maxDescriptor = servSock[port];
 }
 

 printf("Starting server: Hit return to shutdown\n");
 while(running)
 {
   FD_ZERO(&sockSet);
   FD_SET(STDIN_FILENO,&sockSet);
   for(port = 0; port < noPorts;port++)
       FD_SET(servSock[port], &sockSet);

   selTimeout.tv_sec = timeout;
   selTimeout.tv_usec = 0;
   
   if(select(maxDescriptor + 1,&sockSet,NULL,NULL,&selTimeout) == 0)
      printf("No echo requests for %ld secs....Server still alive\n",timeout);
   else
   {
     if(FD_ISSET(STDIN_FILENO,&sockSet))
     {
       printf("shutting down server\n");
       getchar();
       running = 0;
     }
     for(port=0;port < noPorts;port++)
         if(FD_ISSET(servSock[port], &sockSet))
         {
           printf("request on port %d: ", port);
           HandleTCPClient(AcceptTCPConnection(servSock[port]));
         }
    }
  }

  for(port = 0;port < noPorts;port++)
     close(servSock[port]);
  free(servSock);

  exit(0);
}   
