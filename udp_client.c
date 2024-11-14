#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#define PORT1 55555

int main(){
	int sfd;
	struct sockaddr_in client,server;
	socklen_t addrlen = sizeof(client);
	
	if((sfd = socket(AF_INET,SOCK_DGRAM,0)) < 0){
		perror("Socket failed");
		exit(EXIT_FAILURE);
	}	
		
	client.sin_family = AF_INET;
	client.sin_port = htons(PORT1);
	server.sin_family = AF_INET;
	server.sin_port = htons(15555);
	if (inet_pton(AF_INET, "127.0.0.1", &client.sin_addr)
        <= 0) {
        printf(
            "\nInvalid address/ Address not supported \n");
        return -1;
    }
	if (inet_pton(AF_INET, "127.0.0.1", &server.sin_addr)
        <= 0) {
        printf(
            "\nInvalid address/ Address not supported \n");
        return -1;
    }
	
	
	if(bind(sfd,(struct sockaddr*)&client,addrlen) < 0){
		perror("Binding failed");
		exit(EXIT_FAILURE);
	}
	
	char buff[1024] = "Hello From client1! \n";
	
	if(sendto(sfd,buff,1024,0,(struct sockaddr*)&server, addrlen) < 0){
		perror("Sending failed.");
		exit(EXIT_FAILURE);
	}
	
}