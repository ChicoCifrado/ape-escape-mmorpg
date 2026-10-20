# Protocolo de Red - Ape Escape MMORPG

## Formato de Mensaje

Todos los mensajes usan un header común de 8 bytes:

```
+--------+--------+--------+--------+--------+--------+--------+--------+
|  type  | flags  |       seq (u16)       |       size (u32)          |
+--------+--------+--------+--------+--------+--------+--------+--------+
```

- **type**: Tipo de mensaje (AeNetMsgType)
- **flags**: AE_NET_FLAG_RELIABLE (0x01), AE_NET_FLAG_COMPRESSED (0x02), AE_NET_FLAG_BROADCAST (0x04)
- **seq**: Número de secuencia para ordenamiento
- **size**: Tamaño del payload en bytes

## Transporte

| Canal | Protocolo | Uso |
|-------|-----------|-----|
| TCP | Puerto 7777 | Eventos confiables (auth, chat, captura, items) |
| UDP | Puerto 7778 | Posición/rotación (baja latencia) |

## Flujo de Conexión

```
Cliente                          Servidor
  |                                |
  |--- MSG_CONNECT (TCP) --------->|
  |    {protocol_version, name}    |
  |                                |
  |<-- MSG_CONNECT_ACK (TCP) ------|
  |    {player_id, result, ts}     |
  |                                |
  |--- MSG_AREA_ENTER (TCP) ------>|
  |    {player_id, area_id}        |
  |                                |
  |<-- MSG_PLAYER_SPAWN (TCP) -----|  (para cada jugador existente)
  |                                |
  |--- MSG_PLAYER_MOVE (UDP) ----->|  (cada frame)
  |    {player_id, pos, rot}       |
  |                                |
  |<-- MSG_PLAYER_MOVE (UDP) ------|  (broadcast a otros)
  |                                |
  |--- MSG_CHAT_MESSAGE (TCP) ---->|
  |    {sender, message}           |
  |                                |
  |<-- MSG_CHAT_MESSAGE (TCP) -----|  (relay a área)
  |                                |
  |--- MSG_MONKEY_CAUGHT (TCP) --->|
  |    {monkey_id}                 |
  |                                |
  |<-- MSG_MONKEY_CAUGHT (TCP) ----|  (broadcast)
  |                                |
```

## Tipos de Mensaje

### Conexión
| Código | Nombre | Dirección | Descripción |
|--------|--------|-----------|-------------|
| 0x01 | MSG_CONNECT | C→S | Solicitud de conexión |
| 0x02 | MSG_CONNECT_ACK | S→C | Confirmación de conexión |
| 0x03 | MSG_DISCONNECT | C→S | Desconexión |
| 0x04 | MSG_HEARTBEAT | C↔S | Keep-alive |

### Jugador
| Código | Nombre | Dirección | Descripción |
|--------|--------|-----------|-------------|
| 0x10 | MSG_PLAYER_SPAWN | S→C | Jugador aparece en área |
| 0x11 | MSG_PLAYER_DESPAWN | S→C | Jugador sale del área |
| 0x12 | MSG_PLAYER_MOVE | C→S/S→C | Posición/rotación |
| 0x13 | MSG_PLAYER_MOVE_ACK | S→C | Confirmación de movimiento |
| 0x14 | MSG_PLAYER_ANIM | C→S | Estado de animación |
| 0x15 | MSG_PLAYER_DAMAGE | S→C | Daño recibido |
| 0x16 | MSG_PLAYER_DEATH | S→C | Muerte del jugador |
| 0x17 | MSG_PLAYER_RESPAWN | S→C | Respawn |

### Monos
| Código | Nombre | Dirección | Descripción |
|--------|--------|-----------|-------------|
| 0x20 | MSG_MONKEY_SPAWN | S→C | Mono aparece |
| 0x21 | MSG_MONKEY_DESPAWN | S→C | Mono desaparece |
| 0x22 | MSG_MONKEY_MOVE | S→C | Posición/estado del mono |
| 0x23 | MSG_MONKEY_CAUGHT | C→S/S→C | Mono capturado |
| 0x24 | MSG_MONKEY_STATE | S→C | Cambio de estado AI |

### Enemigos (Zako)
| Código | Nombre | Dirección | Descripción |
|--------|--------|-----------|-------------|
| 0x30 | MSG_ENEMY_SPAWN | S→C | Enemigo aparece |
| 0x31 | MSG_ENEMY_DESPAWN | S→C | Enemigo desaparece |
| 0x32 | MSG_ENEMY_MOVE | S→C | Posición/estado |
| 0x33 | MSG_ENEMY_DAMAGE | C→S | Daño al enemigo |
| 0x34 | MSG_ENEMY_DEATH | S→C | Muerte del enemigo |

