#include <stdio.h>  
#include <string.h>   
#include <stdlib.h>  
#include <errno.h>  
#include <unistd.h>    
#include <arpa/inet.h>    
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <sys/time.h>  
#include <time.h>


    
typedef struct SOCKET {
	int fd;
	struct SOCKET * next;
	int play; 
	int wins;
} SOCKET;

SOCKET* add_socket (SOCKET* list, int fd) {
	SOCKET * s = list;
	SOCKET * socket = (SOCKET *) malloc(sizeof(SOCKET));
	socket->fd = fd;
	socket->next = NULL;
	socket->play = 0;
	socket->wins = 0;
	if (list == NULL) return socket;
	while (s->next != NULL) s = s->next;
	s->next = socket;
	return list;
}


SOCKET* remove_socket (SOCKET* list, SOCKET* s) {
	if (list == s) {
		list = list->next;
		free(s);
		return list;
	}
	for (SOCKET* i = list; i; i=i->next) {
		if (i->next == s) {				
			i->next = s->next;
			free(s);
		}
	}

	return list;
}

char* hide_word(char* epigram, char * correct) {
	char newString[10][10];
	int i=0;

 	char dest[200] = "";
    int j=0; int ctr=0;

	memset(correct, '\0', 20);
    for(i=0;i<=(strlen(epigram));i++)
    {
        // if space or NULL found, assign NULL into newString[ctr]
        if(epigram[i]==' '||epigram[i]=='\0')
        {
            newString[ctr][j]='\0';
            ctr++;
            j=0;    
        }
        else
        {
            newString[ctr][j]=epigram[i];
            j++;
        }
    }

	srand(time(0)); 
	int x = rand()%ctr;
 
	memset(correct, '\0', 20);
	strcpy(correct, newString[x]);
	memset(newString[x], '-', strlen(newString[x]));
	strcat(dest, "C: ");
	for(i=0;i < ctr;i++) {
		strcat(dest, newString[i]);
		
		if(i < ctr-1) strcat(dest, " ");

	}
	if (dest[strlen(dest)-1] != '\n') {
		dest[strlen(dest)-1] = '\n';
	}
	if (correct[strlen(correct)-1] == '\n') {
		correct[strlen(correct)-1] = '\0';
	}
	strcpy(epigram, dest);
	return correct;
}


void get_epigram(FILE *pf, char * epigram, char * command) {
	memset(epigram, '\0', 1024); 
	if (pf) fgets(epigram, 1024 , pf);
	if (pclose(pf) < 0) printf("Error: Failed to close command stream \n");
}

int main(int argc , char *argv[])   { 
    
	FILE * pf;
	char command[20] = "fortune -n 32 -s\n";
	char guess[1024];
   	char epigram[1024];
	char correct[20];
	char msg[100];

	int port = 4444;	   //defualt port if port is not provided form command line

    int master_socket, new_socket, activity, valread, sd, max_sd;   
    struct sockaddr_in serveraddr, newaddr;   

    SOCKET * socket_list = NULL;
	socklen_t addr_size;
     
    fd_set readfds;   

    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)  {   
        perror("socket failed");   
        exit(EXIT_FAILURE);   
    }   
		

	if (argc == 2) {
		port = atoi(argv[1]);
	}
	
    memset(&serveraddr, '\0', sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;   
    serveraddr.sin_addr.s_addr = INADDR_ANY;  
    serveraddr.sin_port = htons(port);   
     
    //bind 
    if (bind(master_socket, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)  {   
        perror("bind failed");   
        exit(EXIT_FAILURE);   
    }   
    printf("Listener on port %d \n", port);   
         
    //listen
    if (listen(master_socket, 5) < 0)  {   
        perror("listen");   
        exit(EXIT_FAILURE);   
    }   
         

    puts("Waiting for connections ...");   
	    
	//event loop 
    while(1) {  
 
        //clear the socket set  
        FD_ZERO(&readfds);   
     
        //add master socket to set  
        FD_SET(master_socket, &readfds);   
        max_sd = master_socket;   
             
        //add client sockets to set  
		if (socket_list) {
			for (SOCKET* s = socket_list; s; s=s->next) {
		        sd = s->fd;       
		        //if valid socket descriptor then add to read list  
		        if(sd > 0) FD_SET( sd , &readfds);   
		        if(sd > max_sd) max_sd = sd;   

			}
		}

        //wait for activity 
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);   
       
        if ((activity < 0) && (errno!=EINTR)) printf("select error");   
          
        //accept incoming connection  
        if (FD_ISSET(master_socket, &readfds)) {   
            if ((new_socket = accept(master_socket, (struct sockaddr *)&newaddr, &addr_size))<0) {   
                perror("accept");   
                exit(EXIT_FAILURE);   
            }   

			printf("Adding socket: %d\n" , new_socket);
			socket_list = add_socket(socket_list, new_socket);
			memset(msg, '\0', sizeof(msg));
			sprintf(msg, "%s%s", "M: Guess the missing ____!\n", "M: Send your guess in the form 'R: word\\r\\n'.\n");	

			pf = popen(command,"r"); 
			get_epigram(pf, epigram, command);
			hide_word(epigram, correct);
			hide_word(epigram, correct);
			strcat(msg, epigram);
			write(new_socket, msg, strlen(msg)); 
		

                 
		}   
           
		for (SOCKET* s = socket_list; s; s=s->next) {
            sd = s->fd;   

            if (FD_ISSET(sd , &readfds)) {   
             
				memset(guess, '\0', sizeof(guess)); 
                if ((valread = read( sd , guess, 1024)) <= 0)   {   
                    getpeername(sd , (struct sockaddr*)&newaddr , &addr_size);   
                    printf("Host disconnected , ip %s , port %d \n", inet_ntoa(newaddr.sin_addr), ntohs(newaddr.sin_port));   
                    socket_list = remove_socket(socket_list, s);  
               
                }   

 				else if (strcmp(guess, "Q:") == 0) {
					memset(msg, '\0', sizeof(msg));
					sprintf(msg,  "M: You mastered %d/%d challenges. Good bye!\n", s->wins, s->play);
					write(sd, msg, strlen(msg));
                    socket_list = remove_socket(socket_list, s);  
               
				
				}
				
                else if (guess[0] == 'R') { 	

					memset(msg, '\0', sizeof(msg));
					
					if (s->play == 0) {
						if(strcmp(correct, guess+3) ==0) {
							sprintf(msg,  "O: Congratulation - challenge passed!\n");
							s->wins++;
						} 
						else {
							sprintf(msg,  "F: Wrong guess - expected: %s\n", correct);

						}
					}
					else {
						if(strcmp(correct, guess+3) ==0){
							sprintf(msg,  "O: Congratulation - challenge passed!\n");
							s->wins++;
						}
						else {
							sprintf(msg,  "F: Wrong guess - expected: %s\n", correct);

						} 

					}
					//call backs
					pf = popen(command,"r"); 
					get_epigram(pf, epigram, command);
					hide_word(epigram, correct);
					strcat(msg, epigram);
					write(sd, msg, strlen(msg));

					s->play++;
			
                }

				else {
					memset(msg, '\0', sizeof(msg));
					sprintf(msg, "M: Send your guess in the form 'R: word\\r\\n'. or 'Q:' to quit\n");
					write(sd, msg, strlen(msg));
				}

                 
            }   
		}
    }   
         
    return 0;   
}   
