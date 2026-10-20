// server.c - Servidor MMORPG Ape Escape (prototipo)

#include "server.h"
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
#include <sys/select.h>
#include <time.h>

// Inicializar servidor
int ae_server_init(AeNetServer *s, uint16_t port) {
    memset(s, 0, sizeof(AeNetServer));
    s->port = port;
    s->next_player_id = 1;
    s->next_monkey_id = 1;
    s->next_enemy_id = 1;
    s->running = 0;

    // Crear socket TCP
    s->listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (s->listen_socket < 0) {
        perror("socket");
        return -1;
    }

    // Reutilizar direccion
    int opt = 1;
    setsockopt(s->listen_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Bind
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(s->listen_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(s->listen_socket);
        return -1;
    }

    // Listen
    if (listen(s->listen_socket, 16) < 0) {
        perror("listen");
        close(s->listen_socket);
        return -1;
    }

    // Socket no bloqueante
    fcntl(s->listen_socket, F_SETFL, O_NONBLOCK);

    // Crear socket UDP
    s->udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (s->udp_socket < 0) {
        perror("udp socket");
        close(s->listen_socket);
        return -1;
    }
    if (bind(s->udp_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("udp bind");
        close(s->listen_socket);
        close(s->udp_socket);
        return -1;
    }
    fcntl(s->udp_socket, F_SETFL, O_NONBLOCK);

    printf("[SERVER] Inicializado en puerto %d (TCP+UDP)\n", port);
    ae_server_load_accounts(s, AE_ACCOUNTS_FILE);
    return 0;
}

// Aceptar nuevas conexiones
void ae_server_handle_connections(AeNetServer *s) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    while (1) {
        int client_sock = accept(s->listen_socket, (struct sockaddr*)&client_addr, &addr_len);
        if (client_sock < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            perror("accept");
            break;
        }

        // Buscar slot libre
        int slot = -1;
        for (int i = 0; i < AE_SERVER_MAX_CLIENTS; i++) {
            if (s->clients[i].socket == 0) {
                slot = i;
                break;
            }
        }

        if (slot < 0) {
            printf("[SERVER] Servidor lleno, rechazando conexion\n");
            close(client_sock);
            continue;
        }

        fcntl(client_sock, F_SETFL, O_NONBLOCK);

        AeServerClient *c = &s->clients[slot];
        memset(c, 0, sizeof(AeServerClient));
        c->socket = client_sock;
        c->player_id = s->next_player_id++;
        c->connected = 0;  // Pendiente de handshake
        c->in_area = 0;
        c->area_id = 0;
        s->client_count++;

        printf("[SERVER] Nueva conexion (pendiente handshake): slot=%d, ip=%s\n",
               slot, inet_ntoa(client_addr.sin_addr));
    }
}

// Procesar mensajes de clientes
void ae_server_handle_messages(AeNetServer *s) {
    for (int i = 0; i < AE_SERVER_MAX_CLIENTS; i++) {
        AeServerClient *c = &s->clients[i];
        if (!c->connected && c->socket == 0) continue;  // Slot vacio
        if (c->socket <= 0) continue;

        // MSG_PEEK para verificar header sin consumir
        char peek_buf[sizeof(AeNetHeader)];
        int peek_bytes = recv(c->socket, peek_buf, sizeof(AeNetHeader), MSG_DONTWAIT | MSG_PEEK);
        if (peek_bytes < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) continue;
            printf("[SERVER] Cliente %u desconectado (error)\n", c->player_id);
            close(c->socket);
            c->socket = 0;
            c->connected = 0;
            s->client_count--;
            continue;
        }
        if (peek_bytes == 0) {
            if (c->connected) printf("[SERVER] Cliente %u desconectado\n", c->player_id);
            if (c->connected) {
                for (int ai = 0; ai < AE_SERVER_MAX_ACCOUNTS; ai++) {
                    if (s->accounts[ai].exists && s->accounts[ai].player_id == c->player_id) {
                        s->accounts[ai].monkeys_caught = c->monkeys_caught;
                        s->accounts[ai].score = c->score;
                        break;
                    }
                }
                ae_server_save_accounts(s, AE_ACCOUNTS_FILE);
            }
            close(c->socket);
            c->socket = 0;
            c->connected = 0;
            s->client_count--;
            continue;
        }
        if (peek_bytes < (int)sizeof(AeNetHeader)) continue;

        AeNetHeader hdr;
        memcpy(&hdr, peek_buf, sizeof(hdr));

        if (hdr.size > 4096) {
            printf("[SERVER] Mensaje demasiado grande (%u bytes), drenando\n", hdr.size);
            char drain[1024];
            while (recv(c->socket, drain, sizeof(drain), MSG_DONTWAIT) > 0) {}
            continue;
        }

        // Verificar que payload completo esta disponible
        int total_expected = sizeof(AeNetHeader) + hdr.size;
        char msg_buf[4096 + sizeof(AeNetHeader)];
        int msg_bytes = recv(c->socket, msg_buf, total_expected, MSG_DONTWAIT | MSG_PEEK);
        if (msg_bytes < total_expected) {
            continue;
        }

        // Consumir los datos
        recv(c->socket, msg_buf, total_expected, MSG_DONTWAIT);
        memcpy(&hdr, msg_buf, sizeof(AeNetHeader));
        char *payload = msg_buf + sizeof(AeNetHeader);

        // Si no esta conectado, solo aceptar MSG_CONNECT
        if (!c->connected && hdr.type != MSG_CONNECT) continue;

        // Procesar segun tipo (payload ya es char*, no array local)
        // Necesitamos que payload sea accesible en el switch
        // payload apunta a msg_buf, que es valido en este scope

        // Procesar segun tipo
        switch (hdr.type) {
            case MSG_CONNECT: {
                AeNetConnect *conn = (AeNetConnect*)payload;
                if (conn->protocol_version != AE_NET_PROTOCOL_VERSION) {
                    AeNetError err;
                    err.error_code = 1;
                    snprintf(err.error_msg, sizeof(err.error_msg), "Version de protocolo incompatible");
                    AeNetHeader eh;
                    eh.type = MSG_ERROR;
                    eh.flags = AE_NET_FLAG_RELIABLE;
                    eh.seq = 0;
                    eh.size = sizeof(err);
                    send(c->socket, &eh, sizeof(eh), 0);
                    send(c->socket, &err, sizeof(err), 0);
                    break;
                }
                strncpy(c->player_name, conn->player_name, sizeof(c->player_name) - 1);
                // Persistencia de cuentas: buscar o crear
                uint32_t acc_pid = 0;
                int acc_new = ae_server_find_or_create_account(s, conn->player_name, &acc_pid);
                if (acc_new < 0) {
                    printf("[SERVER] No se pudo crear cuenta para '%s'\n", conn->player_name);
                } else {
                    c->player_id = acc_pid;
                    // Cargar stats de la cuenta
                    for (int ai = 0; ai < AE_SERVER_MAX_ACCOUNTS; ai++) {
                        if (s->accounts[ai].exists && s->accounts[ai].player_id == acc_pid) {
                            c->monkeys_caught = s->accounts[ai].monkeys_caught;
                            c->score = s->accounts[ai].score;
                            break;
                        }
                    }
                }
                c->connected = 1;
                printf("[SERVER] Jugador '%s' (id=%u) conectado\n", c->player_name, c->player_id);

                // Enviar ACK
                AeNetConnectAck ack;
                memset(&ack, 0, sizeof(ack));
                ack.player_id = c->player_id;
                ack.result = 0;
                ack.server_timestamp = (uint32_t)time(NULL);

                AeNetHeader ah;
                ah.type = MSG_CONNECT_ACK;
                ah.flags = AE_NET_FLAG_RELIABLE;
                ah.seq = 0;
                ah.size = sizeof(ack);
                send(c->socket, &ah, sizeof(ah), 0);
                send(c->socket, &ack, sizeof(ack), 0);
                break;
            }

            case MSG_PLAYER_MOVE: {
                AeNetPlayerMove *move = (AeNetPlayerMove*)payload;
                c->pos_x = move->pos.x;
                c->pos_y = move->pos.y;
                c->pos_z = move->pos.z;
                c->rot_yaw = move->rot.yaw;
                c->anim_state = move->anim_state;
                c->last_update = (uint32_t)time(NULL);

                // Broadcast a otros clientes
                ae_server_broadcast_player_move(s, c->player_id, c->pos_x, c->pos_y, c->pos_z, c->rot_yaw, c->anim_state);
                break;
            }

            case MSG_AREA_ENTER: {
                AeNetAreaEnter *ae = (AeNetAreaEnter*)payload;
                c->area_id = ae->area_id;
                c->in_area = 1;
                printf("[SERVER] Jugador %u entro al area %u\n", c->player_id, c->area_id);
                break;
            }

            case MSG_CHAT_MESSAGE: {
                AeNetChatMsg *chat = (AeNetChatMsg*)payload;
                printf("[SERVER] Chat de %s: %s\n", chat->sender_name, chat->message);
                // Broadcast a todos los clientes del area
                for (int j = 0; j < AE_SERVER_MAX_CLIENTS; j++) {
                    if (s->clients[j].connected && s->clients[j].area_id == c->area_id) {
                        AeNetHeader ch;
                        ch.type = MSG_CHAT_MESSAGE;
                        ch.flags = AE_NET_FLAG_RELIABLE;
                        ch.seq = 0;
                        ch.size = sizeof(AeNetChatMsg);
                        send(s->clients[j].socket, &ch, sizeof(ch), 0);
                        send(s->clients[j].socket, chat, sizeof(AeNetChatMsg), 0);
                    }
                }
                break;
            }

            case MSG_MONKEY_CAUGHT: {
                // Esto viene del cliente que capturo el mono
                // El servidor valida y broadcast
                AeNetMonkeyCaught *mc = (AeNetMonkeyCaught*)payload;
                for (int m = 0; m < s->monkey_count; m++) {
                    if (s->monkeys[m].monkey_id == mc->monkey_id) {
                        s->monkeys[m].active = 3; // caught
                        s->monkeys[m].catcher_id = c->player_id;
                        c->monkeys_caught++;
                        // Persistir en cuenta
                        for (int ai = 0; ai < AE_SERVER_MAX_ACCOUNTS; ai++) {
                            if (s->accounts[ai].exists && s->accounts[ai].player_id == c->player_id) {
                                s->accounts[ai].monkeys_caught = c->monkeys_caught;
                                s->accounts[ai].score = c->score;
                                break;
                            }
                        }
                        printf("[SERVER] Mono %u capturado por jugador %u\n", mc->monkey_id, c->player_id);

                        // Broadcast a todos
                        mc->catcher_player_id = c->player_id;
                        for (int j = 0; j < AE_SERVER_MAX_CLIENTS; j++) {
                            if (s->clients[j].connected) {
                                AeNetHeader mh;
                                mh.type = MSG_MONKEY_CAUGHT;
                                mh.flags = AE_NET_FLAG_RELIABLE;
                                mh.seq = 0;
                                mh.size = sizeof(AeNetMonkeyCaught);
                                send(s->clients[j].socket, &mh, sizeof(mh), 0);
                                send(s->clients[j].socket, mc, sizeof(AeNetMonkeyCaught), 0);
                            }
                        }
                        break;
                    }
                }
                break;
            }

            default:
                printf("[SERVER] Mensaje desconocido tipo 0x%02X de jugador %u\n", hdr.type, c->player_id);
                break;
        }
    }
}

