// client.c - Cliente de red para Ape Escape MMORPG

#include "client.h"
#include "../protocol/protocol.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

int ae_client_init(AeNetClient *c, const char *server_ip, uint16_t port) {
    memset(c, 0, sizeof(AeNetClient));
    strncpy(c->player_name, "Player", sizeof(c->player_name) - 1);

    // Socket TCP
    c->tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (c->tcp_socket < 0) {
        perror("tcp socket");
        return -1;
    }

    // Socket UDP
    c->udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (c->udp_socket < 0) {
        perror("udp socket");
        close(c->tcp_socket);
        return -1;
    }

    // Conectar TCP
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, &addr.sin_addr);

    if (connect(c->tcp_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(c->tcp_socket);
        close(c->udp_socket);
        return -1;
    }

    fcntl(c->tcp_socket, F_SETFL, O_NONBLOCK);
    fcntl(c->udp_socket, F_SETFL, O_NONBLOCK);

    printf("[CLIENT] Conectado a %s:%d\n", server_ip, port);
    return 0;
}

int ae_client_connect(AeNetClient *c, const char *player_name) {
    strncpy(c->player_name, player_name, sizeof(c->player_name) - 1);

    AeNetConnect conn;
    memset(&conn, 0, sizeof(conn));
    conn.protocol_version = AE_NET_PROTOCOL_VERSION;
    strncpy(conn.player_name, player_name, sizeof(conn.player_name) - 1);

    AeNetHeader hdr;
    hdr.type = MSG_CONNECT;
    hdr.flags = AE_NET_FLAG_RELIABLE;
    hdr.seq = 0;
    hdr.size = sizeof(conn);

    send(c->tcp_socket, &hdr, sizeof(hdr), 0);
    send(c->tcp_socket, &conn, sizeof(conn), 0);

    // Esperar ACK
    int timeout = 50; // 5 segundos (100ms * 50)
    while (timeout-- > 0) {
        AeNetHeader rhdr;
        int bytes = recv(c->tcp_socket, &rhdr, sizeof(rhdr), MSG_DONTWAIT);
        if (bytes > 0 && rhdr.type == MSG_CONNECT_ACK) {
            // Leer payload completo
            char ack_buf[256];
            int total = 0;
            while (total < (int)rhdr.size) {
                int r = recv(c->tcp_socket, ack_buf + total, rhdr.size - total, 0);
                if (r <= 0) break;
                total += r;
            }
            if (total == (int)rhdr.size) {
                AeNetConnectAck *ack = (AeNetConnectAck*)ack_buf;
                if (ack->result == 0) {
                    c->player_id = ack->player_id;
                    c->connected = 1;
                    printf("[CLIENT] Conexion aceptada. Player ID: %u\n", c->player_id);
                    return 0;
                } else {
                    printf("[CLIENT] Conexion rechazada: %s\n", ack->error_msg);
                    return -1;
                }
            }
        } else if (bytes > 0 && rhdr.type == MSG_ERROR) {
            char err_buf[512];
            int total = 0;
            while (total < (int)rhdr.size) {
                int r = recv(c->tcp_socket, err_buf + total, rhdr.size - total, 0);
                if (r <= 0) break;
                total += r;
            }
            AeNetError *err = (AeNetError*)err_buf;
            printf("[CLIENT] Error del servidor: %s\n", err->error_msg);
            return -1;
        }
        usleep(100000); // 100ms
    }

    printf("[CLIENT] Timeout esperando ACK\n");
    return -1;
}

void ae_client_disconnect(AeNetClient *c) {
    if (c->connected) {
        AeNetHeader hdr;
        hdr.type = MSG_DISCONNECT;
        hdr.flags = AE_NET_FLAG_RELIABLE;
        hdr.seq = 0;
        hdr.size = 0;
        send(c->tcp_socket, &hdr, sizeof(hdr), 0);
        c->connected = 0;
    }
    close(c->tcp_socket);
    close(c->udp_socket);
    printf("[CLIENT] Desconectado\n");
}

void ae_client_send_move(AeNetClient *c, float x, float y, float z, float yaw, uint8_t anim) {
    if (!c->connected) return;

    AeNetPlayerMove msg;
    msg.player_id = c->player_id;
    msg.pos.x = x;
    msg.pos.y = y;
    msg.pos.z = z;
    msg.rot.yaw = yaw;
    msg.anim_state = anim;
    msg.flags = 0;
    msg.timestamp = (uint32_t)time(NULL);

    AeNetHeader hdr;
    hdr.type = MSG_PLAYER_MOVE;
    hdr.flags = 0; // UDP
    hdr.seq = 0;
    hdr.size = sizeof(msg);

    // Enviar por TCP por ahora (simplificado)
    send(c->tcp_socket, &hdr, sizeof(hdr), 0);
    send(c->tcp_socket, &msg, sizeof(msg), 0);
}

void ae_client_send_chat(AeNetClient *c, const char *message) {
    if (!c->connected) return;

    AeNetChatMsg msg;
    memset(&msg, 0, sizeof(msg));
    msg.sender_id = c->player_id;
    strncpy(msg.sender_name, c->player_name, sizeof(msg.sender_name) - 1);
    strncpy(msg.message, message, sizeof(msg.message) - 1);
    msg.channel = 0; // global

    AeNetHeader hdr;
    hdr.type = MSG_CHAT_MESSAGE;
    hdr.flags = AE_NET_FLAG_RELIABLE;
    hdr.seq = 0;
    hdr.size = sizeof(msg);

    send(c->tcp_socket, &hdr, sizeof(hdr), 0);
    send(c->tcp_socket, &msg, sizeof(msg), 0);
}

