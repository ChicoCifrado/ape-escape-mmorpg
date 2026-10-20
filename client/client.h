// client.h - Cliente de red para Ape Escape MMORPG

#ifndef AE_NET_CLIENT_H
#define AE_NET_CLIENT_H

#include <stdint.h>
#include "../protocol/protocol.h"

#define AE_CLIENT_SERVER_PORT 7777
#define AE_CLIENT_SERVER_IP "127.0.0.1"

typedef struct {
    int tcp_socket;
    int udp_socket;
    uint32_t player_id;
    char player_name[32];
    uint8_t connected;
    uint8_t in_area;
    uint16_t area_id;
    // Posicion local
    float pos_x, pos_y, pos_z;
    float rot_yaw;
    uint8_t anim_state;
    // Otros jugadores
    struct {
        uint32_t player_id;
        char name[32];
        float pos_x, pos_y, pos_z;
        float rot_yaw;
        uint8_t anim_state;
        uint8_t active;
        uint32_t last_update;
    } remote_players[64];
    int remote_player_count;
    // Monos
    struct {
        uint32_t monkey_id;
        float pos_x, pos_y, pos_z;
        float rot_yaw;
        uint8_t ai_state;
        uint8_t activity;
        uint8_t active;
    } monkeys[256];
    int monkey_count;
} AeNetClient;

int ae_client_init(AeNetClient *c, const char *server_ip, uint16_t port);
int ae_client_connect(AeNetClient *c, const char *player_name);
void ae_client_disconnect(AeNetClient *c);
void ae_client_send_move(AeNetClient *c, float x, float y, float z, float yaw, uint8_t anim);
void ae_client_send_chat(AeNetClient *c, const char *message);
void ae_client_send_monkey_caught(AeNetClient *c, uint32_t monkey_id);
void ae_client_send_area_enter(AeNetClient *c, uint16_t area_id);
void ae_client_poll(AeNetClient *c);
void ae_client_update(AeNetClient *c);

#endif // AE_NET_CLIENT_H