// Broadcast de posicion de jugador
void ae_server_broadcast_player_move(AeNetServer *s, uint32_t player_id, float x, float y, float z, float yaw, uint8_t anim) {
    AeNetPlayerMoveBroadcast msg;
    msg.player_id = player_id;
    msg.pos.x = x;
    msg.pos.y = y;
    msg.pos.z = z;
    msg.rot.yaw = yaw;
    msg.anim_state = anim;
    msg.flags = 0;
    msg.server_timestamp = (uint32_t)time(NULL);

    for (int i = 0; i < AE_SERVER_MAX_CLIENTS; i++) {
        if (s->clients[i].connected && s->clients[i].player_id != player_id) {
            AeNetHeader hdr;
            hdr.type = MSG_PLAYER_MOVE;
            hdr.flags = 0; // UDP, no reliable
            hdr.seq = 0;
            hdr.size = sizeof(msg);

            send(s->clients[i].socket, &hdr, sizeof(hdr), 0);
            send(s->clients[i].socket, &msg, sizeof(msg), 0);
        }
    }
}

// Broadcast de estado de mono
void ae_server_broadcast_monkey_state(AeNetServer *s, uint32_t monkey_id) {
    for (int m = 0; m < s->monkey_count; m++) {
        if (s->monkeys[m].monkey_id == monkey_id) {
            AeNetMonkeyState msg;
            msg.monkey_id = s->monkeys[m].monkey_id;
            msg.pos.x = s->monkeys[m].pos_x;
            msg.pos.y = s->monkeys[m].pos_y;
            msg.pos.z = s->monkeys[m].pos_z;
            msg.rot.yaw = s->monkeys[m].rot_yaw;
            msg.ai_state = s->monkeys[m].ai_state;
            msg.activity = s->monkeys[m].active;
            msg.health = 100;
            msg.flags = 0;

            for (int i = 0; i < AE_SERVER_MAX_CLIENTS; i++) {
                if (s->clients[i].connected) {
                    AeNetHeader hdr;
                    hdr.type = MSG_MONKEY_MOVE;
                    hdr.flags = 0;
                    hdr.seq = 0;
                    hdr.size = sizeof(msg);
                    send(s->clients[i].socket, &hdr, sizeof(hdr), 0);
                    send(s->clients[i].socket, &msg, sizeof(msg), 0);
                }
            }
            break;
        }
    }
}

