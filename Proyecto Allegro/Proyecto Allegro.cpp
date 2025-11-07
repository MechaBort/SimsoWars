/*
 * =============================================================================
 * PROYECTO ALLEGRO - VECTOR ONSLAUGHT
 * =============================================================================
 * Juego de naves estilo Asteroids con oleadas de enemigos
 *
 * Este archivo contiene:
 * - Inicializacion de Allegro 5
 * - Bucle principal del programa
 * - Menu principal y navegacion
 * - Pantalla de high scores
 * =============================================================================
 */

#include <stdio.h> // Cabecera de la libreria estandar de C para entrada y salida basica
#include <cmath> // Funciones matematicas como seno, coseno y raiz cuadrada
#include <iostream> // Flujo de entrada y salida de C++ para depuracion opcional
#include <cstdlib> // Utilidades generales de la libreria C como conversiones y random
#include <ctime> // Manejo del tiempo para semillas de numeros aleatorios
#include <string> // Clase string de C++ usada para nombres y mensajes

#include <allegro5/allegro.h> // Cabecera central de Allegro 5 para iniciar el motor
#include <allegro5/allegro_font.h> // Soporte para fuentes bitmap basicas
#include <allegro5/allegro_ttf.h> // Soporte para fuentes TrueType
#include <allegro5/allegro_native_dialog.h> // Mostrar cuadros de dialogo nativos del sistema
#include <allegro5/allegro_image.h> // Manejo de imagenes en Allegro
#include <allegro5/allegro_primitives.h> // Dibujo de primitivas geometricas
#include <allegro5/allegro_audio.h> // Sistema de audio de Allegro
#include <allegro5/allegro_acodec.h> // Codecs de audio para Allegro

#include "Funciones.h" // Declaraciones compartidas de estructuras y utilidades del juego
#include "juego.h" // Funciones especificas del gameplay

using namespace std; // Evita escribir std:: de forma repetida en el archivo

// ========== ENUMERACIONES ==========

enum OpcionMenu {
        MENU_JUGAR = 0, // Entrada de menu que inicia la partida
        MENU_HIGH_SCORES = 1, // Entrada que muestra la pantalla de high scores
        MENU_SALIR = 2 // Entrada que finaliza la aplicacion
};

enum EstadoApp {
        APP_MENU, // La aplicacion esta mostrando el menu principal
        APP_JUGANDO, // La aplicacion se encuentra dentro del gameplay
        APP_HIGH_SCORES // La aplicacion muestra el listado de puntuaciones
};

// ========== FUNCIONES DE RENDERIZADO ==========

// Dibuja el menu principal con opciones y fondo
void renderizarMenu(int opcion, ALLEGRO_FONT* fuente_grande, ALLEGRO_FONT* fuente_mediana, ALLEGRO_FONT* fuente_pequena, int ancho, int alto, float timer, ALLEGRO_BITMAP* fondo) {
        al_clear_to_color(al_map_rgb(0, 0, 0)); // Limpia la pantalla con un color negro uniforme

        if (fondo) { // Comprueba si se paso un bitmap de fondo valido
                al_draw_scaled_bitmap(fondo, 0, 0, al_get_bitmap_width(fondo), al_get_bitmap_height(fondo), 0, 0, ancho, alto, 0); // Ajusta el fondo a la resolucion actual
        }

        al_draw_text(fuente_grande, al_map_rgb(150, 150, 150), ancho / 2, alto / 2 - 250, ALLEGRO_ALIGN_CENTER, "VECTOR ONSLAUGHT"); // Titulo principal centrado
        al_draw_text(fuente_mediana, al_map_rgb(150, 150, 150), ancho / 2, alto / 2 - 150, ALLEGRO_ALIGN_CENTER, "Survival Space Shooter"); // Subtitulo descriptivo

        const char* texto[3] = {"JUGAR", "VER HIGH SCORES", "SALIR"}; // Lista de opciones del menu
        int y = alto / 2 - 50; // Coordenada vertical base para colocar las opciones

        for (int i = 0; i < 3; i++) { // Recorre cada opcion disponible
                int yPos = y + (i * 60); // Calcula la posicion vertical desplazada segun el indice
                ALLEGRO_COLOR color = (i == opcion) ? al_map_rgb(255, 255, 0) : al_map_rgb(150, 150, 150); // Destaca la opcion activa en amarillo

                al_draw_text(fuente_mediana, color, ancho / 2, yPos, ALLEGRO_ALIGN_CENTER, texto[i]); // Dibuja el texto de la opcion actual

                if (i == opcion) { // Si la opcion esta seleccionada
                        al_draw_text(fuente_mediana, color, ancho / 2 - 120, yPos, ALLEGRO_ALIGN_CENTER, ">"); // Dibuja una flecha indicadora
                }
        }

        al_draw_text(fuente_pequena, al_map_rgb(100, 100, 100), ancho / 2, alto - 100, ALLEGRO_ALIGN_CENTER, "Usa W/S o Flechas para navegar"); // Muestra instrucciones de navegacion
        al_draw_text(fuente_pequena, al_map_rgb(100, 100, 100), ancho / 2, alto - 70, ALLEGRO_ALIGN_CENTER, "Presiona ENTER para seleccionar"); // Indica como seleccionar una opcion

        al_flip_display(); // Presenta en pantalla el frame renderizado del menu
}

