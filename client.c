#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

int main(int argc, char * argv[]){

	int clientSocket, ret;
	struct sockaddr_in serverAddr;
	char buffer[1024];
	clientSocket = socket(AF_INET, SOCK_STREAM, 0);

	char * ip = "127.0.0.1"; //default ip 
	int PORT = 4444; //default port

	if (argc == 3) {
		ip = argv[1];
		PORT = atoi(argv[2]);

	}

	if(clientSocket < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}


	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr(ip);

	ret = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}

	memset(buffer, '\0', 1024);
	if(read(clientSocket, buffer, 1024) < 0) {
		printf("[-]Error in receiving data.\n");
	}
	else {
		printf("%s", buffer);
	}		
	

	while(1){
	
		memset(buffer, '\0', 1024);
		fgets(buffer, 1024, stdin);
		send(clientSocket, buffer, strlen(buffer)-1, 0);

		if(strcmp(buffer, "Q:\n") == 0){
			recv(clientSocket, buffer, 1024, 0);
			printf("%s", buffer);
			close(clientSocket);
			exit(1);
		}

		if (recv(clientSocket, buffer, 1024, 0) < 0) {
			printf("[-]Error in receiving data.\n");
		}
		else {
			printf("%s", buffer);
		}
		
	}

	return 0;  
}

