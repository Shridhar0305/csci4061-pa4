#include "client.h"

#define PORT 9637
#define BUFFER_SIZE 1024 

int send_file(int socket, const char *filename) {
    // Open the file

    // Set up the request packet for the server and send it

    // Send the file data
    return 0;
}

int receive_file(int socket, const char *filename) {
    // Open the file

    // Receive response packet

    // Receive the file data

    // Write the data to the file
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
    d = opendir(output_dir);
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
    
    // Check that the request was acknowledged

    // Receive the processed image and save it in the output dir

    // Terminate the connection once all images have been processed

    // Release any resources
    close(sockfd);
    return 0;
}
