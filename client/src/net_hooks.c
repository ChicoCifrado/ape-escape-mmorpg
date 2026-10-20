// net_hooks.c - Hooks de red para integración decomp + MMORPG
// Implementa los 5 puntos de inyección de red en el game loop

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "net_lib.h"
#include "client.h"
#include "protocol.h"

// Estado global del cliente de red
static AeNetClient *g_net_client = NULL;
static int g_net_initialized = 0;

// Punto 1: Después de PadUpdate - leer mensajes del servidor
void ae_net_hook_pad_update(void) {
    if (!g_net_client || !g_net_client->connected) return;
    ae_client_poll(g_net_client);
    ae_client_update(g_net_client);
}

// Punto 2: Después de PlayerUpdate - enviar posición
void ae_net_hook_player_update(float x, float y, float z, float yaw, uint8_t anim) {
    if (!g_net_client || !g_net_client->connected) return;
    ae_client_send_move(g_net_client, x, y, z, yaw, anim);
}

// Punto 3: Después de MonkeyUpdateAll - sincronizar monos
void ae_net_hook_monkey_update(void) {
    if (!g_net_client || !g_net_client->connected) return;
    // El servidor envía estado de monos periódicamente
    // El cliente actualiza su estado local con los datos del servidor
    // (ya manejado en ae_client_poll)
}

// Punto 4: Después de ZakoUpdateAll - sincronizar enemigos
void ae_net_hook_enemy_update(void) {
    if (!g_net_client || !g_net_client->connected) return;
    // El servidor envía estado de enemigos periódicamente
}

// Punto 5: Después de ItemsUpdateDraw - sincronizar items
void ae_net_hook_items_update(void) {
    if (!g_net_client || !g_net_client->connected) return;
    // El servidor envía estado de items periódicamente
}

// Inicializar red
int ae_net_hooks_init(const char *server_ip, uint16_t port, const char *player_name) {
    if (g_net_initialized) return 0;

    ae_net_init();

    g_net_client = (AeNetClient *)calloc(1, sizeof(AeNetClient));
    if (!g_net_client) return -1;

    if (ae_client_init(g_net_client, server_ip, port) != 0) {
        free(g_net_client);
        g_net_client = NULL;
        return -1;
    }

    if (ae_client_connect(g_net_client, player_name) != 0) {
        free(g_net_client);
        g_net_client = NULL;
        return -1;
    }

    g_net_initialized = 1;
    printf("[NET] Conectado al servidor %s:%d como %s\n", server_ip, port, player_name);
    return 0;
}

// Desconectar red
void ae_net_hooks_shutdown(void) {
    if (g_net_client) {
        ae_client_disconnect(g_net_client);
        free(g_net_client);
        g_net_client = NULL;
    }
    ae_net_cleanup();
    g_net_initialized = 0;
}

// Obtener cliente de red (para uso externo)
AeNetClient *ae_net_hooks_get_client(void) {
    return g_net_client;
}