// Tick del servidor
void ae_server_tick(AeNetServer *s) {
    s->tick_count++;
    s->last_tick_time = (uint32_t)time(NULL);

    // Actualizar monos (logica simplificada)
    for (int i = 0; i < s->monkey_count; i++) {
        if (s->monkeys[i].active == 1) {
            // Movimiento basico: patrullar
            // (En la version completa, esto usaria la AI del decomp)
            s->monkeys[i].pos_x += 0.1f; // placeholder
            if (s->monkeys[i].pos_x > 100.0f) s->monkeys[i].pos_x = -100.0f;
        }
    }

    // Broadcast de estado de monos periodicamente
    if (s->tick_count % 3 == 0) {
        for (int i = 0; i < s->monkey_count; i++) {
            ae_server_broadcast_monkey_state(s, s->monkeys[i].monkey_id);
        }
    }
}

// Loop principal del servidor
int ae_server_start(AeNetServer *s) {
    s->running = 1;
    printf("[SERVER] Servidor iniciado. Esperando conexiones...\n");

    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 1000000000 / AE_SERVER_TICK_RATE;

    while (s->running) {
        ae_server_handle_connections(s);
        ae_server_handle_messages(s);
        ae_server_tick(s);

        // Sleep para mantener tick rate
        nanosleep(&ts, NULL);
    }

    return 0;
}

