// net_hooks.h - Hooks de red para integración decomp + MMORPG

#ifndef AE_NET_HOOKS_H
#define AE_NET_HOOKS_H

#include "../client.h"

#include <stdint.h>

// Inicializar red
int ae_net_hooks_init(const char *server_ip, uint16_t port, const char *player_name);

// Desconectar red
void ae_net_hooks_shutdown(void);

// Punto 1: Después de PadUpdate
void ae_net_hook_pad_update(void);

// Punto 2: Después de PlayerUpdate
void ae_net_hook_player_update(float x, float y, float z, float yaw, uint8_t anim);

// Punto 3: Después de MonkeyUpdateAll
void ae_net_hook_monkey_update(void);

// Punto 4: Después de ZakoUpdateAll
void ae_net_hook_enemy_update(void);

// Punto 5: Después de ItemsUpdateDraw
void ae_net_hook_items_update(void);

// Obtener cliente de red
AeNetClient *ae_net_hooks_get_client(void);

#endif // AE_NET_HOOKS_H
