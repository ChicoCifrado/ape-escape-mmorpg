# Ape Escape MMORPG - Arquitectura de Integracion

## Objetivo
Convertir el codigo decompilado de Ape Escape (single-player) en un cliente MMORPG, manteniendo la logica original y anadiendo capa de red.

## Puntos de inyeccion en el game loop

El loop principal esta en src/game/ae_modes.c::stage_frame_body(). Los puntos de inyeccion son:

### 1. Input (despues de PadUpdate)
- Inyectar: leer input de red (mensajes del servidor)
- Inyectar: aplicar movimiento de jugadores remotos

### 2. PlayerUpdate (linea ~236)
- Original: CALL0(PlayerUpdate, 0x80060780u)
- Inyectar ANTES: enviar posicion local al servidor (ae_client_send_move)
- Inyectar DESPUES: recibir correcciones del servidor
- Inyectar: renderizar jugadores remotos (ghosts)

### 3. MonkeyUpdateAll (linea ~252)
- Original: CALL0(MonkeyUpdateAll, 0x80042E18u)
- Inyectar ANTES: recibir estado de monos del servidor
- Inyectar DESPUES: enviar captura de mono al servidor (ae_client_send_monkey_caught)
- Inyectar: aplicar estado de monos del servidor (posicion, AI state, activity)

### 4. ZakoUpdateAll (linea ~251)
- Original: CALL0(ZakoUpdateAll, 0x80050C34u)
- Inyectar: recibir estado de enemigos del servidor
- Inyectar: enviar danio a enemigos al servidor

### 5. ItemsUpdateDraw (linea ~256)
- Original: CALL0(ItemsUpdateDraw, 0x80084598u)
- Inyectar: recibir items del servidor
- Inyectar: enviar pickup de item al servidor

### 6. CameraUpdate (linea ~229)
- Original: CALL0(CameraUpdate, 0x80047BD0u)
- Inyectar: opcion de camara MMO (seguir jugador local, no camara original)

## Funciones nuevas necesarias

### Cliente (C)
1. ae_net_init() - Inicializar networking
2. ae_net_connect() - Conectar al servidor
3. ae_net_send_player_move() - Enviar posicion
4. ae_net_recv_player_moves() - Recibir posiciones de otros jugadores
5. ae_net_render_remote_players() - Renderizar otros jugadores
6. ae_net_sync_monkeys() - Sincronizar monos con servidor
7. ae_net_sync_enemies() - Sincronizar enemigos con servidor
8. ae_net_handle_chat() - Manejar chat
9. ae_net_handle_items() - Manejar items compartidos

### Servidor (C)
1. ae_server_accept_clients() - Aceptar conexiones
2. ae_server_handle_player_move() - Procesar movimiento
3. ae_server_broadcast_player_moves() - Broadcast posiciones
4. ae_server_handle_monkey_caught() - Procesar captura
5. ae_server_update_monkeys() - Actualizar AI de monos
6. ae_server_update_enemies() - Actualizar AI de enemigos
7. ae_server_handle_item_pickup() - Procesar pickup
8. ae_server_handle_chat() - Procesar chat
9. ae_server_persist_state() - Guardar estado

## Estructuras de datos nuevas

### Cliente: RemotePlayer
```c
typedef struct {
    uint32_t player_id;
    char name[32];
    float pos[3];
    float rot_yaw;
    uint8_t anim_state;
    uint8_t active;
    uint32_t last_update;
    // Renderizado
    uint32_t model_handle;
    uint32_t anim_frame;
} AeRemotePlayer;
```

### Servidor: AreaState
```c
typedef struct {
    uint16_t area_id;
    AeServerMonkey monkeys[64];
    int monkey_count;
    AeServerEnemy enemies[32];
    int enemy_count;
    AeServerItem items[64];
    int item_count;
    uint32_t players_in_area[64];
    int player_count;
} AeServerAreaState;
```

## Estrategia de integracion

### Paso 1: Compilar decomp con networking
- Copiar src/ del decomp a mmorpg/client/src/
- Anadir modulo net/ con cliente de red
- Anadir hooks en ae_modes.c
- Compilar con CMake

### Paso 2: Renderizado de jugadores remotos
- Usar el sistema de modelo/animacion del decomp
- Crear "ghost" players con modelo del jugador
- Sincronizar animacion con estado recibido

### Paso 3: Sincronizacion de monos
- El servidor envia estado de monos periodicamente
- El cliente aplica estado (posicion, AI state, activity)
- Cuando un jugador captura un mono, el servidor lo marca como caught
- El mono desaparece para todos los jugadores

### Paso 4: Sincronizacion de enemigos
- El servidor ejecuta AI de enemigos
- El cliente recibe posiciones y aplica
- El danio se calcula en el servidor

## Archivos a crear/modificar

### Nuevos
- mmorpg/client/src/net/ae_net_client.c
- mmorpg/client/src/net/ae_net_client.h
- mmorpg/client/src/net/ae_net_render.c
- mmorpg/client/src/net/ae_net_sync.c
- mmorpg/server/src/ae_server_area.c
- mmorpg/server/src/ae_server_monkey.c
- mmorpg/server/src/ae_server_enemy.c

### Modificar
- mmorpg/client/src/game/ae_modes.c (anadir hooks)
- mmorpg/client/src/game/ae_game_hooks.c (anadir net hooks)
- mmorpg/client/CMakeLists.txt (anadir net module)

## Dependencias
- decomp/src/ (codigo portado)
- mmorpg/protocol/ (protocolo de red)
- mmorpg/server/ (servidor)
- SDL3, OpenGL (renderizado)
- enet o custom (networking)

## Riesgos
1. El codigo decompilado opera sobre AeRam (RAM de PSX). Los jugadores remotos no estan en esta RAM.
2. El renderizado de jugadores remotos requiere un sistema de modelo separado.
3. La sincronizacion de animaciones es compleja (el decomp usa animaciones basadas en frames).
4. El servidor debe replicar la logica de AI de monos y enemigos (o delegar al cliente).
