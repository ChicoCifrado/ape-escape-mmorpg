// game_client_main.c - Main del game_client (decomp + red)
// Prototipo mínimo que combina la lógica del decomp con networking

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "net_lib.h"
#include "client.h"
#include "net_hooks.h"

// Headers del decomp
#include "core/ae_ram.h"
#include "game/ae_modes.h"
#include "game/ae_game.h"

#define DEFAULT_SERVER_IP "127.0.0.1"
#define DEFAULT_SERVER_PORT 7777

int main(int argc, char **argv) {
    const char *server_ip = DEFAULT_SERVER_IP;
    uint16_t server_port = DEFAULT_SERVER_PORT;
    const char *player_name = "Player";

    // Parsear argumentos
    if (argc > 1) server_ip = argv[1];
    if (argc > 2) server_port = (uint16_t)atoi(argv[2]);
    if (argc > 3) player_name = argv[3];

    printf("=== Ape Escape MMORPG Client ===\n");
    printf("Servidor: %s:%d\n", server_ip, server_port);
    printf("Jugador: %s\n", player_name);

    // Inicializar red
    if (ae_net_hooks_init(server_ip, server_port, player_name) != 0) {
        fprintf(stderr, "Error: No se pudo conectar al servidor\n");
        return 1;
    }

    printf("[NET] Conectado. Iniciando game loop...\n");
    printf("[NET] Presiona Ctrl+C para salir\n");

    // Loop principal (prototipo)
    // En la versión completa, aquí se integraría el game loop del decomp
    // con los hooks de red en los 5 puntos de inyección
    int running = 1;
    int tick = 0;
    while (running) {
        // Punto 1: Leer mensajes del servidor
        ae_net_hook_pad_update();

        // Punto 2: Enviar posición (prototipo: posición fija)
        ae_net_hook_player_update(0.0f, 0.0f, 0.0f, 0.0f, 0);

        // Punto 3: Sincronizar monos
        ae_net_hook_monkey_update();

        // Punto 4: Sincronizar enemigos
        ae_net_hook_enemy_update();

        // Punto 5: Sincronizar items
        ae_net_hook_items_update();

        tick++;
        if (tick % 100 == 0) {
            printf("[GAME] Tick %d\n", tick);
        }

        // Sleep para no saturar CPU (60 FPS ~ 16ms)
        usleep(16000);
    }

    ae_net_hooks_shutdown();
    return 0;
}