### Items
| Código | Nombre | Dirección | Descripción |
|--------|--------|-----------|-------------|
| 0x40 | MSG_ITEM_SPAWN | S→C | Item aparece |
| 0x41 | MSG_ITEM_PICKUP | C→S | Recoger item |
| 0x42 | MSG_ITEM_DROP | C→S | Soltar item |

### Chat
| Código | Nombre | Dirección | Descripción |
|--------|--------|-----------|-------------|
| 0x50 | MSG_CHAT_MESSAGE | C↔S | Mensaje de chat |
| 0x51 | MSG_CHAT_SYSTEM | S→C | Mensaje del sistema |

### Área
| Código | Nombre | Dirección | Descripción |
|--------|--------|-----------|-------------|
| 0x60 | MSG_AREA_ENTER | C→S | Entrar a área |
| 0x61 | MSG_AREA_EXIT | C→S | Salir de área |
| 0x62 | MSG_AREA_STATE | S→C | Estado completo del área |

### Inventario
| Código | Nombre | Dirección | Descripción |
|--------|--------|-----------|-------------|
| 0x70 | MSG_INVENTORY_UPDATE | S→C | Actualización de inventario |
| 0x71 | MSG_INVENTORY_USE | C→S | Usar item |

### Sistema
| Código | Nombre | Dirección | Descripción |
|--------|--------|-----------|-------------|
| 0xFF | MSG_ERROR | S→C | Error del servidor |

## Estructuras de Payload

### AeNetVec3 (12 bytes)
```
float x, y, z
```

### AeNetRot (4 bytes)
```
float yaw
```

### AeNetPlayerMove (C→S, UDP)
```
uint32_t player_id
AeNetVec3 pos (12 bytes)
AeNetRot rot (4 bytes)
uint8_t anim_state
uint8_t flags
uint32_t timestamp
Total: 28 bytes
```

### AeNetPlayerMoveBroadcast (S→C, UDP)
```
uint32_t player_id
AeNetVec3 pos (12 bytes)
AeNetRot rot (4 bytes)
uint8_t anim_state
uint8_t flags
uint32_t server_timestamp
Total: 28 bytes
```

### AeNetMonkeyState (S→C, UDP/TCP)
```
uint32_t monkey_id
AeNetVec3 pos (12 bytes)
AeNetRot rot (4 bytes)
uint8_t ai_state
uint8_t activity
uint8_t health
uint32_t flags
Total: 28 bytes
```

### AeNetMonkeyCaught (S→C, TCP)
```
uint32_t monkey_id
uint32_t catcher_player_id
uint8_t monkey_kind
Total: 9 bytes
```

### AeNetChatMsg (C↔S, TCP)
```
uint32_t sender_id
char sender_name[32]
char message[256]
uint8_t channel
Total: 293 bytes
```

### AeNetAreaEnter (C→S, TCP)
```
uint32_t player_id
uint16_t area_id
uint16_t entry_point
Total: 8 bytes
```

### AeNetConnect (C→S, TCP)
```
uint32_t protocol_version
char player_name[32]
char auth_token[64]
Total: 100 bytes
```

### AeNetConnectAck (S→C, TCP)
```
uint32_t player_id
uint8_t result
char error_msg[128]
uint32_t server_timestamp
Total: 137 bytes
```

### AeNetError (S→C, TCP)
```
uint16_t error_code
char error_msg[256]
Total: 258 bytes
```

## Códigos de Error

| Código | Descripción |
|--------|-------------|
| 0x0001 | Protocolo incompatible |
| 0x0002 | Nombre de jugador inválido |
| 0x0003 | Autenticación fallida |
| 0x0004 | Área no encontrada |
| 0x0005 | Servidor lleno |
| 0x0006 | Jugador ya conectado |
| 0x0007 | Timeout de conexión |
| 0x0008 | Acción no permitida |

## Canales de Chat

| Canal | ID | Descripción |
|-------|----|-------------|
| Global | 0 | Todos los jugadores del servidor |
| Área | 1 | Jugadores en la misma área |
| Party | 2 | Miembros del grupo |
| Guild | 3 | Miembros del guild |

## Reglas de Validación del Servidor

1. **Posición**: El servidor valida que la posición del jugador sea alcanzable (velocidad máxima, colisión)
2. **Captura de mono**: El servidor valida que el mono esté en el área y no esté ya capturado
3. **Chat**: El servidor filtra mensajes (longitud máxima, rate limiting)
4. **Área**: El servidor valida que el área exista y el jugador tenga acceso
5. **Autenticación**: El servidor valida el token de autenticación y la versión del protocolo
