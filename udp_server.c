#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#define PORT1 15555
#define PORT2 14444
#define Number_of_Ports 2
int main(){
	int sfd[Number_of_Ports], nsfd;
	struct sockaddr_in addr1,addr2,client;
	int op=1;
	socklen_t addrlen = sizeof(addr1);
	socklen_t clientlen = sizeof(client);
	char buff[1024] = { 0 };
	char * hello = "hello from the server!\n";
	
	//creating socket file descriptor
	if((sfd[0] = socket(AF_INET, SOCK_DGRAM,0)) < 0){
		perror("Socket failed");
		exit(EXIT_FAILURE);
	}
	
	if((sfd[1] = socket(AF_INET, SOCK_DGRAM,0)) < 0){
		perror("Socket failed");
		exit(EXIT_FAILURE);
	}
	
	addr1.sin_family = AF_INET;
	addr1.sin_addr.s_addr = INADDR_ANY;
	addr1.sin_port = htons(PORT1);
	
	addr2.sin_family = AF_INET;
	addr2.sin_addr.s_addr = INADDR_ANY;
	addr2.sin_port = htons(PORT2);
	
	if(bind(sfd[0],(struct sockaddr*)&addr1,addrlen) < 0 ){
		perror("Binding Failed");
		exit(EXIT_FAILURE);
	}
	if(bind(sfd[1],(struct sockaddr*)&addr2,addrlen) < 0 ){
		perror("Binding Failed");
		exit(EXIT_FAILURE);
	}
	
	
	int n = recvfrom(sfd[0],buff,1024,0,(struct sockaddr*)&client, &clientlen);
	
	printf("Received data from IP: %s, Port: %d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
	return 0;	
}

