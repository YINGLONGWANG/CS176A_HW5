/**
 *
 * The basic structure was copied from http://www.linuxhowtos.org/data/6/client.c
 * Lines copied: line 7 - line 48
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <ctype.h>
void error(const char *msg)
{
	perror(msg);
	exit(0);
}

int main(int argc, char *argv[])
{
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer[128];
	if (argc < 3) {
		fprintf(stderr,"usage %s hostname port\n", argv[0]);
		exit(0);
	}
	portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");
	server = gethostbyname(argv[1]);
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, 
			(char *)&serv_addr.sin_addr.s_addr,
			server->h_length);
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
		error("ERROR connecting");
	bzero(buffer, 128);
	n = read(sockfd, buffer, 128);
	if(isalpha(buffer[0])){
		printf("%s\n", buffer);
		close(sockfd);
		return 0;
	}
	printf("Ready to start game?(y/n): ");
	bzero(buffer,128);
	fgets(buffer, 128, stdin);
	while(strlen(buffer) != 2 ||( buffer[0] != 'Y' && buffer[0] != 'y' && buffer[0] != 'N' && buffer[0] != 'n')){
		printf("Ready to start game?(y/n): ");
		bzero(buffer,128);
		fgets(buffer, 128, stdin);
	}
	if(buffer[0] == 'Y' || buffer[0] == 'y'){
		// send empty message to indicate game starts
		// TODO
		char message[128];
		bzero(message, 128);
		n = write(sockfd, message, strlen(message));		
		if(n < 0){
			error("Error in writing\n");
		}
		n = read(sockfd, buffer, 128);
		printf("\n");
		printf("%.*s\n", strlen(buffer) - 3, buffer + 3);
		printf("Incorrect Guesses:\n\n");
		char guess[10];
		while(1){
			// prompts user to guess
			bzero(guess, 10);
			printf("Letter to guess: ");
			fgets(guess, 10, stdin);
			// error checking until it's right
			while(strlen(guess) != 2 || !isalpha(guess[0])){
				bzero(guess, 10);
				printf("Error! Please guess one letter.\n");
				printf("Letter to guess: ");
				fgets(guess, 10, stdin);
			}
			// format guess
			char guess_letter = tolower(guess[0]);
			guess[0] = '1';
			guess[1] = guess_letter;
			guess[2] = '\0';
			// send the guess
			n = write(sockfd, guess, strlen(guess));
			if(n < 0){
				error("Error in writing\n");
			}
			bzero(buffer, 128);	
			if(read(sockfd, buffer, 128) > 0){
				// reading response from server
				int msg_flag = buffer[0] - '0';
				if(msg_flag != 0){
					// game over
					int length = buffer[0];
					printf("%.*s", length , buffer + 1);
					close(sockfd);
					return 0;
				}else{
					int word_length = buffer[1] - '0';
                                	int num_incorrect = buffer[2] - '0';
					printf("%.*s\n", word_length, buffer + 3);
					printf("Incorrect Guesses: %.*s\n\n", num_incorrect, buffer + 3 + word_length);
		
				}	

			}else{
				error("ERROR reading\n");
			}

		}
	}else{
		// close the connection and terminate the program
		close(sockfd);
		return 0;
	}

	// n = write(sockfd,buffer,strlen(buffer));
	//if (n < 0) 
	//	error("ERROR writing to socket");
	//bzero(buffer,128);

	//while(read(sockfd, buffer, 128) > 0){
	//}
	//close(sockfd);
	return 0;
}
