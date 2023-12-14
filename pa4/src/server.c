#include "server.h"
#include "common.h"

#define PORT 9637
#define MAX_CLIENTS 5
#define BUFFER_SIZE 1024 
#define BACKLOG 5


void *clientHandler(void *socket) {
    int socket_fd = *(int *) socket;
    while (1) {
        // Receive packets from the client
        char data[PACKETSZ];
        memset(data, 0, PACKETSZ);
        int ret = recv(socket_fd, data, PACKETSZ, 0);
        if (ret == -1) {
            perror("recv error");
            exit(-1);
        }
        if (ret == 0) {
            continue;
        }
        packet_t *packet = deserializeData(data);

        // Determine the packet operation and flags
        if (packet->operation == IMG_OP_EXIT) {
            free(packet);
            close(socket_fd);
            pthread_exit(NULL);
        }

        // Receive the image data using the size
        if (packet->operation == IMG_OP_ROTATE) {
            char *image = (char *) malloc(packet->size);
            memset(image, 0, packet->size);
            ret = recv(socket_fd, image, packet->size, 0);
            if (ret == -1) {
                perror("recv error");
                exit(-1);
            }
            const char *input = "input.png";
            FILE *file = fopen(input, "w");
            if (file == NULL) {
                perror("Error opening file");
                exit(-1);
            }
            fwrite(image, 1, packet->size, file);
            fclose(file);

            int width;
            int height;
            int bpp;

            uint8_t* image_result = stbi_load(input, &width, &height, &bpp, CHANNEL_NUM);

            uint8_t** img_matrix = (uint8_t **) malloc(sizeof(uint8_t *) * width);
            uint8_t** result_matrix = (uint8_t **) malloc(sizeof(uint8_t *) * width);
            for (int i = 0; i < width; i++) {
                img_matrix[i] = (uint8_t *) malloc(sizeof(uint8_t) * height);
                result_matrix[i] = (uint8_t *) malloc(sizeof(uint8_t) * height);
            }
    
            linear_to_image(image_result, img_matrix, width, height);

            // Process the image data based on the set of flags
            if (packet->flags == IMG_FLAG_ROTATE_180) { // need to get angle from struct
                flip_left_to_right(img_matrix, result_matrix, width, height);
            } else if (packet->flags == IMG_FLAG_ROTATE_270) {
                flip_upside_down(img_matrix, result_matrix, width, height);
            }
    
            uint8_t* img_array = (uint8_t *) malloc(sizeof(uint8_t) * width * height);
            flatten_mat(result_matrix, img_array, width, height);

            const char *output = "output.png";
            stbi_write_png(output, width, height, CHANNEL_NUM, img_array, width*CHANNEL_NUM);

            // Acknowledge the request and return the processed image data
            packet_t ack;
            ack.operation = IMG_OP_ACK;
            ack.flags = packet->flags;
            ack.size = packet->size;
            
            for (int i = 0; i < width; i++) {
                free(img_matrix[i]);
                free(result_matrix[i]);
            }

            free(image_result);
            free(img_matrix);
            free(result_matrix);
            free(img_array);
        }
        free(packet);
    }
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
    while (1) {
        conn_fd = accept(listen_fd, (struct sockaddr *) &clientaddr, &clientaddr_len);
        if(conn_fd == -1){
            perror("accept error");
        }
        pthread_t proc_thread;
        pthread_create(&proc_thread, NULL, (void *)clientHandler, (void *)&conn_fd);
        pthread_detach(proc_thread);
    }
    // For intermediate, just accept one package then exit
    /*packet_t package;
    ret = recv(conn_fd, &package, sizeof(packet_t), 0);
    if(ret == -1){
        perror("recv error");
    }else{
        printf("package received!\n");
    }*/

    // Release any resources
    close(conn_fd); 
    close(listen_fd);
    return 0;
}
