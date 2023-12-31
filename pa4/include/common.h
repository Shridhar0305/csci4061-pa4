#ifndef COMMON_H_
#define COMMON_H_

#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 9637
#define STRSZ 1024

// Protocol operations
#define PROTO_REV 0
#define PROTO_ACK 1

// Packet structure
struct Packet_Structure {
    int operation;
    char data[STRSZ];
};

typedef struct Packet_Structure Packet;

const int PACKETSZ = sizeof(packet_t);

// serialize packet
char *serializePacket(packet_t *packet){
    char *serializedData = (char *)malloc(sizeof(char) * PACKETSZ);
    memset(serializedData, 0, PACKETSZ);
    memcpy(serializedData, packet, PACKETSZ);
    return serializedData;
}

// deserialize data
packet_t *deserializeData(char *serializedData){
    packet_t *packet = (packet_t *)malloc(sizeof(packet_t));
    memset(packet, 0, PACKETSZ);
    memcpy(packet, serializedData, PACKETSZ);
    return packet;
}

#endif
