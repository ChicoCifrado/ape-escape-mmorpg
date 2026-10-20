// main.c - Entry point del servidor MMORPG Ape Escape

#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

static AeNetServer *g_server = NULL;

void signal_handler(int sig) {
    printf("\n[SERVER] Senal %d recibida, deteniendo...\n", sig);
    if (g_server) {
        ae_server_stop(g_server);
    }
    exit(0);
}

int main(int argc, char **argv) {
    uint16_t port = 7777;
    if (argc > 1) {
        port = (uint16_t)atoi(argv[1]);
    }

    AeNetServer server;
    g_server = &server;

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    if (ae_server_init(&server, port) != 0) {
        fprintf(stderr, "Error inicializando servidor\n");
        return 1;
    }

    // Agregar algunos monos de prueba
    for (int i = 0; i < 5; i++) {
        server.monkeys[server.monkey_count].monkey_id = server.next_monkey_id++;
        server.monkeys[server.monkey_count].active = 1;
        server.monkeys[server.monkey_count].ai_state = 0;
        server.monkeys[server.monkey_count].kind = 1;
        server.monkeys[server.monkey_count].pos_x = (float)(i * 10 - 20);
        server.monkeys[server.monkey_count].pos_y = 0.0f;
        server.monkeys[server.monkey_count].pos_z = 0.0f;
        server.monkeys[server.monkey_count].rot_yaw = 0.0f;
        server.monkeys[server.monkey_count].area_id = 1;
        server.monkey_count++;
    }
    printf("[SERVER] %d monos de prueba agregados\n", server.monkey_count);

    ae_server_start(&server);

    ae_server_stop(&server);
    return 0;
}
