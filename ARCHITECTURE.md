# Ape Escape MMORPG - Arquitectura y Roadmap

## Visión

MMORPG basado en la decompilación de Ape Escape (PS1), donde múltiples jugadores
pueden explorar las áreas del juego, capturar monos, combatir enemigos y
cooperar en tiempo real. El servidor es autoritativo y el cliente ejecuta
la lógica del juego original (decomp) con sincronización de red.

## Arquitectura

### Componentes

```
┌─────────────────────────────────────────────────────┐
│                   CLIENTE (decomp + net)             │
│  ┌──────────┐  ┌──────────┐  ┌──────────────────┐  │
│  │ Game Loop│  │ Renderer │  │ Network Client   │  │
│  │ (ae_modes)│  │ (SDL3)   │  │ (TCP/UDP)        │  │
│  └────┬─────┘  └────┬─────┘  └────────┬─────────┘  │
│       │             │                  │            │
│  ┌────┴─────────────┴──────────────────┴─────────┐  │
│  │           Integración (hooks de red)          │  │
│  │  1. PadUpdate → leer mensajes del servidor    │  │
│  │  2. PlayerUpdate → enviar posición            │  │
│  │  3. MonkeyUpdateAll → sync monos              │  │
│  │  4. ZakoUpdateAll → sync enemigos             │  │
│  │  5. ItemsUpdateDraw → sync items              │  │
│  └──────────────────────────────────────────────┘  │
└──────────────────────┬──────────────────────────────┘
                       │ TCP (eventos) / UDP (posición)
┌──────────────────────┴──────────────────────────────┐
│                   SERVIDOR (autoritativo)            │
│  ┌──────────┐  ┌──────────┐  ┌──────────────────┐  │
│  │ Auth     │  │ World    │  │ Entity Manager   │  │
│  │ (cuentas)│  │ (áreas)  │  │ (jugadores/monos)│  │
│  └──────────┘  └──────────┘  └──────────────────┘  │
│  ┌──────────┐  ┌──────────┐  ┌──────────────────┐  │
│  │ Chat     │  │ Persist  │  │ Anti-cheat       │  │
│  │ (relay)  │  │ (DB)     │  │ (validación)     │  │
│  └──────────┘  └──────────┘  └──────────────────┘  │
└─────────────────────────────────────────────────────┘
```

### Protocolo de Red

- **TCP**: Eventos confiables (auth, chat, captura de mono, items, quest)
- **UDP**: Posición/rotación (baja latencia, best-effort)
- **Header**: 8 bytes (type: u8, flags: u8, seq: u16, size: u32)
- **Mensajes existentes**: MSG_AUTH, MSG_POSITION, MSG_CHAT, MSG_MONKEY_CATCH, MSG_MONKEY_SYNC, MSG_PLAYER_JOIN, MSG_PLAYER_LEAVE

### Flujo de Juego

1. **Login**: Cliente → Servidor (auth) → Servidor valida → OK/FAIL
2. **Join Area**: Cliente carga área → Servidor notifica a otros jugadores → Otros ven nuevo jugador
3. **Game Loop**: 
   - Cliente lee input → actualiza player (decomp)
   - Cliente envía posición (UDP) cada frame
   - Servidor broadcast posición a otros jugadores en el área
   - Cliente recibe posiciones de otros jugadores → renderiza
   - Cliente detecta captura de mono → envía MSG_MONKEY_CATCH (TCP)
   - Servidor valida captura → broadcast a todos → todos ven mono capturado
4. **Chat**: Cliente envía MSG_CHAT (TCP) → Servidor relay a área → todos reciben

## Roadmap

### Fase 0: Fundación (COMPLETADA)
- [x] Decompilación de Ape Escape (1520/3611 funciones)
- [x] Servidor TCP/UDP básico
- [x] Cliente de red básico
- [x] Protocolo de red definido
- [x] Compilación y test (2 clientes)
- [x] Visor nativo (SDL3) compilado
- [x] Assets extraídos (KKIIDDZZ.DAT, STR, DEMO)
- [x] Repositorio GitHub creado

### Fase 1: Integración Decomp + Red (SIGUIENTE)
**Objetivo**: Conectar el game loop del decomp con el cliente de red

- [ ] Compilar el decomp con soporte de red (linkar librería de red)
- [ ] Implementar hooks de red en ae_modes.c (5 puntos de inyección)
- [ ] Leer mensajes del servidor en PadUpdate
- [ ] Enviar posición en PlayerUpdate
- [ ] Renderizar jugadores remotos (nuevos sprites/modelos)
- [ ] Sincronizar monos con servidor
- [ ] Test: 2 jugadores en la misma área viendo movimientos

