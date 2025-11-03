#include <stdio.h>
#include <cmath>
#include <iostream>

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>

#include "Funciones.h"

using namespace std;

constexpr auto FPS = 60.0;

// Polígono local de la nave 
static float Puntos_jugador[] = {
    0.0f, -30.0f,   // A
    -30.0f,   30.0f,   // B  
    0.0f,  10.0f, // C
    30.0f,   30.0f
};

static const int NAVE_N = 4;

static float v[] = { 
    0.0f,-45.0f,
    -35.0f, 27.0f,
    35.0f, 27.0f 
};

// Física estilo Asteroids
static const float ROTACION = 0.07f;   
static const float ACELERACION = 0.35f;   
static const float ROZAMIENTO = 0.995f;  
static const float VELOCIDAD_MAX = 9.0f;    


int main() {
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

    // --- Timer + cola de eventos ---
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / FPS);
    ALLEGRO_EVENT_QUEUE* cola = al_create_event_queue();

    al_register_event_source(cola, al_get_keyboard_event_source());
    al_register_event_source(cola, al_get_display_event_source(monitor));
    al_register_event_source(cola, al_get_timer_event_source(timer));

    Nave jugador;
    Nave seeker;
    Nave wanderer;
    iniciarPersonaje(jugador, X, Y);
    iniciarMonstruo(wanderer, X, Y);
    iniciarMonstruo(seeker, X, Y);

    // --- Input flags ---
    bool AVANZAR = false;   
    bool DERECHA = false;   
    bool IZQUIERDA = false; 

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
                AVANZAR = true;  
                break;

            case ALLEGRO_KEY_D:
                DERECHA = true; 
                break;

            case ALLEGRO_KEY_A:
                IZQUIERDA = true; 
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
            }
        }

        // Lógica por tick
        if (evento.type == ALLEGRO_EVENT_TIMER && evento.timer.source == timer) {


            if (evento.timer.source == timer) {
                movimientoWanderer(wanderer);
            }

            if (evento.timer.source == timer) {
                movimientoSeeker(seeker, jugador);
            }

            // --- Rotación ---
            if (IZQUIERDA) jugador.ang -= ROTACION;
            if (DERECHA)   jugador.ang += ROTACION;

            // --- Thrust (W): empuje en dirección de la "nariz"
            if (AVANZAR) {

                float fx = sin(jugador.ang);
                float fy = -cos(jugador.ang); // modelo apunta a -Y
                jugador.vx += fx * ACELERACION;
                jugador.vy += fy * ACELERACION;
            }

            // --- Rozamiento + clamp velocidad ---
            jugador.vx *= ROZAMIENTO;
            jugador.vy *= ROZAMIENTO;
            float sp2 = jugador.vx * jugador.vx + jugador.vy * jugador.vy;
            if (sp2 > VELOCIDAD_MAX * VELOCIDAD_MAX) {
                float k = VELOCIDAD_MAX / sqrt(sp2);
                jugador.vx *= k; jugador.vy *= k;
            }

            // --- Integración posición ---
            jugador.x += jugador.vx;
            jugador.y += jugador.vy;

            // --- Bordes estilo Asteroids (wrap-around) ---
            if (jugador.x < 0)  jugador.x += X;
            if (jugador.x >= X) jugador.x -= X;
            if (jugador.y < 0)  jugador.y += Y;
            if (jugador.y >= Y) jugador.y -= Y;

            // --- Render ---
            al_clear_to_color(al_map_rgb(0, 0, 0));

            // Dibujo de la nave con rotación + traslación (transform)
            ALLEGRO_TRANSFORM saved;
            al_copy_transform(&saved, al_get_current_transform());

            ALLEGRO_TRANSFORM t;
            al_identity_transform(&t);
            al_rotate_transform(&t, jugador.ang);
            al_translate_transform(&t, jugador.x, jugador.y);
            al_use_transform(&t);

            // Relleno + contorno
            al_draw_filled_polygon(Puntos_jugador, NAVE_N, al_map_rgb(60, 180, 255));
            al_draw_polygon(Puntos_jugador, NAVE_N, ALLEGRO_LINE_JOIN_ROUND,al_map_rgb(255, 255, 255), 1.5f, 1.0f);

            al_use_transform(&saved); // restaurar para futuros dibujados

            al_draw_circle(wanderer.x, wanderer.y, 50.0f, al_map_rgb(170, 255, 170), 2);
            al_draw_circle(wanderer.x, wanderer.y, 45.0f, al_map_rgb(170, 255, 170), 3);

            ALLEGRO_TRANSFORM old; al_copy_transform(&old, al_get_current_transform());
            ALLEGRO_TRANSFORM Ts;  al_identity_transform(&Ts);

            // si quieres que mire al jugador:
            float ang = atan2f(jugador.y - seeker.y, jugador.x - seeker.x) + ALLEGRO_PI/2;
            al_rotate_transform(&Ts, ang);

            al_translate_transform(&Ts, seeker.x, seeker.y);
            al_use_transform(&Ts);

            al_draw_triangle(v[0], v[1], v[2], v[3], v[4], v[5], al_map_rgb(255, 100, 220), 6);
            al_draw_triangle(v[0], v[1], v[2], v[3], v[4], v[5], al_map_rgb(255, 255, 255), 3);
            al_draw_triangle(v[0], v[1] + 20, v[2] + 15, v[3] - 10, v[4] - 15, v[5] - 10, al_map_rgb(255, 100, 220), 3);
            al_draw_triangle(v[0], v[1] + 20, v[2] + 15, v[3] - 10, v[4] - 15, v[5] - 10, al_map_rgb(255, 255, 255), 1);

            al_use_transform(&old);


            /*draw_wanderer_slouch(200.0f, 300.0f);*/
            

            al_flip_display();
        }

        if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            ciclo = false;
        }
    }

    // Cleanup
    al_destroy_event_queue(cola);
    al_destroy_timer(timer);
    al_destroy_display(monitor);
    return 0;
}