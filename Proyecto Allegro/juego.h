/*
 * JUEGO.H
 * -------
 * Logica principal del gameplay (controles, fisica, colisiones, renderizado)
 */

#pragma once // Previene inclusiones multiples de esta cabecera

#include <allegro5/allegro.h> // Tipos basicos y funciones generales de Allegro
#include <allegro5/allegro_font.h> // Soporte para dibujar texto en pantalla
#include <allegro5/allegro_primitives.h> // Permite dibujar primitivas geometricas
#include <cmath> // Utiliza funciones matematicas como seno, coseno y raiz cuadrada
#include "Funciones.h" // Acceso a estructuras, constantes y utilidades compartidas

using namespace std; // Evita el uso de std:: en cada referencia a tipos estandar

// ========== CONSTANTES DE FISICA ==========

const float FPS = 60.0f; // Frecuencia objetivo en fotogramas por segundo
const float ROTACION = 0.07f; // Variacion angular aplicada por frame al rotar la nave
const float ACELERACION = 0.35f; // Magnitud de aceleracion aplicada al impulsar la nave
const float ROZAMIENTO = 0.985f; // Factor de amortiguamiento aplicado cada frame para frenar
const float VELOCIDAD_MAX = 9.0f; // Velocidad maxima permitida para el jugador

// ========== GEOMETRIA ==========

static float Puntos_jugador[] = {
        0.0f, -30.0f, // Vertice superior de la nave del jugador
        -30.0f, 30.0f, // Vertice inferior izquierdo del rombo
        0.0f, 10.0f, // Vertice central inferior que crea la forma de rombo
        30.0f, 30.0f // Vertice inferior derecho del rombo
};

static float v[] = {
        0.0f, -45.0f, // Punta superior del triangulo del seeker
        -35.0f, 27.0f, // Vertice inferior izquierdo del triangulo
        35.0f, 27.0f // Vertice inferior derecho del triangulo
};

// ========== FUNCION PRINCIPAL DEL JUEGO ==========

