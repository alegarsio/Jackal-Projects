#ifndef NET_UTILS_H
#define NET_UTILS_H

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    
    typedef SOCKET socket_t;
    #define CLOSE_SOCKET(s) closesocket(s)
    #define INVALID_SOCKET_VAL INVALID_SOCKET
    #define SOCKET_ERROR_VAL SOCKET_ERROR
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netdb.h>
    #include <errno.h>

    typedef int socket_t;
    #define CLOSE_SOCKET(s) close(s)
    #define INVALID_SOCKET_VAL -1
    #define SOCKET_ERROR_VAL -1
#endif

void net_init(void);
void net_cleanup(void);

#endif