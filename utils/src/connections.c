#include "connections.h"

#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int iniciar_servidor(char* ip, char* port) {
    int optVal = 1;
    struct addrinfo hints;
    struct addrinfo* serverInfo;

    memset(&hints, 0, sizeof(hints));  // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;       // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;   // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;       // fill in my IP for me

    int rv = getaddrinfo(ip, port, &hints, &serverInfo);
    if (rv != 0) {
        printf("getaddrinfo error: %s\n", gai_strerror(rv));
        return EXIT_FAILURE;
    }

    int serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
    if (-1 == serverSocket) {
        printf("Socket creation error\n%s", strerror(errno));
        return EXIT_FAILURE;
    }

    int sockOpt = setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal));
    if (-1 == sockOpt) {
        printf("Sockopt error\n%s", strerror(errno));
        return EXIT_FAILURE;
    }

    int bindVal = bind(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);
    if (-1 == bindVal) {
        printf("Mismatched bind\n%s", strerror(errno));
        return EXIT_FAILURE;
    }

    int listenVal = listen(serverSocket, SOMAXCONN);
    if (-1 == listenVal) {
        printf("Listen error\n%s", strerror(errno));
        return EXIT_FAILURE;
    }

    freeaddrinfo(serverInfo);

    return serverSocket;
}

int conectar_a_servidor(char* ip, char* port) {
    int conn;
    struct addrinfo hints;
    struct addrinfo* serverInfo;
    struct addrinfo* p;

    memset(&hints, 0, sizeof(hints));  // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;       // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;   // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;       // fill in my IP for me

    int rv = getaddrinfo(ip, port, &hints, &serverInfo);
    if (rv != 0) {
        fprintf(stderr, "getaddrinfo error: %s", gai_strerror(rv));
        return EXIT_FAILURE;
    }
    for (p = serverInfo; p != NULL; p = p->ai_next) {
        conn = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (-1 == conn) {
            continue;
        }
        if (connect(conn, p->ai_addr, p->ai_addrlen) != -1) {
            break;
        }
        close(conn);
    }
    freeaddrinfo(serverInfo);
    if (conn != -1 && p != NULL) {
        return conn;
    }
    return -1;
}