// Muestra el Top 5 de mejores puntuaciones
void renderizarPantallaHighScores(ALLEGRO_FONT* fuente_grande, ALLEGRO_FONT* fuente_mediana, int ancho, int alto) {
        al_clear_to_color(al_map_rgb(0, 0, 0)); // Limpia la pantalla antes de dibujar la tabla
        al_draw_text(fuente_grande, al_map_rgb(255, 215, 0), ancho / 2, alto / 2 - 300, ALLEGRO_ALIGN_CENTER, "TOP 5 HIGH SCORES"); // Titulo destacado de la pantalla de puntuaciones

        vector<Estadistica> top5 = obtenerTop5(); // Recupera las cinco mejores estadisticas almacenadas

        if (top5.empty()) { // Si aun no hay registros guardados
                al_draw_text(fuente_mediana, al_map_rgb(150, 150, 150), ancho / 2, alto / 2, ALLEGRO_ALIGN_CENTER, "No hay puntuaciones registradas aun"); // Mensaje informativo al usuario
        } else { // Existen registros que mostrar
                int y = -180; // Desplazamiento vertical inicial relativo al centro de la pantalla
                for (size_t i = 0; i < top5.size(); i++) { // Recorre cada entrada del top 5
                        ALLEGRO_COLOR color; // Variable para almacenar el color segun la posicion
                        if (i == 0) color = al_map_rgb(255, 215, 0); // Oro para el primer lugar
                        else if (i == 1) color = al_map_rgb(192, 192, 192); // Plata para el segundo lugar
                        else if (i == 2) color = al_map_rgb(205, 127, 50); // Bronce para el tercer lugar
                        else color = al_map_rgb(200, 200, 200); // Gris para el resto

                        char nombre[100]; // Buffer para construir la linea de nombre y posicion
                        sprintf_s(nombre, 100, "%d. %s", (int)(i + 1), top5[i].nombre.c_str()); // Formatea la posicion y el nombre
                        al_draw_text(fuente_mediana, color, ancho / 2, alto / 2 + y, ALLEGRO_ALIGN_CENTER, nombre); // Dibuja el nombre con color segun el podio

                        char datos[180]; // Buffer con informacion detallada de la estadistica
                        sprintf_s(datos, 180, "Puntos: %d | Ronda: %d | Tiempo: %.1f s | Enemigos: %d | Disparos: %d", top5[i].puntuacion, top5[i].ronda, top5[i].tiempo, top5[i].enemigos_eliminados, top5[i].proyectiles_disparados); // Compone la descripcion
                        al_draw_text(fuente_mediana, al_map_rgb(120, 120, 120), ancho / 2, alto / 2 + y + 30, ALLEGRO_ALIGN_CENTER, datos); // Muestra los datos secundarios en gris suave

                        y += 80; // Avanza la posicion vertical para la siguiente entrada
                }
        }

        al_draw_text(fuente_mediana, al_map_rgb(150, 150, 150), ancho / 2, alto - 80, ALLEGRO_ALIGN_CENTER, "Presiona ESC para volver al menu"); // Instruccion para regresar al menu
        al_flip_display(); // Actualiza la pantalla con el contenido renderizado
}

// ========== FUNCION PRINCIPAL ==========