void iniciarJuego(int ancho, int alto, ALLEGRO_FONT* font, ALLEGRO_TIMER* timer, ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_BITMAP* fondo_gameplay) {
        tocarMusica(musica_gameplay, 0.05f); // Inicia la musica de fondo del gameplay con volumen bajo

        EstadoJuego estado = JUGANDO; // Estado inicial de la partida
        float timer_trans = 0.0f; // Tiempo restante de la pantalla de transicion entre rondas
        Nave player; // Instancia que representa al jugador
        PtrNave enemigos = NULL; // Lista enlazada de enemigos activos en la partida
        PtrBala balas = NULL; // Lista enlazada de proyectiles disparados
        float cooldown = 0.0f; // Temporizador entre disparos consecutivos
        int ronda = 1; // Numero de ronda actual
        int puntos = 0; // Puntuacion acumulada durante la partida
        int kills = 0; // Conteo de enemigos eliminados
        int proyectiles = 0; // Numero de proyectiles disparados por el jugador
        float tiempo = 0.0f; // Tiempo transcurrido mientras el estado es JUGANDO
        float tiempo_total = 0.0f; // Tiempo total transcurrido incluyendo pantallas auxiliares
        float delay_muerte = 0.0f; // Temporizador entre la muerte y la pantalla de game over
        string nombre = ""; // Buffer de texto para el nombre del jugador
        bool W = false, D = false, A = false, SPACE = false; // Estados de las teclas principales del control

        iniciarPersonaje(player, ancho, alto); // Coloca al jugador en el centro de la pantalla y reinicia sus atributos
        generarOleada(enemigos, ronda, ancho, alto); // Crea la primera oleada de enemigos de acuerdo a la ronda inicial

        bool jugando = true; // Controla la permanencia en el bucle principal del gameplay
        while (jugando) { // Bucle que se mantiene hasta que se abandona el gameplay
                ALLEGRO_EVENT ev; // Almacena el evento recibido desde la cola
                al_wait_for_event(queue, &ev); // Espera de manera bloqueante un nuevo evento

                if (ev.type == ALLEGRO_EVENT_KEY_DOWN) { // Gestiona pulsaciones de teclado
                        if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) { // Escape durante el gameplay
                                jugando = false; // Rompe el bucle y retorna al menu
                        }

                        if (ev.keyboard.keycode == ALLEGRO_KEY_W && estado == JUGANDO) W = true; // Registra que W esta presionada para acelerar
                        if (ev.keyboard.keycode == ALLEGRO_KEY_D && estado == JUGANDO) D = true; // Registra que D esta presionada para girar a la derecha
                        if (ev.keyboard.keycode == ALLEGRO_KEY_A && estado == JUGANDO) A = true; // Registra que A esta presionada para girar a la izquierda
                        if (ev.keyboard.keycode == ALLEGRO_KEY_SPACE && estado == JUGANDO) SPACE = true; // Registra que Space esta presionada para disparar

                        if (ev.keyboard.keycode == ALLEGRO_KEY_ENTER) { // Gestiona la tecla Enter
                                if (estado == GAME_OVER) { // Si se encuentra en la pantalla de game over
                                        estado = INPUT_NOMBRE; // Avanza al estado de captura de nombre
                                        nombre = ""; // Limpia cualquier nombre previo
                                } else if (estado == INPUT_NOMBRE) { // Si ya se esta capturando el nombre
                                        if (nombre.empty()) nombre = "ANONIMO"; // Usa un nombre generico si el jugador no escribio nada

                                        Estadistica s; // Estructura para guardar los datos finales
                                        s.nombre = nombre; // Asigna el nombre capturado
                                        s.puntuacion = puntos; // Registra la puntuacion final
                                        s.tiempo = tiempo; // Guarda el tiempo activo de juego
                                        s.ronda = ronda; // Guarda la ronda alcanzada
                                        s.enemigos_eliminados = kills; // Registra la cantidad de enemigos eliminados
                                        s.proyectiles_disparados = proyectiles; // Guarda los proyectiles disparados
                                        guardarEstadisticas(s); // Persiste la informacion en archivo

                                        jugando = false; // Finaliza el gameplay y regresa al menu
                                }
                        }

                        if (ev.keyboard.keycode == ALLEGRO_KEY_BACKSPACE && estado == INPUT_NOMBRE && !nombre.empty()) {
                                nombre.pop_back(); // Elimina el ultimo caracter del nombre ingresado
                        }

                        if (estado == INPUT_NOMBRE) { // Durante la captura de nombre se procesan letras y numeros
                                int key = ev.keyboard.keycode; // Codigo de la tecla presionada

                                if (key >= ALLEGRO_KEY_A && key <= ALLEGRO_KEY_Z && nombre.length() < 15) {
                                        char letra = 'A' + (key - ALLEGRO_KEY_A); // Convierte el codigo de tecla a una letra mayuscula
                                        nombre += letra; // Agrega la letra al nombre actual
                                }

                                if (key >= ALLEGRO_KEY_0 && key <= ALLEGRO_KEY_9 && nombre.length() < 15) {
                                        char num = '0' + (key - ALLEGRO_KEY_0); // Convierte el codigo de tecla a digito numerico
                                        nombre += num; // Agrega el numero al nombre
                                }

                                if (key == ALLEGRO_KEY_SPACE && nombre.length() < 15) {
                                        nombre += ' '; // Inserta un espacio entre palabras
                                }
                        }
                }

                if (ev.type == ALLEGRO_EVENT_KEY_UP) { // Gestiona la liberacion de teclas
                        if (ev.keyboard.keycode == ALLEGRO_KEY_W) W = false; // Libera la aceleracion
                        if (ev.keyboard.keycode == ALLEGRO_KEY_D) D = false; // Libera el giro a la derecha
                        if (ev.keyboard.keycode == ALLEGRO_KEY_A) A = false; // Libera el giro a la izquierda
                        if (ev.keyboard.keycode == ALLEGRO_KEY_SPACE) SPACE = false; // Libera el disparo continuo
                }

                if (ev.type == ALLEGRO_EVENT_TIMER && ev.timer.source == timer) { // Actualizaciones sincronizadas con el temporizador
                        tiempo_total += 1.0f / FPS; // Incrementa el tiempo total cada frame

                        if (estado == JUGANDO) { // Solo actualiza la logica principal cuando se esta jugando
                                tiempo += 1.0f / FPS; // Incrementa el cronometro de juego activo

                                if (cooldown > 0.0f) cooldown -= 1.0f; // Reduce el tiempo restante para permitir otro disparo
                                if (SPACE && cooldown <= 0.0f && player.activo) { // Comprueba si se puede disparar
                                        dispararBala(balas, player); // Crea una nueva bala hacia la direccion actual
                                        cooldown = CADENCIA_DISPARO; // Reinicia el temporizador de disparo
                                        proyectiles++; // Incrementa el conteo de proyectiles lanzados
                                        if (sfx_disparo) { // Si existe un sample de disparo cargado
                                                al_play_sample(sfx_disparo, 0.3, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL); // Reproduce el efecto de disparo
                                        }
                                }

                                if (player.activo) {
                                        actualizarEnemigos(enemigos, player, ancho, alto); // Actualiza el movimiento de todos los enemigos
                                        actualizarBalas(balas); // Avanza la posicion de todas las balas activas
                                }

                                int muertos = verificarColisionesBalasEnemigos(balas, enemigos); // Detecta impactos de balas contra enemigos
                                if (muertos > 0) { // Si algun enemigo fue destruido
                                        kills += muertos; // Incrementa el total de eliminaciones
                                        puntos += muertos * 100; // Suma puntos por cada enemigo destruido
                                        if (sfx_explosion) al_play_sample(sfx_explosion, 0.5, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL); // Reproduce el efecto de explosion
                                }

                                limpiarBalas(balas); // Elimina balas que se desactivaron
                                limpiarEnemigosInactivos(enemigos); // Remueve enemigos destruidos de la lista

                                if (contarEnemigosActivos(enemigos) == 0 && player.activo) { // Comprueba si la ronda fue completada
                                        estado = CAMBIO_RONDA; // Cambia al estado de transicion
                                        timer_trans = DURACION_TRANSICION; // Establece la duracion de la pantalla intermedia
                                        ronda++; // Incrementa el numero de ronda alcanzado
                                        liberarBalas(balas); // Limpia cualquier bala restante
                                        balas = NULL; // Reinicia el puntero de balas
                                        resetearJugador(player, ancho, alto); // Regresa al jugador al centro y reinicia su movimiento
                                }

                                if (verificarColisionJugadorEnemigos(player, enemigos)) { // Comprueba si el jugador colisiona con un enemigo
                                        player.activo = false; // Desactiva al jugador para detener la logica de movimiento
                                        delay_muerte = 120.0f; // Establece un retraso antes del game over
                                        if (sfx_muerte) al_play_sample(sfx_muerte, 0.7, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL); // Reproduce el efecto de muerte del jugador
                                }

                                if (!player.activo && delay_muerte > 0.0f) { // Mientras espera antes de mostrar el game over
                                        delay_muerte -= 1.0f; // Reduce el temporizador de retraso
                                        if (delay_muerte <= 0.0f) { // Cuando termina el retraso
                                                estado = GAME_OVER; // Cambia al estado de game over
                                                tocarMusica(musica_gameover, 0.6f); // Reproduce la musica de game over
                                        }
                                }

                                if (player.activo) { // Actualiza la fisica de la nave solo si sigue viva
                                        if (A) player.ang -= ROTACION; // Gira hacia la izquierda cuando A esta activa
                                        if (D) player.ang += ROTACION; // Gira hacia la derecha cuando D esta activa

                                        if (W) { // Aplica impulso hacia adelante cuando se presiona W
                                                float fx = sin(player.ang); // Componente horizontal del impulso segun el angulo actual
                                                float fy = -cos(player.ang); // Componente vertical del impulso
                                                player.vx += fx * ACELERACION; // Ajusta la velocidad horizontal del jugador
                                                player.vy += fy * ACELERACION; // Ajusta la velocidad vertical del jugador
                                        }

                                        player.vx *= ROZAMIENTO; // Aplica amortiguamiento a la velocidad horizontal
                                        player.vy *= ROZAMIENTO; // Aplica amortiguamiento a la velocidad vertical

                                        float vel = player.vx * player.vx + player.vy * player.vy; // Calcula la magnitud al cuadrado de la velocidad
                                        if (vel > VELOCIDAD_MAX * VELOCIDAD_MAX) { // Comprueba si supera el limite permitido
                                                float factor = VELOCIDAD_MAX / sqrt(vel); // Calcula el factor de reduccion necesario
                                                player.vx *= factor; // Escala la velocidad horizontal para respetar el limite
                                                player.vy *= factor; // Escala la velocidad vertical
                                        }

                                        player.x += player.vx; // Actualiza la posicion horizontal del jugador
                                        player.y += player.vy; // Actualiza la posicion vertical del jugador

                                        if (player.x < 25) player.x = 25; // Evita que la nave salga por el borde izquierdo
                                        if (player.x >= ancho - 25) player.x = ancho - 25; // Evita que la nave salga por el borde derecho
                                        if (player.y < 25) player.y = 25; // Evita que la nave salga por la parte superior
                                        if (player.y >= alto - 25) player.y = alto - 25; // Evita que la nave salga por la parte inferior
                                }
                        }

                        if (estado == CAMBIO_RONDA) { // Actualiza la pantalla de transicion entre rondas
                                timer_trans -= 1.0f; // Reduce el temporizador de la pantalla intermedia
                                if (timer_trans <= 0.0f) { // Una vez finalizado el temporizador
                                        generarOleada(enemigos, ronda, ancho, alto); // Genera la siguiente oleada de enemigos
                                        estado = JUGANDO; // Regresa al estado de juego activo
                                }
                        }

                        al_clear_to_color(al_map_rgb(0, 0, 0)); // Limpia la pantalla antes de dibujar el nuevo frame

                        if (estado == JUGANDO) {
                                if (fondo_gameplay) {
                                        al_draw_scaled_bitmap(fondo_gameplay, 0, 0, al_get_bitmap_width(fondo_gameplay), al_get_bitmap_height(fondo_gameplay), 0, 0, ancho, alto, 0); // Dibuja el fondo del gameplay ajustado a la pantalla
                                }

                                if (player.activo) {
                                        ALLEGRO_TRANSFORM guardado, t; // Transformaciones para posicionar la nave
                                        al_copy_transform(&guardado, al_get_current_transform()); // Guarda la transformacion actual
                                        al_identity_transform(&t); // Inicializa una transformacion identidad
                                        al_rotate_transform(&t, player.ang); // Aplica la rotacion de la nave
                                        al_translate_transform(&t, player.x, player.y); // Traslada la transformacion a la posicion del jugador
                                        al_use_transform(&t); // Activa la transformacion combinada

                                        al_draw_filled_polygon(Puntos_jugador, 4, al_map_rgb(60, 180, 255)); // Dibuja el cuerpo de la nave del jugador
                                        al_draw_polygon(Puntos_jugador, 4, ALLEGRO_LINE_JOIN_ROUND, al_map_rgb(255, 255, 255), 1.5f, 1.0f); // Dibuja el contorno de la nave

                                        al_use_transform(&guardado); // Restaura la transformacion previa para no afectar dibujos posteriores
                                }

                                PtrNave e = enemigos; // Inicia el recorrido para dibujar cada enemigo
                                while (e != NULL) {
                                        if (e->activo) {
                                                if (e->tipo == 1) {
                                                        al_draw_circle(e->x, e->y, 50.0f, al_map_rgb(170, 255, 170), 2); // Dibuja el contorno exterior del drone
                                                        al_draw_circle(e->x, e->y, 45.0f, al_map_rgb(170, 255, 170), 3); // Dibuja un segundo circulo para efecto visual
                                                } else if (e->tipo == 2) {
                                                        ALLEGRO_TRANSFORM old, Ts; // Transformaciones para orientar el seeker
                                                        al_copy_transform(&old, al_get_current_transform()); // Guarda la transformacion actual
                                                        al_identity_transform(&Ts); // Reinicia una transformacion identidad

                                                        float ang = atan2f(player.y - e->y, player.x - e->x) + 3.14159f / 2.0f; // Calcula el angulo hacia el jugador
                                                        al_rotate_transform(&Ts, ang); // Rota el triangulo del seeker para que apunte al jugador
                                                        al_translate_transform(&Ts, e->x, e->y); // Posiciona el triangulo en la ubicacion del enemigo
                                                        al_use_transform(&Ts); // Aplica la transformacion temporal

                                                        al_draw_triangle(v[0], v[1], v[2], v[3], v[4], v[5], al_map_rgb(255, 100, 220), 6); // Dibuja el contorno grueso del seeker
                                                        al_draw_triangle(v[0], v[1], v[2], v[3], v[4], v[5], al_map_rgb(255, 255, 255), 3); // Dibuja un contorno adicional blanco
                                                        al_draw_triangle(v[0], v[1] + 20, v[2] + 15, v[3] - 10, v[4] - 15, v[5] - 10, al_map_rgb(255, 100, 220), 3); // Dibuja una franja interior
                                                        al_draw_triangle(v[0], v[1] + 20, v[2] + 15, v[3] - 10, v[4] - 15, v[5] - 10, al_map_rgb(255, 255, 255), 1); // Dibuja el borde de la franja interior

                                                        al_use_transform(&old); // Restaura la transformacion previa
                                                }
                                        }
                                        e = e->siguiente; // Avanza al siguiente enemigo en la lista
                                }

                                PtrBala b = balas; // Recorre la lista de balas activas
                                while (b != NULL) {
                                        if (b->activa) {
                                                al_draw_filled_circle(b->x, b->y, 5.0f, al_map_rgb(255, 255, 0)); // Dibuja la bala como un circulo amarillo
                                        }
                                        b = b->siguiente; // Continua con la siguiente bala
                                }

                                al_draw_textf(font, al_map_rgb(255, 255, 255), 10, 10, ALLEGRO_ALIGN_LEFT, "PUNTUACION: %d", puntos); // Muestra la puntuacion actual
                                al_draw_textf(font, al_map_rgb(255, 255, 255), 10, 35, ALLEGRO_ALIGN_LEFT, "RONDA: %d", ronda); // Muestra la ronda activa
                                al_draw_textf(font, al_map_rgb(255, 255, 255), 10, 60, ALLEGRO_ALIGN_LEFT, "TIEMPO: %.1f", tiempo); // Muestra el tiempo de juego
                        }

                        if (estado == CAMBIO_RONDA) {
                                float progreso = 1.0f - (timer_trans / DURACION_TRANSICION); // Calcula el avance de la transicion respecto al tiempo total
                                float fade = (progreso < 0.3f) ? (progreso / 0.3f) : ((progreso > 0.7f) ? ((1.0f - progreso) / 0.3f) : 1.0f); // Determina la intensidad del texto para efecto de fade

                                char txt[50]; // Buffer temporal para el mensaje de ronda
                                sprintf_s(txt, 50, "RONDA %d", ronda); // Formatea el numero de ronda

                                al_draw_text(font, al_map_rgba_f(fade, fade, 0, fade), ancho / 2, alto / 2 - 50, ALLEGRO_ALIGN_CENTER, txt); // Dibuja el mensaje principal con transparencia variable
                                al_draw_text(font, al_map_rgba_f(0.8f * fade, 0.8f * fade, 0.8f * fade, fade), ancho / 2, alto / 2, ALLEGRO_ALIGN_CENTER, "Preparate..."); // Dibuja un mensaje secundario
                        }

                        if (estado == GAME_OVER) {
                                al_draw_text(font, al_map_rgb(255, 0, 0), ancho / 2, alto / 2 - 200, ALLEGRO_ALIGN_CENTER, "GAME OVER"); // Encabezado de la pantalla de derrota
                                al_draw_textf(font, al_map_rgb(255, 255, 255), ancho / 2, alto / 2 - 140, ALLEGRO_ALIGN_CENTER, "Puntuacion Final: %d", puntos); // Muestra la puntuacion final
                                al_draw_textf(font, al_map_rgb(255, 255, 255), ancho / 2, alto / 2 - 110, ALLEGRO_ALIGN_CENTER, "Tiempo: %.1f segundos", tiempo); // Muestra el tiempo de juego
                                al_draw_textf(font, al_map_rgb(255, 255, 255), ancho / 2, alto / 2 - 80, ALLEGRO_ALIGN_CENTER, "Enemigos Eliminados: %d", kills); // Muestra las bajas totales
                                al_draw_textf(font, al_map_rgb(255, 255, 255), ancho / 2, alto / 2 - 50, ALLEGRO_ALIGN_CENTER, "Ronda Alcanzada: %d", ronda); // Muestra la ronda alcanzada
                                al_draw_text(font, al_map_rgb(150, 150, 150), ancho / 2, alto / 2, ALLEGRO_ALIGN_CENTER, "Presiona ENTER para continuar..."); // Instruccion para avanzar a la captura de nombre
                        }

                        if (estado == INPUT_NOMBRE) {
                                al_draw_text(font, al_map_rgb(255, 255, 0), ancho / 2, alto / 2 - 250, ALLEGRO_ALIGN_CENTER, "NUEVA PUNTUACION!"); // Mensaje de felicitacion por entrar al ranking
                                al_draw_textf(font, al_map_rgb(255, 255, 255), ancho / 2, alto / 2 - 200, ALLEGRO_ALIGN_CENTER, "Puntuacion: %d", puntos); // Muestra la puntuacion alcanzada
                                al_draw_text(font, al_map_rgb(200, 200, 200), ancho / 2, alto / 2 - 140, ALLEGRO_ALIGN_CENTER, "Ingresa tu nombre:"); // Indica que se debe ingresar un nombre

                                bool mostrar = ((int)(tiempo_total * 2.0f)) % 2 == 0; // Determina si el cursor debe mostrarse parpadeando
                                string txt = nombre; // Copia el nombre actual para visualizacion
                                if (mostrar) txt += "_"; // Agrega un cursor visible cuando corresponde
                                if (txt.empty()) txt = "_"; // Garantiza que al menos se muestre el cursor

                                al_draw_text(font, al_map_rgb(255, 255, 255), ancho / 2, alto / 2 - 90, ALLEGRO_ALIGN_CENTER, txt.c_str()); // Dibuja el texto ingresado con el cursor
                                al_draw_text(font, al_map_rgb(150, 150, 150), ancho / 2, alto / 2 - 40, ALLEGRO_ALIGN_CENTER, "Presiona ENTER para confirmar"); // Indica como finalizar la captura
                                al_draw_text(font, al_map_rgb(150, 150, 150), ancho / 2, alto / 2 - 10, ALLEGRO_ALIGN_CENTER, "Presiona BACKSPACE para borrar"); // Recordatorio de como borrar caracteres

                                al_draw_text(font, al_map_rgb(255, 255, 0), ancho / 2, alto / 2 + 40, ALLEGRO_ALIGN_CENTER, "=== TOP 5 ==="); // Encabezado de la tabla de mejores puntuaciones

                                vector<Estadistica> top5 = obtenerTop5(); // Obtiene las cinco mejores estadisticas registradas
                                int y = 75; // Posicion vertical inicial para listar el top 5
                                for (size_t i = 0; i < top5.size(); i++) { // Recorre cada entrada del ranking
                                        char linea[100]; // Buffer temporal para formatear la linea
                                        sprintf_s(linea, 100, "%d. %s - %d pts (Ronda %d)", (int)(i + 1), top5[i].nombre.c_str(), top5[i].puntuacion, top5[i].ronda); // Formatea la informacion clave

                                        ALLEGRO_COLOR col; // Color a aplicar segun la posicion en el ranking
                                        if (i == 0) col = al_map_rgb(255, 215, 0); // Oro para el primer lugar
                                        else if (i == 1) col = al_map_rgb(192, 192, 192); // Plata para el segundo lugar
                                        else if (i == 2) col = al_map_rgb(205, 127, 50); // Bronce para el tercer lugar
                                        else col = al_map_rgb(200, 200, 200); // Gris para el resto de posiciones

                                        al_draw_text(font, col, ancho / 2, alto / 2 + y, ALLEGRO_ALIGN_CENTER, linea); // Dibuja la linea correspondiente del top 5
                                        y += 25; // Ajusta la posicion vertical para la siguiente entrada
                                }
                        }

                        al_flip_display(); // Presenta todo el contenido dibujado en el frame actual
                }

                if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) { // Maneja el cierre de la ventana durante el gameplay
                        jugando = false; // Sale del bucle principal del juego
                }
        }

        liberarEnemigos(enemigos); // Libera la memoria asociada a la lista de enemigos
        liberarBalas(balas); // Libera la memoria de todas las balas restantes
}
