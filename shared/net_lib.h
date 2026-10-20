// net_lib.h - Librería de red para Ape Escape MMORPG
// Wrapper de sockets TCP/UDP para integración con el decomp

#ifndef AE_NET_LIB_H
#define AE_NET_LIB_H

#include <stdint.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET ae_socket_t;
#define AE_INVALID_SOCKET INVALID_SOCKET
#define AE_SOCKET_ERROR SOCKET_ERROR
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
typedef int ae_socket_t;
#define AE_INVALID_SOCKET (-1)
#define AE_SOCKET_ERROR (-1)
#endif

#define AE_NET_OK 0
#define AE_NET_ERROR -1
#define AE_NET_WOULD_BLOCK -2

// Inicializar la librería de red (WSAStartup en Windows)
int ae_net_init(void);

// Limpiar la librería de red
void ae_net_cleanup(void);

// Crear socket TCP
ae_socket_t ae_net_tcp_create(void);

// Crear socket UDP
ae_socket_t ae_net_udp_create(void);

// Conectar TCP a un servidor
int ae_net_tcp_connect(ae_socket_t sock, const char *ip, uint16_t port);

// Bind UDP a un puerto local
int ae_net_udp_bind(ae_socket_t sock, uint16_t port);

// Enviar datos TCP
int ae_net_tcp_send(ae_socket_t sock, const void *data, size_t len);

// Recibir datos TCP (no bloqueante)
int ae_net_tcp_recv(ae_socket_t sock, void *buf, size_t len);

// Enviar datos UDP
int ae_net_udp_send(ae_socket_t sock, const void *data, size_t len,
                    const char *ip, uint16_t port);

// Recibir datos UDP (no bloqueante)
int ae_net_udp_recv(ae_socket_t sock, void *buf, size_t len,
                    char *from_ip, uint16_t *from_port);

// Cerrar socket
void ae_net_close(ae_socket_t sock);

// Poner socket en modo no bloqueante
int ae_net_set_nonblocking(ae_socket_t sock);

#endif // AE_NET_LIB_H