// Persistencia de cuentas: cargar
int ae_server_load_accounts(AeNetServer *s, const char *path) {
    s->account_count = 0;
    memset(s->accounts, 0, sizeof(s->accounts));
    FILE *f = fopen(path, "r");
    if (!f) {
        printf("[SERVER] No existe archivo de cuentas '%s', iniciando vacio\n", path);
        return 0;
    }
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        if (s->account_count >= AE_SERVER_MAX_ACCOUNTS) break;
        AeServerAccount *a = &s->accounts[s->account_count];
        memset(a, 0, sizeof(*a));
        if (sscanf(line, "%31s %u %u %u", a->username, &a->player_id, &a->monkeys_caught, &a->score) == 4) {
            a->exists = 1;
            s->account_count++;
            if (a->player_id >= s->next_player_id) s->next_player_id = a->player_id + 1;
        }
    }
    fclose(f);
    printf("[SERVER] Cargadas %d cuentas desde '%s'\n", s->account_count, path);
    return 0;
}

// Persistencia de cuentas: guardar
int ae_server_save_accounts(AeNetServer *s, const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) {
        perror("fopen accounts");
        return -1;
    }
    for (int i = 0; i < AE_SERVER_MAX_ACCOUNTS; i++) {
        if (s->accounts[i].exists) {
            fprintf(f, "%s %u %u %u\n", s->accounts[i].username, s->accounts[i].player_id,
                    s->accounts[i].monkeys_caught, s->accounts[i].score);
        }
    }
    fclose(f);
    printf("[SERVER] Guardadas %d cuentas en '%s'\n", s->account_count, path);
    return 0;
}

// Buscar o crear cuenta; devuelve player_id
int ae_server_find_or_create_account(AeNetServer *s, const char *username, uint32_t *player_id) {
    if (!username || !username[0]) return -1;
    // Buscar existente
    for (int i = 0; i < AE_SERVER_MAX_ACCOUNTS; i++) {
        if (s->accounts[i].exists && strcmp(s->accounts[i].username, username) == 0) {
            if (player_id) *player_id = s->accounts[i].player_id;
            return 0;
        }
    }
    // Crear nueva
    int slot = -1;
    for (int i = 0; i < AE_SERVER_MAX_ACCOUNTS; i++) {
        if (!s->accounts[i].exists) { slot = i; break; }
    }
    if (slot < 0) return -1;
    AeServerAccount *a = &s->accounts[slot];
    memset(a, 0, sizeof(*a));
    strncpy(a->username, username, sizeof(a->username) - 1);
    a->player_id = s->next_player_id++;
    a->exists = 1;
    s->account_count++;
    if (player_id) *player_id = a->player_id;
    printf("[SERVER] Nueva cuenta creada: '%s' (player_id=%u)\n", username, a->player_id);
    return 1;
}
// Detener servidor
void ae_server_stop(AeNetServer *s) {
     s->running = 0;
     for (int i = 0; i < AE_SERVER_MAX_CLIENTS; i++) {
         if (s->clients[i].connected) {
             close(s->clients[i].socket);
             s->clients[i].connected = 0;
         }
     }
     ae_server_save_accounts(s, AE_ACCOUNTS_FILE);
     close(s->listen_socket);
     close(s->udp_socket);
     printf("[SERVER] Servidor detenido\n");
}
