// test_client.c - Cliente de prueba para MMORPG Ape Escape

#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
    const char *server_ip = "127.0.0.1";
    uint16_t port = 7777;
    const char *name = "TestPlayer";

    if (argc > 1) server_ip = argv[1];
    if (argc > 2) port = (uint16_t)atoi(argv[2]);
    if (argc > 3) name = argv[3];

    AeNetClient client;
    if (ae_client_init(&client, server_ip, port) != 0) {
        fprintf(stderr, "Error conectando al servidor\n");
        return 1;
    }

    if (ae_client_connect(&client, name) != 0) {
        fprintf(stderr, "Error en handshake\n");
        ae_client_disconnect(&client);
        return 1;
    }

    ae_client_send_area_enter(&client, 1);

    // Loop principal
    float x = 0, y = 0, z = 0;
    int running = 1;
    int tick = 0;

    printf("[CLIENT] Presiona Ctrl+C para salir\n");

    while (running) {
        // Enviar posicion
        x += 0.1f;
        ae_client_send_move(&client, x, y, z, 0.0f, 0);

        // Poll mensajes
        ae_client_poll(&client);
        ae_client_update(&client);

        // Chat cada 5 segundos
        if (tick % 50 == 0) {
            char msg[64];
            snprintf(msg, sizeof(msg), "Hola desde %s (tick %d)", name, tick);
            ae_client_send_chat(&client, msg);
        }

        // Capturar mono cada 10 segundos
        if (tick % 100 == 0 && tick > 0) {
            ae_client_send_monkey_caught(&client, 1);
        }

        tick++;
        usleep(100000); // 100ms = 10 ticks/s

        if (tick > 300) running = 0; // 30 segundos
    }

    ae_client_disconnect(&client);
    printf("[CLIENT] Fin\n");
    return 0;
}
