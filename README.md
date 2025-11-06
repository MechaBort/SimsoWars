# Vector Onslaught (Proyecto Allegro)

Vector Onslaught es un shooter espacial estilo *Asteroids* desarrollado con Allegro 5. El proyecto combina un menú animado, un bucle de juego basado en oleadas y un sistema de puntuaciones persistente que incentiva la rejugabilidad. Este README describe en detalle cada subsistema del juego para facilitar su comprensión y mantenimiento.

## Requisitos y preparación del entorno

1. **Dependencias de Allegro 5**: el proyecto utiliza los módulos de fuentes (`allegro_font`, `allegro_ttf`), imágenes (`allegro_image`), primitivas (`allegro_primitives`), audio (`allegro_audio`, `allegro_acodec`) y diálogos nativos (`allegro_native_dialog`).
2. **Recursos incluidos**: fuentes TrueType (`MONSTER.ttf`), bitmaps para los fondos (`Imagenes/menu.png`, `Imagenes/gameplay.png`), así como música y efectos ubicados en la carpeta `Musica/`.
3. **Compilación en Visual Studio**: abra `Proyecto Allegro.sln`, asegúrese de que Allegro esté correctamente vinculado en el sistema, y compile el objetivo `Proyecto Allegro`.

## Arquitectura general

El código se divide en tres archivos principales:

| Archivo | Responsabilidad principal |
| --- | --- |
| `Proyecto Allegro.cpp` | Punto de entrada, inicialización de Allegro, menú principal y navegación entre pantallas. |
| `juego.h` | Bucle de gameplay, control de estados de partida y renderizado de entidades. |
| `Funciones.h` | Estructuras de datos, lógica de enemigos/balas, utilidades de audio y persistencia de estadísticas. |

Además, `estadisticas.txt` almacena el historial de partidas y se actualiza al finalizar cada sesión.

## Flujo de arranque y menú principal

`main()` configura los módulos de Allegro, crea la ventana a pantalla completa usando la resolución del monitor y carga fuentes, fondos y audio.【F:Proyecto Allegro/Proyecto Allegro.cpp†L78-L157】 Una vez inicializado todo, se muestran tres opciones en el menú principal: **Jugar**, **Ver High Scores** y **Salir**, con navegación mediante `W/S` o las flechas y selección con `Enter`. El menú se renderiza en `renderizarMenu()`, que pinta el fondo, el título, las opciones resaltadas y las instrucciones.【F:Proyecto Allegro/Proyecto Allegro.cpp†L38-L74】 La pantalla de puntuaciones (`renderizarPantallaHighScores()`) consulta el top 5 persistido y lo muestra con colores distintivos para el podio.【F:Proyecto Allegro/Proyecto Allegro.cpp†L76-L126】

## Gestión de estados de la aplicación

El bucle principal mantiene un estado global (`APP_MENU`, `APP_JUGANDO`, `APP_HIGH_SCORES`) para decidir qué pantalla actualizar y dibujar.【F:Proyecto Allegro/Proyecto Allegro.cpp†L28-L135】 Cuando el jugador elige **Jugar**, `iniciarJuego()` toma el control y el menú pausa su música hasta que el gameplay termina. Elegir **Ver High Scores** alterna a la vista de clasificaciones hasta que se presione `Esc`.

## Bucle de juego y estados de partida

`iniciarJuego()` encapsula el bucle del gameplay y trabaja sobre un conjunto de estados (`JUGANDO`, `CAMBIO_RONDA`, `GAME_OVER`, `INPUT_NOMBRE`).【F:Proyecto Allegro/juego.h†L21-L191】 Cada ciclo procesa entradas del teclado, actualiza física y colisiones al ritmo del temporizador de 60 FPS y renderiza la escena completa antes de hacer `al_flip_display()`.

### Controles

- `W`: acelera la nave en la dirección actual.
- `A/D`: giran la nave en sentido antihorario/horario.
- `SPACE`: dispara proyectiles.
- `ESC`: abandona la partida y retorna al menú.

Los controles modifican banderas que afectan la física dentro del evento de temporizador, para garantizar que la actualización ocurra de forma consistente con la tasa de refresco.【F:Proyecto Allegro/juego.h†L59-L128】

### Física del jugador

La nave del jugador se modela con una estructura `Nave` que contiene posición, velocidad, ángulo y radio de colisión.【F:Proyecto Allegro/Funciones.h†L28-L70】 El movimiento incorpora aceleración basada en seno/coseno del ángulo, un factor de rozamiento para simular inercia y un límite de velocidad máxima. Además se restringe a los bordes jugables para evitar que salga de pantalla.【F:Proyecto Allegro/juego.h†L137-L186】 El renderizado utiliza transformaciones para dibujar un rombo orientado en tiempo real.【F:Proyecto Allegro/juego.h†L188-L221】

### Sistema de disparo y balas

Las balas se almacenan en una lista enlazada (`PtrBala`) y cada disparo crea un proyectil con velocidad dirigida hacia adelante y un tiempo de vida limitado (`VIDA_BALA`).【F:Proyecto Allegro/Funciones.h†L92-L181】 `actualizarBalas()` avanza cada proyectil y los marca como inactivos al expirar; `limpiarBalas()` recicla la memoria de los que ya no se usan. Cada disparo respeta una cadencia (`CADENCIA_DISPARO`) para evitar ráfagas infinitas.【F:Proyecto Allegro/juego.h†L104-L122】

