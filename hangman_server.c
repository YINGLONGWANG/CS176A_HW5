/**
 * The basic structure was copied from http://www.linuxhowtos.org/data/6/server.c
 * Lines copied: Line 5 - Line 46
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

int num_connection = 0;
void error(const char *msg)
{
        perror(msg);
        exit(1);
}
char* get_word(){
        FILE *fptr = fopen("hangman_words.txt", "r");
        if(fptr == NULL){
                error("Cannot open file\n");
        }
        char words[15][9];
        int length = 0;
        while(fscanf(fptr, "%s", words[length]) > 0){
                length ++;
        }
        fclose(fptr);
        srand(time(0));
        int random_index = rand() % length;
        char* return_word = malloc(sizeof(char) * 9);
        for(int i = 0; i < 9; i++){
                return_word[i] ='\0';
        }
        strcpy(return_word, words[random_index]);
        return return_word;
}

char* init_packet(char* word_choice){
        char* word = get_word();
        strcpy(word_choice, word);
        char* packet = malloc(sizeof(char) * 18);
        for(int i = 0; i < 18; i++){
                packet[i] = '\0';
        }
        // msg flag
        packet[0] = '0';
        // word length
        packet[1] = '0' + strlen(word);
        // num incorrect
        packet[2] = '0';
        for(int i = 0; i < strlen(word); i++){
                packet[3 + i] = '_';
        }
        return packet;
}
void guess(char* buffer, char c, char* word){
    //printf("%ld\n",strlen(buffer));
    bool correct = false;
    // int i = 3;
    int size = strlen(word);
    for(int i = 0; i < size; i++){
        if (word[i] == c){
            buffer[i+3] = c;
            correct = true;
        }
    }
    if(!correct){
        for(int i = 3+size; i < 17; i++){
            if(buffer[i] == '\0'){
		buffer[2]++;
                buffer[i] = c;
		//printf("123\n");
                break;
            }
            if(buffer[i] == c) break;
        }
    }
    //printf("buffer: %s\n", buffer);
}

void* handle_connection(int* p_client_socket){
  int client_socket = *p_client_socket;
  free(p_client_socket);
  int n;
  //int counter = 0;
  char buffer[128];
  strcpy(buffer, "2");
  write(client_socket,buffer,strlen(buffer));
  // generate a word and initialize packet
    char word[9];
    char* packet = init_packet(word);
    write(client_socket,packet,strlen(packet)); 
  while(1){    
    bzero(buffer,128);
    n = read(client_socket,buffer,128);
    if(n < 0){
      error("Error at handle_connection\n");
      exit(1);
    }
    guess(packet, buffer[1], word);
    if(packet[2] == '6'){
	// number of incorrect guesses reaches 6
	sprintf(buffer, "%cThe word was %s\nYou Lose.\nGame Over!\n",37 + strlen(word), word);
    	n = write(client_socket,buffer,strlen(buffer));
    	if (n < 0) error("ERROR writing to socket\n");
	break;
    }else{
	int finished = 1;
	for(int i = 3; i < 17; i++){
            if(packet[i] == '_'){
		// correct guess but not finished
		//printf("%s\n", packet);
                n = write(client_socket, packet,strlen(packet));
		if (n < 0) error("ERROR writing to socket\n");
		finished = 0;
		break;
	    }
	}
	if(finished == 1){
		sprintf(buffer, "%cThe word was %s\nYou Win!\nGame Over!\n",36 + strlen(word), word);
		n = write(client_socket, buffer,strlen(buffer));
		if (n < 0) error("ERROR writing to socket\n");
		break;
	}
    //counter++;
    //if (counter > 5) break;
        }
}
  num_connection--;
  //printf("%dcheckpoint1`\n", num_connection);
  close(client_socket);
  return NULL;
}

void* reject_connection(int* p_client_socket){
  //printf("%s\n", "reject_connection");
  int client_socket = *p_client_socket;
  free(p_client_socket);
  int n;
  char buffer[128];
  strcpy(buffer, "server-oveloaded\n");
  n = write(client_socket,buffer,strlen(buffer));
  if (n < 0) error("ERROR writing to socket at reject_connection\n");
  close(client_socket);
  return NULL;
}

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno;
	socklen_t clilen;
	// char buffer[128];
	struct sockaddr_in serv_addr, cli_addr;
	//int n;
	if (argc < 2) {
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
				sizeof(serv_addr)) < 0)
		error("ERROR on binding");
	listen(sockfd,5);
	clilen = sizeof(cli_addr);

  //int counter = 0;
	while(1){
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0)  error("ERROR on accept small socket");

                pthread_t t;
                int* p_client = malloc(sizeof(int));
                *p_client = newsockfd;
                if (num_connection < 3){
                        num_connection++;
                        pthread_create(&t,NULL,handle_connection,p_client);
                }
                else{
                        pthread_create(&t,NULL,reject_connection,p_client);
                }
        } //while

  // unreachable
	close(sockfd);
	return 0;
}

/*
  //int counter = 0;
	while(1){
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0)  error("ERROR on accept small socket");

    pthread_t t;
    int* p_client = malloc(sizeof(int));
    *p_client = newsockfd;
    if (num_connection < 3){
      num_connection++;
      pthread_create(&t,NULL,handle_connection,p_client);
    }
    else{
      pthread_create(&t,NULL,reject_connection,p_client);
    }
  } //while

  // unreachable
	close(sockfd);
	return 0;
        */

