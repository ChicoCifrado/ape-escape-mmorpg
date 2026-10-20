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
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${DECOMP_DIR}/src
    ${PROTOCOL_DIR}
)

# Fuentes del cliente
file(GLOB_RECURSE CLIENT_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

# Fuentes del decomp (seleccionadas)
# Nota: en la version completa, incluir todos los modulos del decomp
# Por ahora, solo los modulos necesarios para el prototipo
set(DECOMP_SOURCES
    ${DECOMP_DIR}/src/core/ae_ram.c
    ${DECOMP_DIR}/src/core/ae_pad.c
    ${DECOMP_DIR}/src/player/ae_playerupdate.c
    ${DECOMP_DIR}/src/player/ae_playerupdate_frame.c
    ${DECOMP_DIR}/src/player/ae_player_act.c
    ${DECOMP_DIR}/src/player/ae_player_cond.c
    ${DECOMP_DIR}/src/player/ae_player_spawn.c
    ${DECOMP_DIR}/src/monkeys/ae_monkey_base.c
    ${DECOMP_DIR}/src/monkeys/ae_monkey_level1.c
    ${DECOMP_DIR}/src/camera/ae_camera.c
    ${DECOMP_DIR}/src/collision/ae_collprobe.c
    ${DECOMP_DIR}/src/collision/native_collision.c
    ${DECOMP_DIR}/src/collision/native_ground.c
    ${DECOMP_DIR}/src/math/ae_sdk.c
    ${DECOMP_DIR}/src/math/ae_sdk_matrix.c
    ${DECOMP_DIR}/src/bodies/ae_bodies.c
    ${DECOMP_DIR}/src/effects/effect_vm.c
    ${DECOMP_DIR}/src/audio/sp_audio.c
    ${DECOMP_DIR}/src/text/tl_text.c
    ${DECOMP_DIR}/src/render/native_prop_draw.c
    ${DECOMP_DIR}/src/render/native_sky.c
    ${DECOMP_DIR}/src/indicators/ae_indicators.c
    ${DECOMP_DIR}/src/world/ae_stage.c
    ${DECOMP_DIR}/src/game/ae_modes.c
    ${DECOMP_DIR}/src/game/ae_calls.c
    ${DECOMP_DIR}/src/game/ae_game_hooks.c
    ${DECOMP_DIR}/src/app/native_viewer.c
    ${DECOMP_DIR}/src/app/native_input.c
)

# Ejecutable
add_executable(ae_mmorpg_client
    ${CLIENT_SOURCES}
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
