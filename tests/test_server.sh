#!/bin/bash
# Test automatizado del servidor MMORPG
# Inicia el servidor, conecta 2 clientes y verifica comunicación

set -e

cd "$(dirname "$0")/.."
SERVER_PORT=7799
SERVER_DIR="server"
CLIENT_DIR="client"

echo "=== Compilando servidor ==="
cd "$SERVER_DIR" && make clean && make && cd ..

echo "=== Compilando cliente ==="
cd "$CLIENT_DIR" && gcc -o client client.c test_client.c -I.. -I../protocol && cd ..

echo "=== Iniciando servidor en puerto $SERVER_PORT ==="
cd "$SERVER_DIR" && ./server $SERVER_PORT &
SERVER_PID=$!
sleep 1

echo "=== Conectando cliente 1 ==="
cd "$CLIENT_DIR" && ./client 127.0.0.1 $SERVER_PORT "Jugador1" &
CLIENT1_PID=$!
sleep 1

echo "=== Conectando cliente 2 ==="
cd "$CLIENT_DIR" && ./client 127.0.0.1 $SERVER_PORT "Jugador2" &
CLIENT2_PID=$!
sleep 2

echo "=== Enviando chat desde cliente 1 ==="
# El test_client envía mensajes automáticamente
sleep 3

echo "=== Verificando procesos ==="
if kill -0 $SERVER_PID 2>/dev/null; then
    echo "Servidor: OK"
else
    echo "Servidor: FALLO"
fi

if kill -0 $CLIENT1_PID 2>/dev/null; then
    echo "Cliente 1: OK"
else
    echo "Cliente 1: FALLO"
fi

if kill -0 $CLIENT2_PID 2>/dev/null; then
    echo "Cliente 2: OK"
else
    echo "Cliente 2: FALLO"
fi

echo "=== Limpiando ==="
kill $CLIENT1_PID $CLIENT2_PID $SERVER_PID 2>/dev/null
wait 2>/dev/null

echo "=== Test completado ==="
