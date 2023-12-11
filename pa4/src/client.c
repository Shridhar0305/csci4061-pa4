#include "client.h"

#define PORT 9637
#define BUFFER_SIZE 1024 

int send_file(int socket, const char *filename) {
    // Open the file
    FILE *file = fopen(filename, "r+");
    if(file == NULL){
        perror("Error opening file\n");
        exit(-1);
    }

    // Set up the request packet for the server and send it
    packet_t response;
    int ret = send(socket, &response, sizeof(packet_t), 0);
    if(ret == -1){
        perror("send error");
        fclose(file);
        exit(-1);
    }
    // Send the file data
    char buffer[BUFFER_SIZE];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        int ret = send(socket, buffer, bytesRead, 0);
        if (ret == -1) {
            perror("send file data error");
            fclose(file);
            exit(-1);
        }
    }
    fclose(file);
    return 0;
}

int receive_file(int socket, const char *filename) {
    // Open the file
    FILE *file = fopen(filename, "w+");
    if(file == NULL){
        perror("Error opening file\n");
        exit(-1);
    }
    // Receive response packet
    packet_t response;
    int ret = recv(socket, &response, sizeof(packet_t), 0);
    if(ret == -1){
        perror("recv error");
        fclose(file);
        exit(-1);
    }

    // Receive the file data
    // Write the data to the file
    char buffer[BUFFER_SIZE];
    size_t bytesRead;
    while ((bytesRead = recv(socket, buffer, sizeof(buffer), 0)) > 0) {
        int ret = fwrite(buffer, 1, bytesRead, file);
        if (ret != bytesRead) {
            perror("write to file error");
            fclose(file);
            exit(-1);
        }
    }

    fclose(file);
    return 0;
}


int main(int argc, char* argv[]) {
    if(argc != 4){
        fprintf(stderr, "Usage: ./client File_Path_to_images File_Path_to_output_dir Rotation_angle. \n");
        return 1;
    }
    char* image_dir = argv[1];
    char* output_dir = argv[2];
    int angle = atoi(argv[3]);
    
    // Set up socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1){
        perror("socket error");
    }
    // Connect the socket
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);
    int ret = connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    if(ret == -1){
        perror("connect error");
    }

    // Read the directory for all the images to rotate
    request_t queue[MAX_QUEUE_LEN];
    memset(queue, 0, sizeof(request_t) * MAX_QUEUE_LEN);
    int queue_length = 0;
    DIR *d;
    struct dirent *dir;
    d = opendir(image_dir);
    if(d == NULL){
        perror("Unable to open directory");
    }
    if(d){
        while((dir = readdir(d)) != NULL){
            if(strlen(dir->d_name) > 4 && !strcmp(dir->d_name + strlen(dir->d_name) - 4, ".png")){
                strcpy(queue[queue_length].file_name, dir->d_name);
                queue[queue_length].rotation_angle = angle;
                queue_length++;
            }
        }
        closedir(d);
    }
    // Send package here for intermediate
    packet_t inter_package;
    inter_package.operation = IMG_OP_ROTATE;

    ret = send(sockfd, &inter_package, sizeof(packet_t), 0);
    if(ret == -1){
        perror("send error");
    }
    // Send the image data to the server
    for(int i = 0; i < queue_length; i++){
        ret = send_file(sockfd, queue[i].file_name);
        if (ret == -1) {
            fprintf(stderr, "Error sending file: %s\n", queue[i].file_name);
            exit(-1);
        }
    
        // Check that the request was acknowledged
        packet_t ack_packet;
        ret = recv(sockfd, &ack_packet, sizeof(packet_t), 0);
        if (ret == -1) {
            perror("receive acknowledgment error");
            close(sockfd);
            exit(-1);
        }
        if (ack_packet.operation != IMG_OP_ACK) {
            fprintf(stderr, "Unexpected acknowledgment from server\n");
            close(sockfd);
            exit(-1);
        }
        // Receive the processed image and save it in the output dir
        ret = receive_file(sockfd, output_dir);
        if (ret == -1) {
            fprintf(stderr, "Error receiving file: %s\n", queue[i].file_name);
            exit(-1);
        }
        // Terminate the connection once all images have been processed
        if (i == queue_length - 1) {
            packet_t termination_packet;
            termination_packet.operation = IMG_OP_EXIT;
            ret = send(sockfd, &termination_packet, sizeof(packet_t), 0);
            if (ret == -1) {
                perror("send termination error");
                close(sockfd);
                exit(-1);
            }
        }
    }
    // Release any resources
    close(sockfd);
    return 0;
}
