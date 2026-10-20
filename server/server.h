// server.h - Servidor MMORPG Ape Escape

#ifndef AE_NET_SERVER_H
#define AE_NET_SERVER_H

#include <stdint.h>
#include <stddef.h>

#define AE_SERVER_MAX_CLIENTS 64
#define AE_SERVER_MAX_MONKEYS 256
#define AE_SERVER_MAX_ENEMIES 128
#define AE_SERVER_MAX_ACCOUNTS 1024
#define AE_ACCOUNTS_FILE "accounts.dat"
#define AE_SERVER_TICK_RATE 30  // ticks por segundo

// Estado de un cliente conectado
typedef struct {
    int socket;
    uint32_t player_id;
    char player_name[32];
    uint8_t connected;
    uint8_t in_area;
    uint16_t area_id;
    // Posicion actual
    float pos_x, pos_y, pos_z;
    float rot_yaw;
    uint8_t anim_state;
    uint32_t last_update;
    // Contadores
    uint32_t monkeys_caught;
    uint32_t score;
} AeServerClient;

// Cuenta persistente de jugador
typedef struct {
    char username[32];
    uint32_t player_id;
    uint32_t monkeys_caught;
    uint32_t score;
    uint8_t exists;
} AeServerAccount;

// Estado de un mono en el servidor
typedef struct {
    uint32_t monkey_id;
    uint8_t active;       // 0=inactivo, 1=activo, 3=capturado
    uint8_t ai_state;
    uint8_t kind;
    float pos_x, pos_y, pos_z;
    float rot_yaw;
    uint32_t area_id;
    uint32_t spawn_timer;
    uint32_t catcher_id;  // 0 si no capturado
} AeServerMonkey;

// Estado de un enemigo
typedef struct {
    uint32_t enemy_id;
    uint8_t active;
    uint8_t type;
    float pos_x, pos_y, pos_z;
    float rot_yaw;
    int16_t health;
    uint32_t area_id;
    uint32_t target_player; // Jugador al que persigue
} AeServerEnemy;

// Estado del servidor
typedef struct {
    int listen_socket;
    int udp_socket;
    uint16_t port;
    uint8_t running;
    uint32_t next_player_id;
    uint32_t next_monkey_id;
    uint32_t next_enemy_id;
    uint32_t tick_count;
    uint32_t last_tick_time;

    AeServerClient clients[AE_SERVER_MAX_CLIENTS];
    int client_count;

    AeServerAccount accounts[AE_SERVER_MAX_ACCOUNTS];
    int account_count;

    AeServerMonkey monkeys[AE_SERVER_MAX_MONKEYS];
    int monkey_count;

    AeServerEnemy enemies[AE_SERVER_MAX_ENEMIES];
    int enemy_count;
} AeNetServer;

// Funciones del servidor
int ae_server_init(AeNetServer *s, uint16_t port);
int ae_server_start(AeNetServer *s);
void ae_server_stop(AeNetServer *s);
void ae_server_tick(AeNetServer *s);
void ae_server_handle_connections(AeNetServer *s);
void ae_server_handle_messages(AeNetServer *s);
void ae_server_broadcast_monkey_state(AeNetServer *s, uint32_t monkey_id);
void ae_server_broadcast_player_move(AeNetServer *s, uint32_t player_id, float x, float y, float z, float yaw, uint8_t anim);

// Persistencia de cuentas
int ae_server_load_accounts(AeNetServer *s, const char *path);
int ae_server_save_accounts(AeNetServer *s, const char *path);
int ae_server_find_or_create_account(AeNetServer *s, const char *username, uint32_t *player_id);

#endif // AE_NET_SERVER_H
