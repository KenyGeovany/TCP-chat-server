#include <stdio.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h> /*strcpy, strcat, strncmp*/
#include <errno.h>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
#define DATA_BUFFER 5000
#define MAX_CONNECTIONS 10

void create_menu(char * buf_menu, char clients_name[MAX_CONNECTIONS][100], int * clients_status){
    /*This function creates a menu in order to the clients choose their peers.*/
    strcpy(buf_menu,"------------------\nMENU:\n");
    for(int i=4;i<MAX_CONNECTIONS;i++){
        if(clients_status[i]==0){
            strcat(buf_menu, "* ");
            strcat(buf_menu,clients_name[i]);
        }
    }
    strcat(buf_menu,"------------------\n");
    strcat(buf_menu,"Write the name of the person you want to talk.\n");
}

int create_tcp_server_socket() {
    struct sockaddr_in saddr;
    int fd, ret_val;

    /* Step1: create a TCP socket */
    fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd == -1) {
        fprintf(stderr, "socket failed [%s]\n", strerror(errno));
        return -1;
    }
    printf("Created a socket with fd: %d\n", fd);

    /* Initialize the socket address structure */
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(7503);
    saddr.sin_addr.s_addr = INADDR_ANY;

    /* Step2: bind the socket to port 7000 on the local host */
    ret_val = bind(fd, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in));
    if (ret_val != 0) {
        fprintf(stderr, "bind failed [%s]\n", strerror(errno));
        close(fd);
        return -1;
    }


    /* Step3: listen for incoming connections */
    ret_val = listen(fd, 5);
    if (ret_val != 0) {
        fprintf(stderr, "listen failed [%s]\n", strerror(errno));
        close(fd);
        return -1;
    }

    return fd;
}

