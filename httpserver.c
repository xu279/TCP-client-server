#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

int create_server_socket(int server_port);
int create_server_socket_ping(int server_port_ping);

int main(int argc, char **argv)
{
  char buffer_in[1024] = "";
  char ping_buffer[1024] = "";
  int server_socket = 0;
  int server_socket_ping = 0;
  int new_server_socket = 0;
  int server_port = 0;
  int server_port_ping = 0;
  socklen_t client_addrlen;
  struct sockaddr_in client_address;
  struct hostent *hosten;
  char *dotip;
  char *arg;
  int temp,temp2;
  server_port = atoi(argv[1]);    // HTTP port
  server_port_ping = atoi(argv[2]); //UDP port 
  server_socket = create_server_socket(server_port);
  server_socket_ping = create_server_socket_ping(server_port_ping);

  //socket built, bind succ
  //always listening and waiting for client connection
  

  fd_set fds;
  int select_res,max;
  while(1)
    {
      //select(event multiplexer) between HTTP and UDP
      FD_ZERO(&fds);
      FD_SET(server_socket,&fds);
      FD_SET(server_socket_ping,&fds);
      //printf("%d,%d\n",server_socket_ping,server_socket);
      if (server_socket >= server_socket_ping)
	{
	  max = server_socket +1;
	}
      else 
	{
	  max = server_socket_ping +1;
	}
      select_res = select(max,&fds,NULL,NULL,NULL);

      //select HTTP
      if(FD_ISSET(server_socket,&fds))
	{
	  client_addrlen = sizeof(client_address);
	  if((new_server_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_addrlen)) < 0)
	    {
	      //printf("server accept fails");
	      return -1;
	    }
	  //inform user
	  //printf("New connection , socket fd is %d , ip is : %d , port : %d \n" , new_server_socket , inet_ntoa(client_address.sin_addr) , ntohs(client_address.sin_port));

	  //reset buffer
	  memset(buffer_in,0,sizeof(buffer_in));
	  if (fork() == 0)
	    {
	      close(server_socket);
	      //connect succ
	      if(read(new_server_socket,buffer_in,sizeof(buffer_in))!=0)
		{
		  //split incoming request by " "
		  arg = strtok(buffer_in," ");
		  if(!strcmp(arg,"GET"))
		    {
		      //get the path
		      arg = strtok(NULL, " ");
		      char *inputfile;
		      strcpy(inputfile,arg);
		      temp = open(arg, O_RDONLY);

		      memset(buffer_in,0,sizeof(buffer_in));
		      if(errno == ENOENT)
			{
			  strcpy(buffer_in,"HTTP/1.0 404 Not Found\r\n\r\n");
			  write(new_server_socket,buffer_in,sizeof(buffer_in));
			}
		      else if(errno == EACCES)
			{
			  strcpy(buffer_in,"HTTP/1.0 403 Forbidden\r\n\r\n");
			  write(new_server_socket,buffer_in,sizeof(buffer_in));
			}
		      //access succ
		      else 
			{
			  close(temp);
			  strcpy(buffer_in,"HTTP/1.0 200 OK\r\n\r\n");
			  write(new_server_socket,buffer_in,strlen(buffer_in));
			  memset(buffer_in,'\0',sizeof(buffer_in));
			  //write to the client
			  //Open file
			  FILE *f1 = NULL;
		     	  f1 = fopen(inputfile,"r");
			  if (f1 == NULL) {
				printf("File open error\n");
				return -1;
			  }
			   while (!feof(f1))
			    {
			      int rval = fread(buffer_in, 1, sizeof(buffer_in), f1);
			      write(new_server_socket,buffer_in,rval);
			      memset(buffer_in,'\0',sizeof(buffer_in));
			    }
			  close(temp);
			  fclose(f1);
			}
		    }
		}
	    
	    }
	  close(new_server_socket);
    
	}

      //select UDP
      //print seq number
      else if (FD_ISSET(server_socket_ping,&fds))
	{
	  client_addrlen = sizeof(client_address);
	  ssize_t recvlen; //# bytes received
	  ssize_t temp = 0;
	  recvlen = recvfrom(server_socket_ping,ping_buffer,1024,0,(struct sockaddr *)&client_address, &client_addrlen);
	  ping_buffer[3]++;
	  temp = sendto(server_socket_ping,ping_buffer,recvlen,0,(struct sockaddr *)&client_address,client_addrlen);
	  if ( temp != recvlen)
	    {
	      printf("recv error\n");

	    }
	}
    }			     

  return 0;
}


//create ping socket
int create_server_socket_ping(int server_port_ping)
{
  int server_socket_ping = 1;
  int test = 0;
  struct sockaddr_in server_address_ping;
  //create server socket ping
  if((server_socket_ping = socket(AF_INET, SOCK_DGRAM,0)) < 0)
    {
      printf("Socket create fails\n");
      return -1;
    }
  //empty server_address_ping before using it
  bzero((char *) &server_address_ping, sizeof(server_address_ping));
  server_address_ping.sin_family = AF_INET;
  server_address_ping.sin_addr.s_addr = htonl(INADDR_ANY); 
  server_address_ping.sin_port = htons((unsigned short)server_port_ping); 
  //bind server socket ping
  if((test = bind(server_socket_ping,(struct sockaddr *)&server_address_ping, sizeof(server_address_ping)))<0)
    {
      printf("bind Socket ping fails%d\n",test);
      return -1;
    }
  return server_socket_ping;
}
			
int create_server_socket(int server_port)
{
  int server_socket = 1;
  struct sockaddr_in server_address;
  //create server socket
  if((server_socket = socket(AF_INET, SOCK_STREAM,0)) < 0)
    {
      printf("Socket create fails\n");
      return -1;
    }
  //check bind addr
  //empty server_address before using it
  bzero((char *) &server_address, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = htonl(INADDR_ANY); 
  server_address.sin_port = htons((unsigned short)server_port); 
  //bind server socket
  if((bind(server_socket,(struct sockaddr *)&server_address, sizeof(server_address)))<0)
    {
      printf("bind Socket fails\n");
      return -1;
    }
  if(listen(server_socket,10)<0)
    {
      printf("server listen fails");
      return -1;
    }
  return server_socket;
}
