#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define PORT 7503
#define BUFFER_SIZE 100

int main(int argc, char *argv[]){
    int sock_server; /*Socket*/
    int connect_value; long receive_value; /*Auxiliary variables*/
    char text_to_send[BUFFER_SIZE]; char text_to_receive[BUFFER_SIZE]; /*Buffers*/
    int i; /*Others*/

    /*Define socket server name*/
    struct sockaddr_in server;
    server.sin_family=AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr= inet_addr("127.0.0.1");

    /*Create the server socket*/
    sock_server = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_server < 0){ /*Check errno*/
        fprintf(stderr,"[SERVER-error]: Socket creating fail. %d: %s \n",errno,strerror(errno));
        exit(1);
    }
    else printf("Socket created successfully.\n");

    /*Bind the socket with the peer structure and connect*/
    connect_value = connect(sock_server, (struct sockaddr *)&server, sizeof(server));
    if(connect_value < 0){ /*Check errno*/
        fprintf(stderr,"[SERVER-error]: Connect failed. %d: %s \n",errno,strerror(errno));
        exit(1);
    }
    else printf("Successful connection\n");

    /*Unlock the server_socket and the input file descriptor*/
    if ( fcntl(sock_server, F_SETFL, O_NONBLOCK) < 0 ){
        fprintf(stderr,"[SERVER-error]: Nonblocking socket fail. %d: %s \n",errno,strerror(errno));
        exit(1);
    }
    if ( fcntl(0, F_SETFL, O_NONBLOCK) < 0 ){
        fprintf(stderr,"[SERVER-error]: Nonblocking input fail. %d: %s \n",errno,strerror(errno));
        exit(1);
    }

    /*--------------------------------------------------------------------------------------*/
    /*Send a name*/
    for(i=0;i<100;i++) text_to_send[i] = '\0';
    strcpy(text_to_send,argv[1]);
    strcat(text_to_send,"\n");
    if( send(sock_server,text_to_send, 100,MSG_DONTWAIT) < 0 ){
        fprintf(stderr,"[CLIENT-error]: Cannot be reading. %d: %s \n",errno,strerror(errno));
        exit(1);
    }

    /*Do to conversation.*/
    while(1){
        /*Check if the client want to exit or not*/
        if(strncmp("Exit",text_to_send,4)==0){
            break;
        }
        else{
            /*Receive messages*/
            for(i=0;i<100;i++) text_to_receive[i] = '\0';
            if(recv(sock_server, text_to_receive, 100, 0)>0){
                printf("%s",text_to_receive);
            }

            /*Send messages*/
            for(i=0;i<100;i++) text_to_send[i] = '\0';
            if(read(0,text_to_send, 100)>0){
                if(send(sock_server,text_to_send, strlen(text_to_send),0)>=0){
                    printf("[ME]: %s",text_to_send);
                }
            }
            //sleep(1);
        }

    }

    /*Close the communication*/
    close(sock_server);

    /*Exit*/
    exit(0);
}
