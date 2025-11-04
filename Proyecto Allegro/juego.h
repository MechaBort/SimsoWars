#pragma once

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <cmath>
#include "Funciones.h"

// ============================================================================
// CONSTANTES DE FÍSICA Y GAMEPLAY
// ============================================================================

const float FPS = 60.0f;
const float ROTACION = 0.07f;   
const float ACELERACION = 0.35f;   
const float ROZAMIENTO = 0.995f;  
const float VELOCIDAD_MAX = 9.0f;

// ============================================================================
// GEOMETRÍA DE LAS NAVES
// ============================================================================

// Polígono local de la nave del jugador
static float Puntos_jugador[] = {
    0.0f, -30.0f,   // A
    -30.0f, 30.0f,  // B  
    0.0f, 10.0f,    // C
    30.0f, 30.0f    // D
};
static const int NAVE_N = 4;

// Triángulos del Seeker (diseño original con 4 capas)
static float v[] = { 
    0.0f, -45.0f,
    -35.0f, 27.0f,
    35.0f, 27.0f 
};

// ============================================================================
// FUNCIONES DE ACTUALIZACIÓN DEL JUEGO
// ============================================================================

// Actualizar la lógica del juego según el estado actual
void actualizarEstadoJuego(
    EstadoJuego& estado_actual,
    float& timer_transicion,
    Nave& jugador,
    PtrNave& lista_enemigos,
    PtrBala& lista_balas,
    float& cooldown_disparo,
    int& ronda_actual,
    int& puntuacion,
    int& enemigos_totales_eliminados,
    float& tiempo_juego,
    bool AVANZAR,
    bool DERECHA,
    bool IZQUIERDA,
    bool DISPARAR,
    int anchoMax,
    int altoMax
) {
    switch (estado_actual) {

    // ========== ESTADO: JUGANDO ==========
    case JUGANDO: {
        // Incrementar tiempo de juego
        tiempo_juego += 1.0f / FPS;

        // --- Sistema de disparo con cooldown ---
        if (cooldown_disparo > 0.0f) {
            cooldown_disparo -= 1.0f;
        }

        if (DISPARAR && cooldown_disparo <= 0.0f && jugador.activo) {
            dispararBala(lista_balas, jugador);
            cooldown_disparo = CADENCIA_DISPARO;
        }

        // --- Actualizar enemigos ---
        actualizarEnemigos(lista_enemigos, jugador, anchoMax, altoMax);

        // --- Actualizar balas ---
        actualizarBalas(lista_balas);

        // --- Verificar colisiones balas-enemigos ---
        int eliminados = verificarColisionesBalasEnemigos(lista_balas, lista_enemigos);
        if (eliminados > 0) {
            enemigos_totales_eliminados += eliminados;
            puntuacion += eliminados * 100;
        }

        // --- Limpiar balas inactivas ---
        limpiarBalas(lista_balas);
        
        // --- Limpiar enemigos muertos ---
        limpiarEnemigosInactivos(lista_enemigos);

        // --- Verificar si terminó la oleada ---
        if (contarEnemigosActivos(lista_enemigos) == 0 && jugador.activo) {
            // Oleada completada - Transición a CAMBIO_RONDA
            estado_actual = CAMBIO_RONDA;
            timer_transicion = DURACION_TRANSICION;
            ronda_actual++;
            
            // Limpiar TODAS las balas
            liberarBalas(lista_balas);
            lista_balas = nullptr;
            
            // Resetear jugador al centro
            resetearJugador(jugador, anchoMax, altoMax);
        }

        // --- Verificar colisión jugador-enemigos ---
        if (verificarColisionJugadorEnemigos(jugador, lista_enemigos)) {
            jugador.activo = false;
            estado_actual = GAME_OVER;
        }

        // --- Movimiento del jugador ---
        if (jugador.activo) {
            if (IZQUIERDA) jugador.ang -= ROTACION;
            if (DERECHA)   jugador.ang += ROTACION;

            // --- Thrust (W): empuje en dirección de la "nariz" ---
            if (AVANZAR) {
                float fx = sin(jugador.ang);
                float fy = -cos(jugador.ang);
                jugador.vx += fx * ACELERACION;
                jugador.vy += fy * ACELERACION;
            }

            // --- Rozamiento + clamp velocidad ---
            jugador.vx *= ROZAMIENTO;
            jugador.vy *= ROZAMIENTO;
            float sp2 = jugador.vx * jugador.vx + jugador.vy * jugador.vy;
            if (sp2 > VELOCIDAD_MAX * VELOCIDAD_MAX) {
                float k = VELOCIDAD_MAX / sqrt(sp2);
                jugador.vx *= k; 
                jugador.vy *= k;
            }

            // --- Integración posición ---
            jugador.x += jugador.vx;
            jugador.y += jugador.vy;

            // --- Bordes limitados (sin wrap-around) ---
            if (jugador.x < 25)  jugador.x = 25;
            if (jugador.x >= anchoMax - 25) jugador.x = anchoMax - 25;
            if (jugador.y < 25)  jugador.y = 25;
            if (jugador.y >= altoMax - 25) jugador.y = altoMax - 25;
        }
        break;
    }

    // ========== ESTADO: CAMBIO_RONDA ==========
    case CAMBIO_RONDA: {
        timer_transicion -= 1.0f;
        
        if (timer_transicion <= 0.0f) {
            // Fin de la transición - Generar nueva oleada
            generarOleada(lista_enemigos, ronda_actual, anchoMax, altoMax);
            estado_actual = JUGANDO;
        }
        break;
    }

    // ========== ESTADO: GAME_OVER ==========
    case GAME_OVER: {
        // Por ahora solo espera, más adelante agregamos input de nombre
        break;
    }
    }
}