**Archivos clave:**
- decomp/src/game/ae_modes.c (game loop)
- decomp/src/game/ae_game.c (hooks)
- mmorpg/client/client.c (red)
- mmorpg/protocol/protocol.h (mensajes)

### Fase 2: Sincronización de Entidades
**Objetivo**: Sincronizar monos, enemigos e items entre clientes

- [ ] MSG_MONKEY_SYNC: servidor envía estado de monos periódicamente
- [ ] MSG_ZAKO_SYNC: servidor envía estado de enemigos
- [ ] MSG_ITEM_SYNC: servidor envía estado de items
- [ ] Validación server-side de capturas
- [ ] Anti-cheat básico (posición válida, velocidad máxima)
- [ ] Test: 2 jugadores capturando el mismo mono (solo uno gana)

### Fase 3: Persistencia y Cuentas
**Objetivo**: Guardar progreso de jugadores

- [ ] Base de datos (SQLite o PostgreSQL)
- [ ] Sistema de cuentas (registro, login)
- [ ] Guardar: posición, monos capturados, items, nivel
- [ ] Cargar estado al hacer login
- [ ] Sistema de inventario

### Fase 4: Funciones MMO
**Objetivo**: Funcionalidades multijugador avanzadas

- [ ] Chat global y por área
- [ ] Sistema de grupos (party)
- [ ] Trading entre jugadores
- [ ] Quests compartidas
- [ ] Leaderboard (monos capturados)
- [ ] Sistema de niveles/XP

### Fase 5: Contenido y Pulido
**Objetivo**: Añadir contenido y pulir la experiencia

- [ ] Todas las áreas jugables (30+ áreas)
- [ ] Todos los tipos de monos (200+)
- [ ] Todos los enemigos (zakos)
- [ ] Efectos visuales de otros jugadores
- [ ] Sonido de otros jugadores
- [ ] Optimización de red (interpolación, predicción)

## Assets Necesarios

### Del decomp (ya disponibles)
- Modelos de player, monos, enemigos (GLTF exportables)
- Texturas (TIM → PNG)
- Audio (ADPCM → WAV)
- Mapas de áreas (terrain + props)
- Efectos (partículas, trails)
- Texto y UI

### Por crear
- Modelos de jugadores remotos (variantes de color)
- Indicadores de otros jugadores (nombre, nivel)
- Efectos de red (conexión/desconexión)
- UI de chat, inventario, party

## Tecnologías

| Componente | Tecnología |
|---|---|
| Cliente | C (decomp) + SDL3 + OpenGL |
| Servidor | C (autoritativo) |
| Red | TCP (eventos) + UDP (posición) |
| Base de datos | SQLite (dev) → PostgreSQL (prod) |
| Assets | GLTF (modelos), PNG (texturas), WAV (audio) |
| Build | CMake (cliente) + Make (servidor) |
| Version control | Git + GitHub |

## Estructura del Repositorio

```
ape-escape-mmorpg/
├── decomp/          # Decompilación (subrepo: ape-escape-decomp-port)
├── aemt/            # Asset extraction (subrepo: aemt)
├── ape-recomp/      # PSX recompilation (subrepo: ApeEscapeRecomp)
├── mmorpg/
│   ├── client/      # Cliente de red (C)
│   ├── server/      # Servidor autoritativo (C)
│   ├── protocol/    # Definiciones de protocolo
│   ├── shared/      # Código compartido
│   └── docs/        # Documentación
├── assets/          # Assets extraídos y convertidos
│   ├── models/      # GLTF
│   ├── textures/    # PNG
│   ├── audio/       # WAV
│   └── areas/       # Mapas de áreas
└── tools/           # Scripts de conversión
```

## Próximo Paso Inmediato

**Fase 1, Tarea 1**: Compilar el decomp con soporte de red

1. Crear una librería de red (net_lib.c) que envuelva socket(), connect(), send(), recv()
2. Linkearla con el decomp
3. Añadir hooks de red en ae_modes.c
4. Test básico: 2 instancias del decomp conectándose al servidor

## Referencias

- INTEGRATION.md: Arquitectura de integración decomp + red
- ROADMAP.md: Plan maestro original
- HANDOFF.md: Estado del decomp
- protocol/protocol.h: Definiciones de mensajes
- decomp/notes/INTEGRATION.md: Integración con decomp
