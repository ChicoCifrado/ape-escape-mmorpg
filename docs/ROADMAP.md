# Ape Escape MMORPG - Plan Maestro

## Resumen ejecutivo

MMORPG basado en la decompilacion de Ape Escape (PS1, SCUS-94423). Se parte del codigo portado a C (src/) como base del cliente. Se anade capa de red, servidor autoritativo y persistencia. El servidor gestiona estado compartido (monos, enemigos, jugadores). El cliente predice localmente y reconcilia.

## Estructura del proyecto

- decomp/ - Codigo decompilado de Ape Escape (port nativo, ~1500 funciones verificadas)
- aemt/ - Ape Escape Manipulation Toolkit (extraccion de assets)
- ape-recomp/ - Recompilacion PSXRecomp (referencia)
- mmorpg/ - Proyecto MMORPG
  - client/ - Cliente basado en decomp + networking
  - server/ - Servidor autoritativo
  - protocol/ - Definicion de protocolo de red
  - shared/ - Codigo compartido (structs, serializacion)
  - docs/ - Documentacion tecnica

## Fases

### Fase 0: Fundacion (COMPLETADA)
- Decompilacion disponible (decomp/)
- Ghidra configurado (ghidra_9.2_PUBLIC)
- AEMT para extraccion de assets
- Native port funcional (areas 01, 02, 05, 44 verificadas byte-a-byte)

### Fase 1: Analisis de arquitectura (SEMANA 1-2)
- [ ] Mapear todos los subsistemas del decomp
- [ ] Identificar puntos de inyeccion para multiplayer
- [ ] Definir protocolo de red (mensajes, serializacion)
- [ ] Disenar modelo de datos del servidor
- [ ] Crear estructura de directorios del MMORPG

### Fase 2: Networking basico (SEMANA 3-4)
- [ ] Implementar capa de red en cliente (TCP/UDP)
- [ ] Implementar servidor basico (aceptar conexiones, lobby)
- [ ] Sincronizacion de posicion de jugadores (UDP)
- [ ] Renderizar otros jugadores en el cliente (ghosts)
- [ ] Chat basico (TCP)

### Fase 3: Estado compartido (SEMANA 5-8)
- [ ] Servidor gestiona estado de monos (captura, respawn)
- [ ] Servidor gestiona enemigos (AI, danio, muerte)
- [ ] Sincronizacion de items/collectibles
- [ ] Sistema de areas/instancias
- [ ] Reconciliacion cliente-servidor

### Fase 4: Persistencia (SEMANA 9-10)
- [ ] Base de datos de jugadores (cuentas, inventario, progreso)
- [ ] Sistema de autenticacion
- [ ] Guardado/carga de estado
- [ ] Sistema de personajes

### Fase 5: Funciones MMO (SEMANA 11-16)
- [ ] Sistema de guilds/clanes
- [ ] Sistema de trading
- [ ] Sistema de quest/missions
- [ ] Sistema de ranking/leaderboards
- [ ] Sistema de combate PvP (si aplica)
- [ ] Sistema de economia (monedas, items)

### Fase 6: Escalado (SEMANA 17+)
- [ ] Optimizacion de red (compresion, delta encoding)
- [ ] Sharding de areas
- [ ] Load balancing
- [ ] Anti-cheat basico
- [ ] Testing de carga

## Decisiones de diseno

### Autoridad del servidor
El servidor es autoritativo sobre todo estado compartido. El cliente predice localmente (movimiento del jugador) y reconcilia con el servidor. Esto previene cheating.

### Protocolo de red
- TCP para eventos criticos (captura de mono, item pickup, chat, auth)
- UDP para posicion/rotacion (baja latencia, tolerante a perdida)
- Mensajes serializados en formato binario compacto

### Modelo de areas
Las areas del juego original (0x01, 0x02, etc.) se convierten en "zonas" del MMO. Cada zona puede tener multiples instancias si hay muchos jugadores. Los monos y enemigos se gestionan por zona.

### Jugadores en el mundo
Cada jugador tiene su propia instancia de Player en la RAM del cliente. Los otros jugadores se renderizan como entidades de red con posicion/rotacion sincronizada. Su logica de juego (animaciones, colisiones locales) se ejecuta en el cliente para fluidez.

### Monos compartidos
Los monos son entidades del servidor. Cuando un jugador captura un mono, el servidor lo marca como capturado para todos. El mono deja de existir en el mundo para todos los jugadores.

### Enemigos compartidos
Los enemigos son gestionados por el servidor. Sus posiciones y estados se sincronizan con todos los clientes. El danio se calcula en el servidor.

## Riesgos y limitaciones

1. El codigo original es single-player. Adaptarlo a multiplayer requiere refactor significativo.
2. La RAM de PSX (2MB) es muy limitada. El cliente nativo no tiene esta limitacion, pero el codigo asume direcciones fijas.
3. La sincronizacion de estado complejo (animaciones, efectos) puede ser costosa.
4. El proyecto de decompilacion esta en pausa hasta 2026-10-23.
5. No hay acceso al ISO del juego (copyright). Se usa el codigo decompilado + assets del usuario.

## Tecnologias propuestas

- Cliente: C (decomp) + SDL3 + OpenGL + enet o custom UDP/TCP
- Servidor: C o Rust (alto rendimiento) o Python (prototipado rapido)
- Base de datos: SQLite (prototipo) o PostgreSQL (produccion)
- Serializacion: Protocol Buffers o MessagePack o custom binario
- Build: CMake + uv (Python tools)

## Proximo paso inmediato

Explorar el codigo de player y monkeys para identificar:
1. Funcion principal de update del jugador (PlayerUpdate)
2. Funcion principal de update de monos (MonkeyUpdateAll)
3. Funcion de captura de mono
4. Estructura de datos de jugador y mono
5. Puntos de inyeccion para networking
