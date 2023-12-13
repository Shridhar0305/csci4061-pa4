#include "server.h"

#define PORT 9637
#define MAX_CLIENTS 5
#define BUFFER_SIZE 1024 
#define BACKLOG 5


void *clientHandler(void *socket) {
    processing_args_t *pargs = (processing_args_t *) socket;
    printf("[SERVER]: client handler\n");

    while (1) {
        // Receive packets from the client
        char data[BUFF_SIZE];
        memset(data, 0, BUFF_SIZE);
        recv(pargs->number_worker, data, BUFF_SIZE, 0);
        packet_t *packet = deserializeData(data);

        // Determine the packet operation and flags
        if (packet->operation == IMG_OP_EXIT) {
            free(packet);
            close(socket);
            pthread_exit(NULL);
        }

        // Receive the image data using the size

        // Process the image data based on the set of flags
        if (packet->operation == IMG_OP_ROTATE) {
            printf("%s\n", pargs->file_name);
            int width;
            int height;
            int bpp;

            uint8_t* image_result = stbi_load(socket->filename, &width, &height, &bpp, CHANNEL_NUM);

            uint8_t** img_matrix = (uint8_t **) malloc(sizeof(uint8_t *) * width);
            uint8_t** result_matrix = (uint8_t **) malloc(sizeof(uint8_t *) * width);
            for (int i = 0; i < width; i++) {
                img_matrix[i] = (uint8_t *) malloc(sizeof(uint8_t) * height);
                result_matrix[i] = (uint8_t *) malloc(sizeof(uint8_t) * height);
            }
    
            linear_to_image(image_result, img_matrix, width, height);

            if (packet->flags == IMG_FLAG_ROTATE_180) { // need to get angle from struct
                flip_left_to_right(img_matrix, result_matrix, width, height);
            } else if (packet->flags == IMG_FLAG_ROTATE_270) {
                flip_upside_down(img_matrix, result_matrix, width, height);
            }
    
            uint8_t* img_array = (uint8_t *) malloc(sizeof(uint8_t) * width * height);
            flatten_mat(result_matrix, img_array, width, height);

            if (stbi_write_png(out, width, height, CHANNEL_NUM, img_array, width*CHANNEL_NUM) == 0) {
                exit(-1);
            }

            memset(buf, 0, 1024);
            memset(out, 0, 1024);

            for (int i = 0; i < width; i++) {
                free(img_matrix[i]);
                free(result_matrix[i]);
            }

            free(image_result);
            free(img_matrix);
            free(result_matrix);
            free(img_array);
        }

        // Acknowledge the request and return the processed image data

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
    conn_fd = accept(listen_fd, (struct sockaddr *) &clientaddr, &clientaddr_len);
    if(conn_fd == -1){
        perror("accept error");
    }

    /*pthread_t proc_thread;
    pthread_create(&proc_thread, NULL, (void *)clientHandler, (void *)&conn_fd);
    printf("[SERVER]: thread created for client\n");

    pthread_join(proc_thread, NULL);*/
    
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