// ============================================================================
// FUNCIONES DE RENDERIZADO
// ============================================================================

// Renderizar el juego según el estado actual
void renderizarJuego(
    EstadoJuego estado_actual,
    float timer_transicion,
    Nave& jugador,
    PtrNave lista_enemigos,
    PtrBala lista_balas,
    int ronda_actual,
    int puntuacion,
    int enemigos_totales_eliminados,
    float tiempo_juego,
    ALLEGRO_FONT* fuente,
    int anchoMax,
    int altoMax
) {
    al_clear_to_color(al_map_rgb(0, 0, 0));

    switch (estado_actual) {

    // ========== RENDER: JUGANDO ==========
    case JUGANDO: {
        // --- Dibujar jugador ---
        if (jugador.activo) {
            ALLEGRO_TRANSFORM saved;  
            al_copy_transform(&saved, al_get_current_transform());

            ALLEGRO_TRANSFORM t;
            al_identity_transform(&t);
            al_rotate_transform(&t, jugador.ang);
            al_translate_transform(&t, jugador.x, jugador.y);
            al_use_transform(&t);

            al_draw_filled_polygon(Puntos_jugador, NAVE_N, al_map_rgb(60, 180, 255));
            al_draw_polygon(Puntos_jugador, NAVE_N, ALLEGRO_LINE_JOIN_ROUND, al_map_rgb(255, 255, 255), 1.5f, 1.0f);

            al_use_transform(&saved);
        }

        // --- Dibujar enemigos ---
        PtrNave enemigo = lista_enemigos;
        while (enemigo != nullptr) {
            if (enemigo->activo) {
                if (enemigo->tipo == 1) { // Drone
                    al_draw_circle(enemigo->x, enemigo->y, 50.0f, al_map_rgb(170, 255, 170), 2);
                    al_draw_circle(enemigo->x, enemigo->y, 45.0f, al_map_rgb(170, 255, 170), 3);
                } 
                else if (enemigo->tipo == 2) { // Seeker
                    ALLEGRO_TRANSFORM old; 
                    al_copy_transform(&old, al_get_current_transform());
                    ALLEGRO_TRANSFORM Ts;  
                    al_identity_transform(&Ts);

                    float ang = atan2f(jugador.y - enemigo->y, jugador.x - enemigo->x) + ALLEGRO_PI/2;
                    al_rotate_transform(&Ts, ang);
                    al_translate_transform(&Ts, enemigo->x, enemigo->y);
                    al_use_transform(&Ts);

                    al_draw_triangle(v[0], v[1], v[2], v[3], v[4], v[5], al_map_rgb(255, 100, 220), 6);
                    al_draw_triangle(v[0], v[1], v[2], v[3], v[4], v[5], al_map_rgb(255, 255, 255), 3);
                    al_draw_triangle(v[0], v[1] + 20, v[2] + 15, v[3] - 10, v[4] - 15, v[5] - 10, al_map_rgb(255, 100, 220), 3);
                    al_draw_triangle(v[0], v[1] + 20, v[2] + 15, v[3] - 10, v[4] - 15, v[5] - 10, al_map_rgb(255, 255, 255), 1);

                    al_use_transform(&old);
                }
            }
            enemigo = enemigo->siguiente;
        }

        // --- Dibujar balas ---
        PtrBala bala = lista_balas;
        while (bala != nullptr) {
            if (bala->activa) {
                al_draw_filled_circle(bala->x, bala->y, RADIO_BALA, al_map_rgb(255, 255, 0));
            }
            bala = bala->siguiente;
        }

        // --- HUD ---
        al_draw_textf(fuente, al_map_rgb(255, 255, 255), 10, 10, ALLEGRO_ALIGN_LEFT, "PUNTUACION: %d", puntuacion);
        al_draw_textf(fuente, al_map_rgb(255, 255, 255), 10, 25, ALLEGRO_ALIGN_LEFT, "RONDA: %d", ronda_actual);
        al_draw_textf(fuente, al_map_rgb(255, 255, 255), 10, 40, ALLEGRO_ALIGN_LEFT, "TIEMPO: %.1f", tiempo_juego);
        break;
    }

    // ========== RENDER: CAMBIO_RONDA ==========
    case CAMBIO_RONDA: {
        char texto_ronda[50];
        sprintf_s(texto_ronda, sizeof(texto_ronda), "RONDA %d", ronda_actual);
        
        // Efecto de parpadeo
        float alpha = (int)(timer_transicion / 10) % 2 == 0 ? 1.0f : 0.5f;
        ALLEGRO_COLOR color = al_map_rgba_f(1.0f * alpha, 1.0f * alpha, 1.0f * alpha, alpha);
        
        al_draw_text(fuente, color, anchoMax / 2, altoMax / 2 - 50, ALLEGRO_ALIGN_CENTER, texto_ronda);
        al_draw_text(fuente, al_map_rgb(200, 200, 200), anchoMax / 2, altoMax / 2, ALLEGRO_ALIGN_CENTER, "Preparate...");
        break;
    }

    // ========== RENDER: GAME_OVER ==========
    case GAME_OVER: {
        al_draw_text(fuente, al_map_rgb(255, 0, 0), anchoMax / 2, altoMax / 2 - 100, ALLEGRO_ALIGN_CENTER, "GAME OVER");
        al_draw_textf(fuente, al_map_rgb(255, 255, 255), anchoMax / 2, altoMax / 2 - 50, ALLEGRO_ALIGN_CENTER, "Puntuacion Final: %d", puntuacion);
        al_draw_textf(fuente, al_map_rgb(255, 255, 255), anchoMax / 2, altoMax / 2 - 25, ALLEGRO_ALIGN_CENTER, "Tiempo: %.1f segundos", tiempo_juego);
        al_draw_textf(fuente, al_map_rgb(255, 255, 255), anchoMax / 2, altoMax / 2, ALLEGRO_ALIGN_CENTER, "Enemigos Eliminados: %d", enemigos_totales_eliminados);
        al_draw_textf(fuente, al_map_rgb(255, 255, 255), anchoMax / 2, altoMax / 2 + 25, ALLEGRO_ALIGN_CENTER, "Ronda Alcanzada: %d", ronda_actual);
        al_draw_text(fuente, al_map_rgb(150, 150, 150), anchoMax / 2, altoMax / 2 + 60, ALLEGRO_ALIGN_CENTER, "Presiona ESC para salir");
        break;
    }
    }

    al_flip_display();
}
