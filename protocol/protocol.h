// protocol.h - Definicion de protocolo de red para Ape Escape MMORPG
// Formato: mensajes binarios con header comun

#ifndef AE_NET_PROTOCOL_H
#define AE_NET_PROTOCOL_H

#include <stdint.h>

// Version del protocolo
#define AE_NET_PROTOCOL_VERSION 1

// Tipos de mensaje
typedef enum {
    // Conexion
    MSG_CONNECT = 0x01,
    MSG_CONNECT_ACK = 0x02,
    MSG_DISCONNECT = 0x03,
    MSG_HEARTBEAT = 0x04,

    // Jugador
    MSG_PLAYER_SPAWN = 0x10,
    MSG_PLAYER_DESPAWN = 0x11,
    MSG_PLAYER_MOVE = 0x12,       // Posicion/rotacion del jugador (UDP)
    MSG_PLAYER_MOVE_ACK = 0x13,   // Confirmacion del servidor
    MSG_PLAYER_ANIM = 0x14,      // Estado de animacion
    MSG_PLAYER_DAMAGE = 0x15,     // Danio recibido
    MSG_PLAYER_DEATH = 0x16,
    MSG_PLAYER_RESPAWN = 0x17,

    // Monos
    MSG_MONKEY_SPAWN = 0x20,
    MSG_MONKEY_DESPAWN = 0x21,
    MSG_MONKEY_MOVE = 0x22,       // Posicion/estado del mono (server->client)
    MSG_MONKEY_CAUGHT = 0x23,     // Mono capturado por un jugador
    MSG_MONKEY_STATE = 0x24,      // Cambio de estado AI

    // Enemigos (Zako)
    MSG_ENEMY_SPAWN = 0x30,
    MSG_ENEMY_DESPAWN = 0x31,
    MSG_ENEMY_MOVE = 0x32,
    MSG_ENEMY_DAMAGE = 0x33,
    MSG_ENEMY_DEATH = 0x34,

    // Items
    MSG_ITEM_SPAWN = 0x40,
    MSG_ITEM_PICKUP = 0x41,
    MSG_ITEM_DROP = 0x42,

    // Chat
    MSG_CHAT_MESSAGE = 0x50,
    MSG_CHAT_SYSTEM = 0x51,

    // Area/Zona
    MSG_AREA_ENTER = 0x60,
    MSG_AREA_EXIT = 0x61,
    MSG_AREA_STATE = 0x62,        // Estado completo del area

    // Inventario
    MSG_INVENTORY_UPDATE = 0x70,
    MSG_INVENTORY_USE = 0x71,

    // Sistema
    MSG_ERROR = 0xFF
} AeNetMsgType;

// Header comun de mensaje (8 bytes)
typedef struct {
    uint8_t type;           // Tipo de mensaje
    uint8_t flags;          // Flags (reliable, compressed, etc.)
    uint16_t seq;           // Numero de secuencia
    uint32_t size;          // Tamano del payload
} AeNetHeader;

// Flags de mensaje
#define AE_NET_FLAG_RELIABLE  0x01
#define AE_NET_FLAG_COMPRESSED 0x02
#define AE_NET_FLAG_BROADCAST 0x04

// Estructura de posicion 3D (12 bytes)
typedef struct {
    float x, y, z;
} AeNetVec3;

// Estructura de rotacion (4 bytes, quaternion simplificado o euler)
typedef struct {
    float yaw;  // Rotacion Y
} AeNetRot;

// Mensaje: Player Move (cliente -> servidor, UDP)
typedef struct {
    uint32_t player_id;
    AeNetVec3 pos;
    AeNetRot rot;
    uint8_t anim_state;
    uint8_t flags;      // En suelo, saltando, etc.
    uint32_t timestamp; // Timestamp del cliente
} AeNetPlayerMove;

// Mensaje: Player Move (servidor -> clientes, UDP)
typedef struct {
    uint32_t player_id;
    AeNetVec3 pos;
    AeNetRot rot;
    uint8_t anim_state;
    uint8_t flags;
    uint32_t server_timestamp;
} AeNetPlayerMoveBroadcast;

// Mensaje: Mono State (servidor -> clientes, UDP/TCP)
typedef struct {
    uint32_t monkey_id;
    AeNetVec3 pos;
    AeNetRot rot;
    uint8_t ai_state;
    uint8_t activity;    // 0=inactive, 3=caught, 4=active
    uint8_t health;
    uint32_t flags;
} AeNetMonkeyState;

// Mensaje: Mono Caught (servidor -> clientes, TCP, reliable)
typedef struct {
    uint32_t monkey_id;
    uint32_t catcher_player_id;
    uint8_t monkey_kind;
} AeNetMonkeyCaught;

// Mensaje: Chat (TCP, reliable)
typedef struct {
    uint32_t sender_id;
    char sender_name[32];
    char message[256];
    uint8_t channel;    // 0=global, 1=area, 2=party, 3=guild
} AeNetChatMsg;

// Mensaje: Area Enter (TCP)
typedef struct {
    uint32_t player_id;
    uint16_t area_id;
    uint16_t entry_point;
} AeNetAreaEnter;

// Mensaje: Connect (TCP)
typedef struct {
    uint32_t protocol_version;
    char player_name[32];
    char auth_token[64];
} AeNetConnect;

// Mensaje: Connect ACK (TCP)
typedef struct {
    uint32_t player_id;
    uint8_t result;     // 0=ok, 1=error
    char error_msg[128];
    uint32_t server_timestamp;
} AeNetConnectAck;

// Mensaje: Error
typedef struct {
    uint16_t error_code;
    char error_msg[256];
} AeNetError;

#endif // AE_NET_PROTOCOL_H
