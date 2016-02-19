
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
//define the IP address as localhost
#define SRV_IP "127.0.0.1"
int get_clientfb(char * servername, int serverport);

int main (int argc, char **argv)
{
  char* servername="";
  char *DATA;
  int UDPport = 0;
  int UDPfb;
  int serverport = 0;
  char* pathname ="";
  int clientfb = 0;
  int clientfb2 = 0; //send two http request
  char buffer[1024] = "GET ";
  char buffer_in[1024] = "";
  char buffer_in2[1024] = "";
  FILE *f = fopen("httpreply1.txt", "w");
  if (f == NULL)
    {
      printf("Error opening file!\n");
      exit(1);
    }
  FILE *f2 = fopen("httpreply2.txt", "w");
  if (f2 == NULL)
    {
      printf("Error opening file!\n");
      exit(1);
    }

  //input should be <servername> <serverport> <pathname>
  if (argc<4)
    {
      printf("Give a correct input\n");
      return(1);
    }
  servername = argv[1];
  serverport = atoi(argv[2]); //convert string to int
  pathname = argv[3];
  UDPport = atoi(argv[4]);
  DATA = argv[5];
  //pass value to client and get the feedback
  clientfb = get_clientfb(servername,serverport);
  clientfb2 = get_clientfb(servername,serverport);

  //client TCP connection established
  //Send the GET command
  strcat(buffer, pathname);
  strcat(buffer," HTTP/1.0\r\n\r\n");
  send(clientfb,buffer,strlen(buffer),0);
  send(clientfb2,buffer,strlen(buffer),0);
  //receive file and write to stdout
  int temp;
  memset(buffer_in, '\0',sizeof(buffer_in));
  //sever part or buffer not clear
  while((temp = recv(clientfb,buffer_in,1023,0)) > 0)
    {
      buffer_in[temp] = '\0';
      fputs(buffer_in,f);
      memset(buffer_in,'\0',sizeof(buffer_in));
    }
  memset(buffer_in,0,sizeof(buffer_in));
  memset(buffer_in2,0,sizeof(buffer_in2));
  temp = 0;
  int ct = 0;
  while((temp = recv(clientfb2,buffer_in2,1023,0)) > 0)
    {
      buffer_in2[temp] = '\0';
      fputs(buffer_in2,f2);
      memset(buffer_in2,'\0',sizeof(buffer_in2));
    }
  memset(buffer_in2,0,sizeof(buffer_in2));
  printf("UDP info is :");
  UDPfb = get_UDPfb(servername,UDPport,DATA);
  close(UDPfb);
  close(clientfb);
  close(clientfb2);
  fclose(f);
  fclose(f2);
  return 0;
}
//create UDP
int get_UDPfb(char* servername, int serverport,char *DATA)
{
  char buffer[1024] = "";
  int client_socket = 0;
  struct hostent *hent;
  struct sockaddr_in remote;
  //check the socket build
  if ((client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))<0)
    {
      printf("Cannot create client socket\n");
      return -1;
    }
  //check servername
  if((hent = gethostbyname(servername)) == NULL)
    {
      printf("Cannot get IP\n");
      return -1;
    }
  //bind the socket (usually do not need to do this
  /*
  if (bind(client_socket,(struct sockaddr*)remote,sizeof(struct sockaddr))<0)
    {
      printf("bind fails\n");
      return -1;
    }
  */
  memset((char *) &remote, 0 , sizeof(remote));
  remote.sin_family = AF_INET;
  //from host format to network format
  remote.sin_port = htons(serverport);
  int len = sizeof(remote);
  if (inet_aton(SRV_IP,&remote.sin_addr) == 0)
    {
      printf("inet_aton error\n");
      return -1;
    }
  int i = 0;
  int j = 0;
  char seq[4] = "";
  int seq1 = 0;

  //continues ping four times
  //if sending empty packet, there will be seq problem
  //DATA = "";
  //divide data to two parts 
  //ntohl!
  while ( i < 4)
    {
      if(sendto(client_socket,DATA,strlen(DATA),0,(struct sockaddr *)&remote,len) < 0)
	{
	  printf("sending error");
	  return -1;
	}
      for(j = 0;j<=3;j++) seq[j] = DATA[j];
      seq1 = *(int *)seq;
      seq1 = htonl(seq1);
      printf("sending sequence is :%d\nsending msg is: %s\n",seq1,DATA);
      i++;
      if (recvfrom(client_socket,buffer,1024,0,(struct sockaddr *)&remote, &len)<0)
	{
	  printf("rcv error");
	  return -1;
	}
      for(j = 0;j<=3;j++) seq[j] = buffer[j];
      seq1 = *(int *)seq;
      seq1 = ntohl(seq1);
      buffer[3] = buffer[3] - 1;
      printf("rcv sequence is :%d\n",seq1);
      puts("msg is :");
      puts(buffer);
      puts("\n");
    }
  return client_socket;
}




int get_clientfb(char* servername, int serverport)
{
  int client_socket = 0;
  struct hostent *hent;
  struct sockaddr_in *remote;
  //check the socket build
  if ((client_socket = socket(AF_INET, SOCK_STREAM, 0))<0)
    {
      printf("Cannot create client socket\n");
      return -1;
    }
  //check IP
  if((hent = gethostbyname(servername)) == NULL)
    {
      printf("Cannot get IP\n");
      return -1;
    }
  remote = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in *));
  remote->sin_family = AF_INET;
  bcopy((char*)hent->h_addr,(char*)&remote->sin_addr.s_addr,hent->h_length);
  //from host format to network format
  remote->sin_port = htons(serverport);
  //check the connect
  if (connect(client_socket,(struct sockaddr *)remote, sizeof(struct sockaddr)) < 0)
    {
      printf("Connection fails\n");
      return -1;
    }
  free(remote);
  return client_socket;
}