int main () {
    fd_set read_fd_set;
    struct sockaddr_in new_addr;
    int server_fd, new_fd, ret_val, i, j, k;
    socklen_t addrlen;
    int all_connections[MAX_CONNECTIONS];

    /* Get the socket server fd */
    server_fd = create_tcp_server_socket();
    if (server_fd == -1) {
        fprintf(stderr, "Failed to create a server\n");
        return -1;
    }

    /* Initialize all_connections and set the first entry to server fd */
    for (i=0;i < MAX_CONNECTIONS;i++) {
        all_connections[i] = -1;
    }
    all_connections[0] = server_fd;

    /*--------------------------------------------------------------------------------------*/
    /*Buffers*/
    char text_to_send[DATA_BUFFER]; char text_to_receive[DATA_BUFFER]; char buf_menu[300];

    /*Arrays of communication control*/
    int clients_linked[MAX_CONNECTIONS];
    for(i=0;i< MAX_CONNECTIONS;i++){clients_linked[i]=-1;}
    clients_linked[3] = 3;
    int clients_status[MAX_CONNECTIONS];
    for(i=0;i< MAX_CONNECTIONS;i++){clients_status[i]=-1;}
    clients_status[3] = 3;
    char clients_name[MAX_CONNECTIONS][100];
    for(i=0;i< MAX_CONNECTIONS;i++){ strcpy(clients_name[i],"\0");}
    strcpy(clients_name[3],"Server");
    /*--------------------------------------------------------------------------------------*/
    while (1) {
        FD_ZERO(&read_fd_set);
        /* Set the fd_set before passing it to the select call */
        for (i=0;i < MAX_CONNECTIONS;i++) {
            if (all_connections[i] >= 0) {
                FD_SET(all_connections[i], &read_fd_set);
            }
        }

        /* Invoke select() and then wait! */
        printf("\nUsing select() to listen for incoming events\n");
        ret_val = select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL);

        /* select() woke up. Identify the fd that has events */
        if (ret_val >= 0 ) {
            printf("Select returned with %d\n", ret_val);
            /* Check if the fd with event is the server fd */
            if (FD_ISSET(server_fd, &read_fd_set)) {
                /* accept the new connection */
                printf("Returned fd is %d (server's fd)\n", server_fd);
                new_fd = accept(server_fd, (struct sockaddr*)&new_addr, &addrlen);
                if (new_fd >= 0) {
                    printf("Accepted a new connection with fd: %d\n", new_fd);
                    for (i=0;i < MAX_CONNECTIONS;i++) {
                        if (all_connections[i] < 0) {
                            all_connections[i] = new_fd; /*Find the fd*/
                            /*----------------------------------------------------*/
                            /*Receive and print the name of the client*/
                            for(j=0;j<100;j++) text_to_receive[j] = '\0';
                            if(recv(new_fd, text_to_receive, 100, 0)>0){
                                printf("[CLIENT NAME]: %s",text_to_receive);
                            } else fprintf(stderr, "recv() name failed [%s]\n", strerror(errno));
                            strcpy(clients_name[new_fd],text_to_receive ); /*Update names array*/
                            clients_status[new_fd] = 0; /*Change the status to waiting*/
                            /*Send a menu to all people connected waiting*/
                            create_menu(buf_menu,clients_name,clients_status);
                            for(j=4;j<MAX_CONNECTIONS;j++){
                                if(clients_status[j] == 0){
                                    if(send(j,buf_menu,strlen(buf_menu),MSG_DONTWAIT)<0){
                                        fprintf(stderr, "send() menu failed [%s]\n", strerror(errno));
                                    }
                                }
                            }
                            /*---------------------------------------------------*/
                            break;
                        }
                    }
                } else fprintf(stderr, "accept failed [%s]\n", strerror(errno));
                ret_val--;
                if (!ret_val) continue;
            }else fprintf(stderr, "select failed [%s]\n", strerror(errno));

            /* Check if the fd with event is a non-server fd */
            for (i=1;i < MAX_CONNECTIONS;i++) {
                if ((all_connections[i] > 0) &&
                    (FD_ISSET(all_connections[i], &read_fd_set))) {
                    /* read incoming data */
                    printf("Returned fd is %d [index, i: %d]\n", all_connections[i], i);
                    for(j=0;j<100;j++) text_to_receive[j] = '\0';
                    ret_val = recv(all_connections[i], text_to_receive, DATA_BUFFER, 0);
                    if (ret_val == 0) {
                        /*---------------------------------------------------*/
                        /*End the conversation*/
                        printf("Closing (by disconnection of client) connection for fd:%d\n", all_connections[i]);
                        if(send(clients_linked[all_connections[i]],"(The conversation is over.)\n", 32,MSG_DONTWAIT)<0){
                            fprintf(stderr, "send() end notification failed [%s]\n", strerror(errno));
                        }
                        /*Update all arrays of control*/
                        strcpy(clients_name[all_connections[i]],"\0");
                        clients_status[clients_linked[all_connections[i]]] = 0;
                        clients_status[all_connections[i]] = -1;
                        clients_linked[clients_linked[all_connections[i]]] = -1;
                        clients_linked[all_connections[i]] = -1;
                        strcpy(clients_name[all_connections[i]],"\0");
                        /*Close the client socket*/
                        close(all_connections[i]);
                        all_connections[i] = -1; /* Connection is now closed */
                        /*Update the menu for every client waiting*/
                        create_menu(buf_menu,clients_name,clients_status);
                        for(j=4;j<MAX_CONNECTIONS;j++){
                            if(clients_status[j] == 0){
                                if(send(j,buf_menu,strlen(buf_menu),MSG_DONTWAIT)<0){
                                    fprintf(stderr, "send() menu failed [%s]\n", strerror(errno));
                                }
                            }
                        }
                        /*---------------------------------------------------*/
                    }
                    if (ret_val > 0) {
                        printf("Received data (len %d bytes, fd: %d): %s\n", ret_val, all_connections[i], text_to_receive);
                        /*---------------------------------------------------*/
                        if((strncmp("Exit",text_to_receive,4)==0)&&(clients_status[all_connections[i]]==0)){
                            /*End the conversation*/
                            printf("Closing (by exit) connection for fd:%d \n", all_connections[i]);
                            /*Update all arrays of control*/
                            strcpy(clients_name[all_connections[i]],"\0");
                            clients_status[all_connections[i]] = -1;
                            /*Close the client socket*/
                            close(all_connections[i]);
                            all_connections[i] = -1; /* Connection is now closed */
                            /*Update the menu for every client waiting*/
                            create_menu(buf_menu,clients_name,clients_status);
                            for(j=4;j<MAX_CONNECTIONS;j++){
                                if(clients_status[j] == 0){
                                    if(send(j,buf_menu,strlen(buf_menu),MSG_DONTWAIT)<0){
                                        fprintf(stderr, "send() menu failed [%s]\n", strerror(errno));
                                    }
                                }
                            }
                        }
                        else if((strncmp("Bye",text_to_receive,3)==0)&&(clients_status[all_connections[i]]==1)){
                            /*End the conversation*/
                            printf("Closing (by good bye) connection for fd:%d\n", all_connections[i]);
                            if(send(clients_linked[all_connections[i]],"Bye. (The conversation is over.)\n", 39,MSG_DONTWAIT)<0){
                                fprintf(stderr, "send() Bye failed [%s]\n", strerror(errno));
                            }
                            /*Update all arrays of control*/
                            clients_status[clients_linked[all_connections[i]]] = 0;
                            clients_status[all_connections[i]] = 0;
                            clients_linked[clients_linked[all_connections[i]]] = -1;
                            clients_linked[all_connections[i]] = -1;
                            /*Update the menu for every client waiting*/
                            create_menu(buf_menu,clients_name,clients_status); /*Actualize the menu*/
                            for(j=4;j<MAX_CONNECTIONS;j++){
                                if(clients_status[j] == 0){
                                    if(send(j,buf_menu,strlen(buf_menu),MSG_DONTWAIT)<0){
                                        fprintf(stderr, "send() menu failed [%s]\n", strerror(errno));
                                    }
                                }
                            }
                        }
                        else{
                            /*Check if the client send message to his peer.*/
                            if(clients_status[all_connections[i]]==1){
                                /*Send message to his peer*/
                                for(k=0;k<100;k++) text_to_send[k] = '\0';
                                strcpy(text_to_send,"[");
                                strcat(text_to_send,clients_name[all_connections[i]]);
                                    for(k=0;k<100;k++){ /*Clear the jump of line*/
                                        if(text_to_send[k]=='\n') text_to_send[k] = '\0';
                                    }
                                strcat(text_to_send,"]: ");
                                strcat(text_to_send,text_to_receive);
                                if(send(clients_linked[all_connections[i]],text_to_send,strlen(text_to_send),MSG_DONTWAIT)<0){
                                    fprintf(stderr, "send() peer name failed [%s]\n", strerror(errno));
                                }
                            }
                            /*Check if the client send message to the server.*/
                            else if(clients_status[all_connections[i]]==0){
                                /*Find the fd of the name associated to him.*/
                                for(j=4;j<MAX_CONNECTIONS;j++){
                                    if(strncmp(text_to_receive,clients_name[j],30)==0){
                                        /*Update all arrays of control*/
                                        clients_linked[all_connections[i]] = j;
                                        clients_linked[j] = all_connections[i];
                                        clients_status[all_connections[i]] = 1;
                                        clients_status[clients_linked[all_connections[i]]] = 1;
                                        /*Report the established connection to the peer.*/
                                        for(k=0;k<100;k++) text_to_send[k] = '\0';
                                        strcpy(text_to_send,"[SERVER]: ");
                                        strcat(text_to_send,clients_name[all_connections[i]]);
                                            for(k=0;k<100;k++){ /*Clear the jump of line*/
                                                if(text_to_send[k]=='\n') text_to_send[k] = '\0';
                                            }
                                        strcat(text_to_send," is connected with you.\n");
                                        if(send(j,text_to_send, strlen(text_to_send),MSG_DONTWAIT)<0){
                                            fprintf(stderr, "send() notification failed [%s]\n", strerror(errno));
                                        }
                                        /*Update the menu for every client waiting*/
                                        create_menu(buf_menu,clients_name,clients_status); /*Actualize the menu*/
                                        for(j=4;j<MAX_CONNECTIONS;j++){
                                            if(clients_status[j] == 0){
                                                if(send(j,buf_menu,strlen(buf_menu),MSG_DONTWAIT)<0){
                                                    fprintf(stderr, "send() menu failed [%s]\n", strerror(errno));
                                                }
                                            }
                                        }
                                        /*If a name is found, then return the fd corresponding to this name. */
                                        j = MAX_CONNECTIONS;
                                    }
                                }
                                /* If it has been written a name not in the list, then ignore it and
                                 * send again the menu. */
                                if(j == MAX_CONNECTIONS){
                                    if(send(all_connections[i],"Name not found.\n", 16,MSG_DONTWAIT)<0){
                                        fprintf(stderr, "send() notification failed [%s]\n", strerror(errno));
                                    }
                                    create_menu(buf_menu,clients_name,clients_status); /*Actualize the menu*/
                                    if(send(all_connections[i],buf_menu,strlen(buf_menu),MSG_DONTWAIT)<0){
                                        fprintf(stderr, "send() menu failed [%s]\n", strerror(errno));
                                    }
                                }
                            }
                        }
                        /*---------------------------------------------------*/
                    }
                    if (ret_val == -1) {
                        printf("recv() failed for fd: %d [%s]\n", all_connections[i], strerror(errno));
                        break;
                    }
                }
                ret_val--;
                if (!ret_val) continue;
            } /* for-loop */
        } /* (ret_val >= 0) */
    } /* while(1) */

    /* Last step: Close all the sockets */
    for (i=0;i < MAX_CONNECTIONS;i++) {
        if (all_connections[i] > 0) {
            close(all_connections[i]);
        }
    }
    return 0;
}
#pragma clang diagnostic pop