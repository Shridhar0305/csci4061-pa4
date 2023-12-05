#include "server.h"

#define PORT 9637
#define MAX_CLIENTS 5
#define BUFFER_SIZE 1024 
#define BACKLOG 5


void *clientHandler(void *socket) {

    // Receive packets from the client

    // Determine the packet operatation and flags

    // Receive the image data using the size

    // Process the image data based on the set of flags

    // Acknowledge the request and return the processed image data
}

int main(int argc, char* argv[]) {

    // Creating socket file descriptor
    int listen_fd, conn_fd;
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_fd == -1){
        perror("socket error");
    }
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
    
    // Bind the socket to the port
    int ret = bind(listen_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    if(ret == -1){
        perror("bind error");
    }
    // Listen on the socket
    ret = listen(listen_fd, BACKLOG);
    if(ret == -1){
        perror("listen error");
    }
    // Accept connections and create the client handling threads
    struct sockaddr_in clientaddr;
    socklen_t clientaddr_len = sizeof(clientaddr);
    conn_fd = accept(listen_fd, (struct sockaddr *) &clientaddr, &clientaddr_len);
    if(conn_fd == -1){
        perror("accept error");
    }
    // For intermediate, just accept one package then exit
    packet_t package;
    ret = recv(conn_fd, &package, sizeof(packet_t), 0);
    if(ret == -1){
        perror("recv error");
    }else{
        printf("package received!\n");
    }

    // Release any resources
    close(conn_fd);
    close(listen_fd);
    return 0;
}
