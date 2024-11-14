#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#define PORT1 14532
#define PORT2 15555

#define Number_of_Ports 2
int main(){
	int sfd[Number_of_Ports], nsfd;
	struct sockaddr_in addr1,addr2;
	int op=1;
	socklen_t addrlen = sizeof(addr1);
	char buff[1024] = { 0 };
	char * hello = "hello from the server!\n";
	
	//creating socket file descriptor
	if((sfd[0] = socket(AF_INET, SOCK_STREAM,0)) < 0){
		perror("Socket failed");
		exit(EXIT_FAILURE);
	}
	
	if((sfd[1] = socket(AF_INET, SOCK_STREAM,0)) < 0){
		perror("Socket failed");
		exit(EXIT_FAILURE);
	}
	
	addr1.sin_family = AF_INET;
	addr1.sin_addr.s_addr = INADDR_ANY;
	addr1.sin_port = htons(PORT1);
	
	addr2.sin_family = AF_INET;
	addr2.sin_addr.s_addr = INADDR_ANY;
	addr2.sin_port = htons(PORT2);
	
	//Socket Binding to their respective addresses
	if(bind(sfd[0], (struct sockaddr*) &addr1, sizeof(addr1)) < 0){
		perror("binding failed");
		exit(EXIT_FAILURE);
	}
	
	if(bind(sfd[1], (struct sockaddr*) &addr2, sizeof(addr2)) < 0){
		perror("binding failed");
		exit(EXIT_FAILURE);
	}
	
	//At max how many ports can sfd listen on
	if (listen(sfd[0], 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if (listen(sfd[1], 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
	
	struct pollfd arr[2];
	arr[0].fd = sfd[0]; arr[1].fd = sfd[1];
	arr[0].events = POLLIN; arr[1].events = POLLIN;
	while(1)
	{
		poll(arr,2,1000);
		for(int i=0;i<2;i++){
			if(arr[i].revents & POLLIN)
			{
				struct sockaddr_in client;
				client.sin_family = AF_INET;
				client.sin_addr.s_addr = INADDR_ANY;
				socklen_t clientlen = sizeof(client);  
				nsfd = accept(sfd[i], (struct sockaddr*) &client, &clientlen);
					if (nsfd < 0) {
						perror("Accepting failed");
						exit(EXIT_FAILURE);
					}
				printf("Connection %d has been established. \n", i+1);
				printf("Connection with %d\n", client.sin_port);
			
                    ssize_t sz = recv(nsfd, buff, sizeof(buff), 0);
                    if (sz < 0) {
                        perror("recv failed");
                        close(nsfd);
                        exit(EXIT_FAILURE);
                    }

                    buff[sz] = '\0';
                    printf("Server received from tcp: %s\n", buff);

                    if (send(nsfd, "from_s3", 3, 0) < 0) {
                        perror("send failed");
                        close(nsfd);
                        exit(EXIT_FAILURE);
                    }
                close(sfd[i]);
                return 0;
			}
		}	
	}
}