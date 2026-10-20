# CMakeLists.txt para cliente MMORPG Ape Escape

cmake_minimum_required(VERSION 3.16)
project(ae_mmorpg_client C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Directorios
set(DECOMP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../decomp")
set(PROTOCOL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../protocol")

# Incluir directorios
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${DECOMP_DIR}/src
    ${PROTOCOL_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared
    ${CMAKE_CURRENT_SOURCE_DIR}/../protocol
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Fuentes del cliente
file(GLOB_RECURSE CLIENT_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

# Excluir game_viewer.c (tiene includes relativos incompatibles)
list(REMOVE_ITEM CLIENT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/game_viewer.c")

# Librería de red
set(NET_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/net_lib.c
)

# Cliente de red
set(CLIENT_NET_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/client.c
)

# Fuentes del decomp (todos los .c)
file(GLOB_RECURSE DECOMP_ALL_SOURCES
    "${DECOMP_DIR}/src/*.c"
)

# Excluir archivos con main() o includes incompatibles
list(REMOVE_ITEM DECOMP_ALL_SOURCES
    "${DECOMP_DIR}/src/app/native_viewer.c"
    "${DECOMP_DIR}/src/app/test_input.c"
    "${DECOMP_DIR}/src/audio/sp_render_wav.c"
    "${DECOMP_DIR}/src/player/test_body.c"
    "${DECOMP_DIR}/src/player/test_gadget.c"
    "${DECOMP_DIR}/src/player/test_player.c"
    "${DECOMP_DIR}/src/player/test_volume.c"
)

set(DECOMP_SOURCES ${DECOMP_ALL_SOURCES})

# Ejecutable
add_executable(ae_mmorpg_client
    ${CLIENT_SOURCES}
    ${NET_SOURCES}
    ${CLIENT_NET_SOURCES}
    ${DECOMP_SOURCES}
)

# Librerias
find_package(SDL3 REQUIRED)
find_package(OpenGL REQUIRED)

target_link_libraries(ae_mmorpg_client
    SDL3::SDL3
    OpenGL::GL
    m
    pthread
)

# Definiciones
target_compile_definitions(ae_mmorpg_client PRIVATE
    AE_MMORPG_CLIENT=1
    AE_NET_ENABLED=1
)