### Enemigos y oleadas

Los enemigos también viven en una lista enlazada (`PtrNave`) y se generan en oleadas crecientes. `generarOleada()` calcula el tamaño de la ronda y crea un 60% de drones erráticos y un 40% de seekers rastreadores.【F:Proyecto Allegro/Funciones.h†L183-L318】

- **Drones (tipo 1)**: rebotan dentro del área de juego cambiando velocidad al tocar los bordes.【F:Proyecto Allegro/Funciones.h†L80-L139】
- **Seekers (tipo 2)**: avanzan hacia el jugador usando vectores normalizados para perseguirlo.【F:Proyecto Allegro/Funciones.h†L140-L181】

`actualizarEnemigos()` delega en el movimiento apropiado y `limpiarEnemigosInactivos()` elimina los que fueron destruidos. Cuando el conteo de enemigos activos llega a cero, el estado cambia a `CAMBIO_RONDA`, se resetea la nave, se limpia la lista de balas y se programa la siguiente oleada tras un breve temporizador.【F:Proyecto Allegro/juego.h†L122-L170】

### Colisiones y puntuación

`verificarColisionesBalasEnemigos()` compara cada bala con cada enemigo usando detección de círculos; cada baja otorga 100 puntos y reproduce un efecto de explosión.【F:Proyecto Allegro/Funciones.h†L200-L278】【F:Proyecto Allegro/juego.h†L115-L134】 Si el jugador colisiona con un enemigo, se reproduce un sonido de muerte y tras un retardo de 2 segundos el estado pasa a `GAME_OVER`, activando la música correspondiente.【F:Proyecto Allegro/juego.h†L134-L153】 El HUD muestra puntuación, ronda y tiempo en todo momento.【F:Proyecto Allegro/juego.h†L229-L244】

### Transiciones, Game Over e ingreso de nombre

Durante `CAMBIO_RONDA`, se muestra un mensaje con efecto de aparición/desvanecimiento mientras corre el temporizador de transición.【F:Proyecto Allegro/juego.h†L246-L264】 En `GAME_OVER`, la pantalla lista las estadísticas de la partida y pide confirmar con `Enter`. Posteriormente, `INPUT_NOMBRE` permite ingresar un alias de hasta 15 caracteres (letras, números y espacios) con cursor parpadeante y retroceso. También se despliega el Top 5 actual para motivar la competencia.【F:Proyecto Allegro/juego.h†L266-L336】

## Persistencia de estadísticas

Al confirmar el nombre, `guardarEstadisticas()` agrega una línea al archivo `estadisticas.txt` con nombre, puntos, tiempo, ronda, enemigos eliminados y proyectiles disparados.【F:Proyecto Allegro/Funciones.h†L320-L383】 `leerEstadisticas()` parsea el archivo, convierte cada campo y ordena los registros por puntuación descendente. `obtenerTop5()` recorta los cinco mejores para mostrarlos tanto en el menú de puntuaciones como en la pantalla de ingreso de nombre.【F:Proyecto Allegro/Funciones.h†L385-L417】

## Audio

El módulo de audio mantiene punteros globales a las pistas de menú, juego y game over, así como a los efectos de disparo, explosión y muerte. `cargarAudio()` y `limpiarAudio()` manejan la vida útil de estos recursos, mientras que `tocarMusica()` garantiza reproducción en bucle con un único canal activo a la vez y `tocarSonido()` permite superponer efectos.【F:Proyecto Allegro/Funciones.h†L419-L476】 La música cambia automáticamente al entrar en gameplay o Game Over, y se reactiva la pista del menú al regresar a la pantalla principal.【F:Proyecto Allegro/Proyecto Allegro.cpp†L132-L184】

## Recursos y arte

- **Fondos**: bitmaps escalados a pantalla completa tanto en el menú como durante el gameplay.
- **Tipografía**: `MONSTER.ttf` se carga en tres tamaños para título, opciones y mensajes secundarios.
- **HUD y figuras**: el jugador se representa con un rombo azul, los drones con círculos verdes y los seekers con triángulos rosas orientados hacia la nave del jugador.

## Controles rápidos y atajos

| Acción | Tecla |
| --- | --- |
| Navegar menú | `W/S` o Flechas ↑/↓ |
| Confirmar opción | `Enter` |
| Volver atrás / Salir | `Esc` |
| Acelerar nave | `W` |
| Girar nave | `A` / `D` |
| Disparar | `Space` |
| Borrar carácter (nombre) | `Backspace` |

## Limpieza y cierre

El cierre del juego destruye fuentes, bitmaps, colas de eventos, temporizador y display, además de liberar enemigos/balas y descargar los recursos de audio para evitar fugas de memoria.【F:Proyecto Allegro/Proyecto Allegro.cpp†L186-L209】【F:Proyecto Allegro/juego.h†L338-L344】

## Créditos y futuras mejoras

El proyecto puede ampliarse con nuevos tipos de enemigos, power-ups, soporte para gamepads o integración de un sistema de logros. Esta documentación proporciona la base para abordar dichas mejoras de forma estructurada.