void ae_client_send_monkey_caught(AeNetClient *c, uint32_t monkey_id) {
    if (!c->connected) return;

    AeNetMonkeyCaught msg;
    msg.monkey_id = monkey_id;
    msg.catcher_player_id = c->player_id;
    msg.monkey_kind = 1;

    AeNetHeader hdr;
    hdr.type = MSG_MONKEY_CAUGHT;
    hdr.flags = AE_NET_FLAG_RELIABLE;
    hdr.seq = 0;
    hdr.size = sizeof(msg);

    send(c->tcp_socket, &hdr, sizeof(hdr), 0);
    send(c->tcp_socket, &msg, sizeof(msg), 0);
}

void ae_client_send_area_enter(AeNetClient *c, uint16_t area_id) {
    if (!c->connected) return;

    AeNetAreaEnter msg;
    msg.player_id = c->player_id;
    msg.area_id = area_id;
    msg.entry_point = 0;

    AeNetHeader hdr;
    hdr.type = MSG_AREA_ENTER;
    hdr.flags = AE_NET_FLAG_RELIABLE;
    hdr.seq = 0;
    hdr.size = sizeof(msg);

    send(c->tcp_socket, &hdr, sizeof(hdr), 0);
    send(c->tcp_socket, &msg, sizeof(msg), 0);
    c->area_id = area_id;
    c->in_area = 1;
}

void ae_client_poll(AeNetClient *c) {
    if (!c->connected) return;

    // Leer mensajes pendientes
    while (1) {
        AeNetHeader hdr;
        int bytes = recv(c->tcp_socket, &hdr, sizeof(hdr), MSG_DONTWAIT);
        if (bytes <= 0) break;
        if (bytes < (int)sizeof(hdr)) break;

        char payload[4096];
        int pb = recv(c->tcp_socket, payload, hdr.size, MSG_DONTWAIT);
        if (pb <= 0) break;

        switch (hdr.type) {
            case MSG_PLAYER_MOVE: {
                AeNetPlayerMoveBroadcast *move = (AeNetPlayerMoveBroadcast*)payload;
                // Buscar o crear jugador remoto
                int found = -1;
                for (int i = 0; i < c->remote_player_count; i++) {
                    if (c->remote_players[i].player_id == move->player_id) {
                        found = i;
                        break;
                    }
                }
                if (found < 0 && c->remote_player_count < 64) {
                    found = c->remote_player_count++;
                    c->remote_players[found].player_id = move->player_id;
                    c->remote_players[found].active = 1;
                }
                if (found >= 0) {
                    c->remote_players[found].pos_x = move->pos.x;
                    c->remote_players[found].pos_y = move->pos.y;
                    c->remote_players[found].pos_z = move->pos.z;
                    c->remote_players[found].rot_yaw = move->rot.yaw;
                    c->remote_players[found].anim_state = move->anim_state;
                    c->remote_players[found].last_update = (uint32_t)time(NULL);
                }
                break;
            }

            case MSG_MONKEY_MOVE: {
                AeNetMonkeyState *ms = (AeNetMonkeyState*)payload;
                int found = -1;
                for (int i = 0; i < c->monkey_count; i++) {
                    if (c->monkeys[i].monkey_id == ms->monkey_id) {
                        found = i;
                        break;
                    }
                }
                if (found < 0 && c->monkey_count < 256) {
                    found = c->monkey_count++;
                    c->monkeys[found].monkey_id = ms->monkey_id;
                    c->monkeys[found].active = 1;
                }
                if (found >= 0) {
                    c->monkeys[found].pos_x = ms->pos.x;
                    c->monkeys[found].pos_y = ms->pos.y;
                    c->monkeys[found].pos_z = ms->pos.z;
                    c->monkeys[found].rot_yaw = ms->rot.yaw;
                    c->monkeys[found].ai_state = ms->ai_state;
                    c->monkeys[found].activity = ms->activity;
                }
                break;
            }

            case MSG_MONKEY_CAUGHT: {
                AeNetMonkeyCaught *mc = (AeNetMonkeyCaught*)payload;
                printf("[CLIENT] Mono %u capturado por jugador %u\n", mc->monkey_id, mc->catcher_player_id);
                for (int i = 0; i < c->monkey_count; i++) {
                    if (c->monkeys[i].monkey_id == mc->monkey_id) {
                        c->monkeys[i].activity = 3; // caught
                        break;
                    }
                }
                break;
            }

            case MSG_CHAT_MESSAGE: {
                AeNetChatMsg *chat = (AeNetChatMsg*)payload;
                printf("[CHAT] %s: %s\n", chat->sender_name, chat->message);
                break;
            }

            case MSG_ERROR: {
                AeNetError *err = (AeNetError*)payload;
                printf("[CLIENT] Error: %s\n", err->error_msg);
                break;
            }

            default:
                break;
        }
    }
}

void ae_client_update(AeNetClient *c) {
    // Limpiar jugadores remotos inactivos (timeout 5s)
    uint32_t now = (uint32_t)time(NULL);
    for (int i = 0; i < c->remote_player_count; i++) {
        if (c->remote_players[i].active && (now - c->remote_players[i].last_update) > 5) {
            c->remote_players[i].active = 0;
        }
    }
}
