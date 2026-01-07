#include "socket/net_utils.h"
#include <stdio.h>

void net_init(void) {
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        fprintf(stderr, "Gagal inisialisasi Winsock. Error: %d\n", WSAGetLastError());
    }
#endif
}

void net_cleanup(void) {
#ifdef _WIN32
    WSACleanup();
#endif
}