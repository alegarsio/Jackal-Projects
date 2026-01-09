#include "net_utils.h"
#include <string.h>

void net_init(void) {
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
}

void net_cleanup(void) {
#ifdef _WIN32
    WSACleanup();
#endif
}

int net_get_last_error(void) {
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
}

socket_t net_socket_create(int domain, int type, int protocol) {
    return socket(domain, type, protocol);
}

int net_socket_bind(socket_t s, const char* ip, int port) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (ip == NULL || strlen(ip) == 0) {
        addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        inet_pton(AF_INET, ip, &addr.sin_addr);
    }
    return bind(s, (struct sockaddr*)&addr, sizeof(addr));
}

int net_socket_connect(socket_t s, const char* host, int port) {
    struct sockaddr_in addr;
    struct hostent *he;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    
    if ((he = gethostbyname(host)) == NULL) return SOCKET_ERROR_VAL;
    memcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);
    
    return connect(s, (struct sockaddr*)&addr, sizeof(addr));
}