int main() {
        srand((unsigned int)time(NULL)); // Inicializa el generador de numeros aleatorios con la hora actual

        if (!al_init()) { // Comprueba si Allegro se inicializa correctamente
                al_show_native_message_box(NULL, "Error", "Error", "No se pudo iniciar Allegro", NULL, 0); // Muestra un dialogo de error
                return -1; // Finaliza la aplicacion con codigo de error
        }

        al_init_font_addon(); // Activa el addon de fuentes de Allegro
        al_init_ttf_addon(); // Activa el addon para fuentes TrueType
        al_init_image_addon(); // Activa el addon de imagenes
        al_init_primitives_addon(); // Activa el addon de primitivas graficas
        al_install_keyboard(); // Habilita la lectura del teclado
        al_uninstall_mouse(); // Desactiva el raton porque no se utiliza
        al_install_audio(); // Inicializa el subsistema de audio
        al_init_acodec_addon(); // Habilita los codecs necesarios para reproducir sonido
        al_reserve_samples(16); // Reserva 16 canales de audio simultaneos para musica y efectos

        ALLEGRO_MONITOR_INFO info; // Estructura para almacenar informacion del monitor principal
        al_get_monitor_info(0, &info); // Obtiene las dimensiones del monitor 0
        int ancho = info.x2 - info.x1; // Calcula el ancho total de la pantalla
        int alto = info.y2 - info.y1; // Calcula el alto total de la pantalla

        ALLEGRO_DISPLAY* pantalla = al_create_display(ancho, alto); // Crea una ventana o pantalla a resolucion completa
        if (!pantalla) { // Verifica que la pantalla se haya creado correctamente
                al_show_native_message_box(NULL, "Error", "Error", "No se pudo crear la pantalla", NULL, 0); // Informa si hubo un error creando la ventana
                return -1; // Termina la ejecucion porque no se puede continuar sin pantalla
        }

        ALLEGRO_FONT* font_grande = al_load_ttf_font("MONSTER.ttf", 72, 0); // Carga la fuente grande usada en el titulo
        if (!font_grande) { // Comprueba que la fuente se haya cargado
                al_show_native_message_box(pantalla, "Error", "Error", "No se pudo cargar la fuente grande", NULL, 0); // Muestra mensaje si falla
                return -1; // Cancela la aplicacion para evitar fallos posteriores
        }

        ALLEGRO_FONT* font_mediana = al_load_ttf_font("MONSTER.ttf", 24, 0); // Carga la fuente mediana para textos generales
        if (!font_mediana) { // Valida la carga de la fuente mediana
                al_show_native_message_box(pantalla, "Error", "Error", "No se pudo cargar la fuente mediana", NULL, 0); // Muestra aviso de error
                return -1; // Interrumpe la ejecucion si no se puede dibujar texto
        }

        ALLEGRO_FONT* font_pequena = al_load_ttf_font("MONSTER.ttf", 16, 0); // Carga la fuente pequena para instrucciones
        if (!font_pequena) { // Comprueba que se cargo la fuente pequena
                al_show_native_message_box(pantalla, "Error", "Error", "No se pudo cargar la fuente pequena", NULL, 0); // Muestra mensaje de error
                return -1; // Finaliza porque el menu necesita esta fuente
        }

        ALLEGRO_BITMAP* fondo_menu = al_load_bitmap("Imagenes/menu.png"); // Intenta cargar la imagen del menu principal
        if (!fondo_menu) { // Si la imagen no esta disponible
                al_show_native_message_box(pantalla, "Advertencia", "Aviso", "No se pudo cargar la imagen de fondo del menu", NULL, ALLEGRO_MESSAGEBOX_WARN); // Advierte al usuario pero no detiene el programa
        }

        ALLEGRO_BITMAP* fondo_gameplay = al_load_bitmap("Imagenes/gameplay.png"); // Carga la imagen de fondo para el gameplay
        if (!fondo_gameplay) { // Verifica si se logro cargar
                al_show_native_message_box(pantalla, "Advertencia", "Aviso", "No se pudo cargar la imagen de fondo del gameplay", NULL, ALLEGRO_MESSAGEBOX_WARN); // Notifica la ausencia del fondo de juego
        }

        cargarAudio(); // Carga todos los samples de audio definidos en Funciones.h
        tocarMusica(musica_menu, 0.5f); // Reproduce la musica del menu en bucle con volumen moderado

        ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0); // Crea un temporizador para generar eventos a 60 FPS
        ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue(); // Crea la cola donde se almacenaran eventos del sistema

        al_register_event_source(queue, al_get_keyboard_event_source()); // Registra el teclado como fuente de eventos
        al_register_event_source(queue, al_get_display_event_source(pantalla)); // Registra la ventana de la pantalla como fuente de eventos
        al_register_event_source(queue, al_get_timer_event_source(timer)); // Registra el temporizador para recibir ticks de actualizacion

        EstadoApp app = APP_MENU; // Variable que guarda el estado actual de la aplicacion, inicia en el menu
        int opcion = 0; // Indica cual opcion del menu esta seleccionada al inicio
        float timer_anim = 0.0f; // Acumula tiempo para potenciales animaciones de interfaz

        al_start_timer(timer); // Inicia el temporizador para que comience a generar eventos de reloj
        bool running = true; // Bandera que indica si el bucle principal debe seguir ejecutandose

        while (running) { // Bucle principal que se ejecuta hasta que el usuario sale
                ALLEGRO_EVENT ev; // Estructura para recibir eventos
                al_wait_for_event(queue, &ev); // Espera bloqueante hasta recibir un evento disponible

                if (app == APP_MENU && ev.type == ALLEGRO_EVENT_KEY_DOWN) { // Gestion de entradas mientras se esta en el menu
                        if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) { // Si se presiona Escape en el menu
                                running = false; // Se sale por completo de la aplicacion
                        }

                        if (ev.keyboard.keycode == ALLEGRO_KEY_W || ev.keyboard.keycode == ALLEGRO_KEY_UP) { // Movimiento hacia arriba en el menu
                                opcion--; // Decrementa la opcion seleccionada
                                if (opcion < 0) opcion = 2; // Hace wrap-around para volver a la ultima opcion
                        }

                        if (ev.keyboard.keycode == ALLEGRO_KEY_S || ev.keyboard.keycode == ALLEGRO_KEY_DOWN) { // Movimiento hacia abajo en el menu
                                opcion++; // Incrementa la opcion activa
                                if (opcion >= 3) opcion = 0; // Reinicia al inicio cuando pasa del ultimo elemento
                        }

                        if (ev.keyboard.keycode == ALLEGRO_KEY_ENTER) { // Confirmacion de la opcion actual
                                if (opcion == 0) { // Si el usuario eligio jugar
                                        iniciarJuego(ancho, alto, font_mediana, timer, queue, fondo_gameplay); // Lanza el gameplay principal con los recursos necesarios
                                        tocarMusica(musica_menu, 0.5f); // Reanuda la musica del menu tras salir del juego
                                } else if (opcion == 1) { // Si el usuario quiere ver los high scores
                                        app = APP_HIGH_SCORES; // Cambia al estado de pantalla de puntuaciones
                                } else if (opcion == 2) { // Si el usuario decide salir
                                        running = false; // Termina el bucle principal para cerrar la aplicacion
                                }
                        }
                }

                if (app == APP_HIGH_SCORES && ev.type == ALLEGRO_EVENT_KEY_DOWN && ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) { // Gestiona la salida de la pantalla de high scores
                        app = APP_MENU; // Regresa al estado de menu cuando se presiona Escape
                }

                if (ev.type == ALLEGRO_EVENT_TIMER && ev.timer.source == timer) { // Se ejecuta cada tick del temporizador
                        timer_anim += 1.0f / 60.0f; // Incrementa el acumulador temporal a razon de un frame

                        if (app == APP_MENU) { // Si se esta en el menu
                                renderizarMenu(opcion, font_grande, font_mediana, font_pequena, ancho, alto, timer_anim, fondo_menu); // Redibuja el menu con la opcion actual
                        } else if (app == APP_HIGH_SCORES) { // Si se esta en la pantalla de puntuaciones
                                renderizarPantallaHighScores(font_grande, font_mediana, ancho, alto); // Actualiza la vista del top 5
                        }
                }

                if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) { // Maneja el evento de cierre de la ventana
                        running = false; // Termina el bucle principal para salir correctamente
                }
        }

        limpiarAudio(); // Libera todos los recursos de audio cargados previamente
        if (fondo_menu) al_destroy_bitmap(fondo_menu); // Destruye el bitmap del menu si fue cargado
        if (fondo_gameplay) al_destroy_bitmap(fondo_gameplay); // Destruye el bitmap del gameplay si existe
        al_destroy_font(font_grande); // Libera la fuente grande
        al_destroy_font(font_mediana); // Libera la fuente mediana
        al_destroy_font(font_pequena); // Libera la fuente pequena
        al_destroy_event_queue(queue); // Destruye la cola de eventos
        al_destroy_timer(timer); // Destruye el temporizador
        al_destroy_display(pantalla); // Cierra y libera la pantalla principal

        return 0; // Finaliza la aplicacion indicando exito
}
