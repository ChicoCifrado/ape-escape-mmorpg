// net_lib.c - Implementación de librería de red

#include "net_lib.h"
#include <string.h>
#include <errno.h>

int ae_net_init(void) {
#ifdef _WIN32
    WSADATA wsa;
    return WSAStartup(MAKEWORD(2, 2), &wsa) == 0 ? AE_NET_OK : AE_NET_ERROR;
#else
    return AE_NET_OK;
#endif
}

void ae_net_cleanup(void) {
#ifdef _WIN32
    WSACleanup();
#endif
}

ae_socket_t ae_net_tcp_create(void) {
    ae_socket_t sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == AE_INVALID_SOCKET) return AE_INVALID_SOCKET;
    ae_net_set_nonblocking(sock);
    return sock;
}

ae_socket_t ae_net_udp_create(void) {
    ae_socket_t sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == AE_INVALID_SOCKET) return AE_INVALID_SOCKET;
    ae_net_set_nonblocking(sock);
    return sock;
}

int ae_net_tcp_connect(ae_socket_t sock, const char *ip, uint16_t port) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) return AE_NET_ERROR;
    int r = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
    if (r == AE_SOCKET_ERROR) {
#ifdef _WIN32
        if (WSAGetLastError() == WSAEWOULDBLOCK) return AE_NET_WOULD_BLOCK;
#else
        if (errno == EINPROGRESS) return AE_NET_WOULD_BLOCK;
#endif
        return AE_NET_ERROR;
    }
    return AE_NET_OK;
}

int ae_net_udp_bind(ae_socket_t sock, uint16_t port) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    return bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == 0 ? AE_NET_OK : AE_NET_ERROR;
}

int ae_net_tcp_send(ae_socket_t sock, const void *data, size_t len) {
    int sent = send(sock, (const char *)data, (int)len, 0);
    if (sent == AE_SOCKET_ERROR) {
#ifdef _WIN32
        if (WSAGetLastError() == WSAEWOULDBLOCK) return AE_NET_WOULD_BLOCK;
#else
        if (errno == EAGAIN || errno == EWOULDBLOCK) return AE_NET_WOULD_BLOCK;
#endif
        return AE_NET_ERROR;
    }
    return sent;
}

int ae_net_tcp_recv(ae_socket_t sock, void *buf, size_t len) {
    int received = recv(sock, (char *)buf, (int)len, 0);
    if (received == AE_SOCKET_ERROR) {
#ifdef _WIN32
        if (WSAGetLastError() == WSAEWOULDBLOCK) return AE_NET_WOULD_BLOCK;
#else
        if (errno == EAGAIN || errno == EWOULDBLOCK) return AE_NET_WOULD_BLOCK;
#endif
        return AE_NET_ERROR;
    }
    if (received == 0) return AE_NET_ERROR; // connection closed
    return received;
}

int ae_net_udp_send(ae_socket_t sock, const void *data, size_t len,
                    const char *ip, uint16_t port) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) return AE_NET_ERROR;
    int sent = sendto(sock, (const char *)data, (int)len, 0,
                      (struct sockaddr *)&addr, sizeof(addr));
    if (sent == AE_SOCKET_ERROR) return AE_NET_ERROR;
    return sent;
}

int ae_net_udp_recv(ae_socket_t sock, void *buf, size_t len,
                    char *from_ip, uint16_t *from_port) {
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    int received = recvfrom(sock, (char *)buf, (int)len, 0,
                            (struct sockaddr *)&addr, &addr_len);
    if (received == AE_SOCKET_ERROR) {
#ifdef _WIN32
        if (WSAGetLastError() == WSAEWOULDBLOCK) return AE_NET_WOULD_BLOCK;
#else
        if (errno == EAGAIN || errno == EWOULDBLOCK) return AE_NET_WOULD_BLOCK;
#endif
        return AE_NET_ERROR;
    }
    if (from_ip) {
        inet_ntop(AF_INET, &addr.sin_addr, from_ip, 16);
    }
    if (from_port) {
        *from_port = ntohs(addr.sin_port);
    }
    return received;
}

void ae_net_close(ae_socket_t sock) {
#ifdef _WIN32
    closesocket(sock);
#else
    close(sock);
#endif
}

int ae_net_set_nonblocking(ae_socket_t sock) {
#ifdef _WIN32
    u_long mode = 1;
    return ioctlsocket(sock, FIONBIO, &mode) == 0 ? AE_NET_OK : AE_NET_ERROR;
#else
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1) return AE_NET_ERROR;
    return fcntl(sock, F_SETFL, flags | O_NONBLOCK) == 0 ? AE_NET_OK : AE_NET_ERROR;
#endif
}
