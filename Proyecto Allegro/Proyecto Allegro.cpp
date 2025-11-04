#include <stdio.h>
#include <cmath>
#include <iostream>
#include <cstdlib>
#include <ctime>

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>

#include "Funciones.h"
#include "juego.h"

using namespace std;


int main() {
    // Inicializar semilla aleatoria
    srand(static_cast<unsigned int>(time(nullptr)));

    // --- Allegro core + addons ---
    if (!al_init()) {
        al_show_native_message_box(nullptr, "Error", "Error", "No se pudo inicializar Allegro", nullptr, 0);
        return -1;
    }
    al_init_font_addon();
    al_init_ttf_addon();
    al_init_image_addon();
    al_init_primitives_addon();
    al_install_keyboard();
    al_uninstall_mouse();

    // --- Display al tamaño de monitor ---
    ALLEGRO_MONITOR_INFO info;
    al_get_monitor_info(0, &info);
    const int X = info.x2 - info.x1;
    const int Y = info.y2 - info.y1;

    ALLEGRO_DISPLAY* monitor = al_create_display(X, Y);
    if (!monitor) {
        al_show_native_message_box(nullptr, "Error", "Error", "No se pudo crear el display", nullptr, 0);
        return -1;
    }

    // --- Cargar fuente (usar fuente built-in de Allegro) ---
    ALLEGRO_FONT* fuente = al_create_builtin_font();
    if (!fuente) {
        al_show_native_message_box(monitor, "Error", "Error", "No se pudo cargar la fuente", nullptr, 0);
        return -1;
    }

    // --- Timer + cola de eventos ---
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / FPS);
    ALLEGRO_EVENT_QUEUE* cola = al_create_event_queue();

    al_register_event_source(cola, al_get_keyboard_event_source());
    al_register_event_source(cola, al_get_display_event_source(monitor));
    al_register_event_source(cola, al_get_timer_event_source(timer));

    // --- Estado del juego ---
    EstadoJuego estado_actual = JUGANDO;
    float timer_transicion = 0.0f;

    // --- Inicialización del jugador ---
    Nave jugador;
    iniciarPersonaje(jugador, X, Y);

    // --- Sistema de oleadas ---
    PtrNave lista_enemigos = nullptr;
    int ronda_actual = 1;
    
    // Generar primera oleada
    generarOleada(lista_enemigos, ronda_actual, X, Y);

    // --- Inicialización de sistema de balas ---
    PtrBala lista_balas = nullptr;
    float cooldown_disparo = 0.0f;

    // --- Variables de juego ---
    int puntuacion = 0;
    int enemigos_totales_eliminados = 0;
    float tiempo_juego = 0.0f;

    // --- Input flags ---
    bool AVANZAR = false;   
    bool DERECHA = false;   
    bool IZQUIERDA = false;
    bool DISPARAR = false;

    // --- Loop principal ---
    al_start_timer(timer);
    bool ciclo = true;
    
    while (ciclo) {
        ALLEGRO_EVENT evento;
        al_wait_for_event(cola, &evento);

        // Teclas down
        if (evento.type == ALLEGRO_EVENT_KEY_DOWN) {
            switch (evento.keyboard.keycode) {
            case ALLEGRO_KEY_ESCAPE:
                ciclo = false;
                break; 

            case ALLEGRO_KEY_W:
                if (estado_actual == JUGANDO) AVANZAR = true;  
                break;

            case ALLEGRO_KEY_D:
                if (estado_actual == JUGANDO) DERECHA = true; 
                break;

            case ALLEGRO_KEY_A:
                if (estado_actual == JUGANDO) IZQUIERDA = true; 
                break;

            case ALLEGRO_KEY_SPACE:
                if (estado_actual == JUGANDO) DISPARAR = true;
                break;
            }
        }
        
        // Teclas up
        if (evento.type == ALLEGRO_EVENT_KEY_UP) {
            switch (evento.keyboard.keycode) {
            case ALLEGRO_KEY_W:
                AVANZAR = false;  
                break;

            case ALLEGRO_KEY_D:
                DERECHA = false; 
                break;

            case ALLEGRO_KEY_A:
                IZQUIERDA = false; 
                break;

            case ALLEGRO_KEY_SPACE:
                DISPARAR = false;
                break;
            }
        }

        // Lógica por tick
        if (evento.type == ALLEGRO_EVENT_TIMER && evento.timer.source == timer) {

            // Actualizar estado del juego
            actualizarEstadoJuego(
                estado_actual,
                timer_transicion,
                jugador,
                lista_enemigos,
                lista_balas,
                cooldown_disparo,
                ronda_actual,
                puntuacion,
                enemigos_totales_eliminados,
                tiempo_juego,
                AVANZAR,
                DERECHA,
                IZQUIERDA,
                DISPARAR,
                X,
                Y
            );

            // Renderizar juego
            renderizarJuego(
                estado_actual,
                timer_transicion,
                jugador,
                lista_enemigos,
                lista_balas,
                ronda_actual,
                puntuacion,
                enemigos_totales_eliminados,
                tiempo_juego,
                fuente,
                X,
                Y
            );
        }

        if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            ciclo = false;
        }
    }

    // Cleanup
    liberarEnemigos(lista_enemigos);
    liberarBalas(lista_balas);
    al_destroy_font(fuente);
    al_destroy_event_queue(cola);
    al_destroy_timer(timer);
    al_destroy_display(monitor);
    
    return 0;
}