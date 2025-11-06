/*
 * JUEGO.H
 * -------
 * Lógica principal del gameplay (controles, física, colisiones, renderizado)
 */

#pragma once

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <cmath>
#include "Funciones.h"

using namespace std;

// ========== CONSTANTES DE FÍSICA ==========

const float FPS = 60.0f;
const float ROTACION = 0.07f;
const float ACELERACION = 0.35f;
const float ROZAMIENTO = 0.985f;
const float VELOCIDAD_MAX = 9.0f;

// ========== GEOMETRÍA ==========

static float Puntos_jugador[] = {
	0.0f, -30.0f,
	-30.0f, 30.0f,
	0.0f, 10.0f,
	30.0f, 30.0f
};

static float v[] = {
	0.0f, -45.0f,
	-35.0f, 27.0f,
	35.0f, 27.0f
};

// ========== FUNCIÓN PRINCIPAL DEL JUEGO ==========

void iniciarJuego(int ancho, int alto, ALLEGRO_FONT* font, ALLEGRO_TIMER* timer, ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_BITMAP* fondo_gameplay) {
	tocarMusica(musica_gameplay, 0.05f);
	
	// Variables del juego
	EstadoJuego estado = JUGANDO;
	float timer_trans = 0.0f;  // Tiempo restante de transición entre rondas
	Nave player;
	PtrNave enemigos = NULL;
	PtrBala balas = NULL;
	float cooldown = 0.0f;  // Cooldown del disparo
	int ronda = 1;
	int puntos = 0;
	int kills = 0;
	int proyectiles = 0;
	float tiempo = 0.0f;  // Tiempo de la partida
	float tiempo_total = 0.0f;  // Tiempo total (incluye pantallas)
	float delay_muerte = 0.0f;  // Pausa antes del game over
	string nombre = "";
	bool W = false, D = false, A = false, SPACE = false;  // Teclas presionadas
	
	iniciarPersonaje(player, ancho, alto);
	generarOleada(enemigos, ronda, ancho, alto);
	
	// Bucle principal
	bool jugando = true;
	while (jugando) {
		ALLEGRO_EVENT ev;
		al_wait_for_event(queue, &ev);
		
		// INPUT
		if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
			if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
				jugando = false;  // Volver al menú
			}
			
			// Controles de movimiento (solo durante gameplay)
			if (ev.keyboard.keycode == ALLEGRO_KEY_W && estado == JUGANDO) W = true;
			if (ev.keyboard.keycode == ALLEGRO_KEY_D && estado == JUGANDO) D = true;
			if (ev.keyboard.keycode == ALLEGRO_KEY_A && estado == JUGANDO) A = true;
			if (ev.keyboard.keycode == ALLEGRO_KEY_SPACE && estado == JUGANDO) SPACE = true;
			
			// ENTER: cambiar de estado
			if (ev.keyboard.keycode == ALLEGRO_KEY_ENTER) {
				if (estado == GAME_OVER) {
					estado = INPUT_NOMBRE;  // Pasar a captura de nombre
					nombre = "";
				} else if (estado == INPUT_NOMBRE) {
					if (nombre.empty()) nombre = "ANONIMO";
					
					// Guardar estadísticas
					Estadistica s;
					s.nombre = nombre;
					s.puntuacion = puntos;
					s.tiempo = tiempo;
					s.ronda = ronda;
					s.enemigos_eliminados = kills;
					s.proyectiles_disparados = proyectiles;
					guardarEstadisticas(s);
					
					jugando = false;  // Volver al menú
				}
			}
			
			// BACKSPACE: borrar letra del nombre
			if (ev.keyboard.keycode == ALLEGRO_KEY_BACKSPACE && estado == INPUT_NOMBRE && !nombre.empty()) {
				nombre.pop_back();
			}
			
			// Capturar nombre (A-Z, 0-9, espacio)
			if (estado == INPUT_NOMBRE) {
				int key = ev.keyboard.keycode;
				
				// Letras A-Z
				if (key >= ALLEGRO_KEY_A && key <= ALLEGRO_KEY_Z && nombre.length() < 15) {
					char letra = 'A' + (key - ALLEGRO_KEY_A);
					nombre += letra;
				}
				
				// Números 0-9
				if (key >= ALLEGRO_KEY_0 && key <= ALLEGRO_KEY_9 && nombre.length() < 15) {
					char num = '0' + (key - ALLEGRO_KEY_0);
					nombre += num;
				}
				
				// Espacio
				if (key == ALLEGRO_KEY_SPACE && nombre.length() < 15) {
					nombre += ' ';
				}
			}
		}
		
		// Soltar teclas
		if (ev.type == ALLEGRO_EVENT_KEY_UP) {
			if (ev.keyboard.keycode == ALLEGRO_KEY_W) W = false;
			if (ev.keyboard.keycode == ALLEGRO_KEY_D) D = false;
			if (ev.keyboard.keycode == ALLEGRO_KEY_A) A = false;
			if (ev.keyboard.keycode == ALLEGRO_KEY_SPACE) SPACE = false;
		}
		
		// ACTUALIZACIÓN (60 FPS)
		if (ev.type == ALLEGRO_EVENT_TIMER && ev.timer.source == timer) {
			tiempo_total += 1.0f / 60.0f;
			
			if (estado == JUGANDO) {
				tiempo += 1.0f / 60.0f;
				
				// Sistema de disparo
				if (cooldown > 0.0f) cooldown -= 1.0f;
				if (SPACE && cooldown <= 0.0f && player.activo) {
					dispararBala(balas, player);
					cooldown = CADENCIA_DISPARO;
					proyectiles++;
					if (sfx_disparo) {
						al_play_sample(sfx_disparo, 0.3, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
					}
				}
				
				// Actualizar entidades (pausar si jugador muere)
				if (player.activo) {
					actualizarEnemigos(enemigos, player, ancho, alto);
					actualizarBalas(balas);
				}
				
				// Detectar colisiones balas-enemigos
				int muertos = verificarColisionesBalasEnemigos(balas, enemigos);
				if (muertos > 0) {
					kills += muertos;
					puntos += muertos * 100;  // 100 puntos por enemigo
					if (sfx_explosion) al_play_sample(sfx_explosion, 0.5, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
				}
				
				// Limpiar muertos
				limpiarBalas(balas);
				limpiarEnemigosInactivos(enemigos);
				
				// Nueva ronda si no quedan enemigos
				if (contarEnemigosActivos(enemigos) == 0 && player.activo) {
					estado = CAMBIO_RONDA;
					timer_trans = DURACION_TRANSICION;
					ronda++;
					liberarBalas(balas);
					balas = NULL;
					resetearJugador(player, ancho, alto);
				}
				
				// Detectar colisión jugador-enemigos
				if (verificarColisionJugadorEnemigos(player, enemigos)) {
					player.activo = false;
					delay_muerte = 120.0f;  // 2 segundos de pausa
					if (sfx_muerte) al_play_sample(sfx_muerte, 0.7, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
				}
				
				// Esperar antes de mostrar Game Over
				if (!player.activo && delay_muerte > 0.0f) {
					delay_muerte -= 1.0f;
					if (delay_muerte <= 0.0f) {
						estado = GAME_OVER;
						tocarMusica(musica_gameover, 0.6f);
					}
				}
				
				// Física del jugador (estilo Asteroids)
				if (player.activo) {
					// Rotación
					if (A) player.ang -= ROTACION;
					if (D) player.ang += ROTACION;
					
					// Impulso (W)
					if (W) {
						float fx = sin(player.ang);
						float fy = -cos(player.ang);
						player.vx += fx * ACELERACION;
						player.vy += fy * ACELERACION;
					}
					
					// Fricción
					player.vx *= ROZAMIENTO;
					player.vy *= ROZAMIENTO;
					
					// Limitar velocidad máxima
					float vel = player.vx * player.vx + player.vy * player.vy;
					if (vel > VELOCIDAD_MAX * VELOCIDAD_MAX) {
						float factor = VELOCIDAD_MAX / sqrt(vel);
						player.vx *= factor;
						player.vy *= factor;
					}
					
					// Aplicar movimiento
					player.x += player.vx;
					player.y += player.vy;
					
					// Limitar a bordes de pantalla
					if (player.x < 25) player.x = 25;
					if (player.x >= ancho - 25) player.x = ancho - 25;
					if (player.y < 25) player.y = 25;
					if (player.y >= alto - 25) player.y = alto - 25;
				}
			}
			
			// Transición entre rondas
			if (estado == CAMBIO_RONDA) {
				timer_trans -= 1.0f;
				if (timer_trans <= 0.0f) {
					generarOleada(enemigos, ronda, ancho, alto);
					estado = JUGANDO;
				}
			}
			
			// RENDERIZADO
			al_clear_to_color(al_map_rgb(0, 0, 0));
			
			if (estado == JUGANDO) {
				// Dibujar fondo
				if (fondo_gameplay) {
					al_draw_scaled_bitmap(fondo_gameplay, 0, 0, al_get_bitmap_width(fondo_gameplay), al_get_bitmap_height(fondo_gameplay), 0, 0, ancho, alto, 0);
				}
				
				// Dibujar jugador (rombo azul)
				if (player.activo) {
					ALLEGRO_TRANSFORM guardado, t;
					al_copy_transform(&guardado, al_get_current_transform());
					al_identity_transform(&t);
					al_rotate_transform(&t, player.ang);
					al_translate_transform(&t, player.x, player.y);
					al_use_transform(&t);
					
					al_draw_filled_polygon(Puntos_jugador, 4, al_map_rgb(60, 180, 255));
					al_draw_polygon(Puntos_jugador, 4, ALLEGRO_LINE_JOIN_ROUND, al_map_rgb(255, 255, 255), 1.5f, 1.0f);
					
					al_use_transform(&guardado);
				}
				
				// Dibujar enemigos
				PtrNave e = enemigos;
				while (e != NULL) {
					if (e->activo) {
						if (e->tipo == 1) {
							// Drone: círculos verdes concéntricos
							al_draw_circle(e->x, e->y, 50.0f, al_map_rgb(170, 255, 170), 2);
							al_draw_circle(e->x, e->y, 45.0f, al_map_rgb(170, 255, 170), 3);
						} else if (e->tipo == 2) {
							// Seeker: triángulo rosa que apunta al jugador
							ALLEGRO_TRANSFORM old, Ts;
							al_copy_transform(&old, al_get_current_transform());
							al_identity_transform(&Ts);
							
							float ang = atan2f(player.y - e->y, player.x - e->x) + 3.14159f / 2.0f;
							al_rotate_transform(&Ts, ang);
							al_translate_transform(&Ts, e->x, e->y);
							al_use_transform(&Ts);
							
							al_draw_triangle(v[0], v[1], v[2], v[3], v[4], v[5], al_map_rgb(255, 100, 220), 6);
							al_draw_triangle(v[0], v[1], v[2], v[3], v[4], v[5], al_map_rgb(255, 255, 255), 3);
							al_draw_triangle(v[0], v[1] + 20, v[2] + 15, v[3] - 10, v[4] - 15, v[5] - 10, al_map_rgb(255, 100, 220), 3);
							al_draw_triangle(v[0], v[1] + 20, v[2] + 15, v[3] - 10, v[4] - 15, v[5] - 10, al_map_rgb(255, 255, 255), 1);
							
							al_use_transform(&old);
						}
					}
					e = e->siguiente;
				}
				
				// Dibujar balas (círculos amarillos)
				PtrBala b = balas;
				while (b != NULL) {
					if (b->activa) {
						al_draw_filled_circle(b->x, b->y, 5.0f, al_map_rgb(255, 255, 0));
					}
				 b = b->siguiente;
				}
				
				// HUD (arriba izquierda)
				al_draw_textf(font, al_map_rgb(255, 255, 255), 10, 10, ALLEGRO_ALIGN_LEFT, "PUNTUACION: %d", puntos);
				al_draw_textf(font, al_map_rgb(255, 255, 255), 10, 35, ALLEGRO_ALIGN_LEFT, "RONDA: %d", ronda);
				al_draw_textf(font, al_map_rgb(255, 255, 255), 10, 60, ALLEGRO_ALIGN_LEFT, "TIEMPO: %.1f", tiempo);
			}
			
			// Pantalla de transición entre rondas
			if (estado == CAMBIO_RONDA) {
				float progreso = 1.0f - (timer_trans / 180.0f);
				// Fade in/out
				float fade = (progreso < 0.3f) ? (progreso / 0.3f) : ((progreso > 0.7f) ? ((1.0f - progreso) / 0.3f) : 1.0f);
				
				char txt[50];
				sprintf_s(txt, 50, "RONDA %d", ronda);
				
				al_draw_text(font, al_map_rgba_f(fade, fade, 0, fade), ancho / 2, alto / 2 - 50, ALLEGRO_ALIGN_CENTER, txt);
				al_draw_text(font, al_map_rgba_f(0.8f * fade, 0.8f * fade, 0.8f * fade, fade), ancho / 2, alto / 2, ALLEGRO_ALIGN_CENTER, "Preparate...");
			}
			
			// Pantalla de Game Over
			if (estado == GAME_OVER) {
				al_draw_text(font, al_map_rgb(255, 0, 0), ancho / 2, alto / 2 - 200, ALLEGRO_ALIGN_CENTER, "GAME OVER");
				al_draw_textf(font, al_map_rgb(255, 255, 255), ancho / 2, alto / 2 - 140, ALLEGRO_ALIGN_CENTER, "Puntuacion Final: %d", puntos);
				al_draw_textf(font, al_map_rgb(255, 255, 255), ancho / 2, alto / 2 - 110, ALLEGRO_ALIGN_CENTER, "Tiempo: %.1f segundos", tiempo);
				al_draw_textf(font, al_map_rgb(255, 255, 255), ancho / 2, alto / 2 - 80, ALLEGRO_ALIGN_CENTER, "Enemigos Eliminados: %d", kills);
				al_draw_textf(font, al_map_rgb(255, 255, 255), ancho / 2, alto / 2 - 50, ALLEGRO_ALIGN_CENTER, "Ronda Alcanzada: %d", ronda);
				al_draw_text(font, al_map_rgb(150, 150, 150), ancho / 2, alto / 2, ALLEGRO_ALIGN_CENTER, "Presiona ENTER para continuar...");
			}
			
			// Pantalla de captura de nombre
			if (estado == INPUT_NOMBRE) {
				al_draw_text(font, al_map_rgb(255, 255, 0), ancho / 2, alto / 2 - 250, ALLEGRO_ALIGN_CENTER, "NUEVA PUNTUACION!");
				al_draw_textf(font, al_map_rgb(255, 255, 255), ancho / 2, alto / 2 - 200, ALLEGRO_ALIGN_CENTER, "Puntuacion: %d", puntos);
				al_draw_text(font, al_map_rgb(200, 200, 200), ancho / 2, alto / 2 - 140, ALLEGRO_ALIGN_CENTER, "Ingresa tu nombre:");
				
				// Cursor parpadeante
				bool mostrar = ((int)(tiempo_total * 2.0f)) % 2 == 0;
				string txt = nombre;
				if (mostrar) txt += "_";
				if (txt.empty()) txt = "_";
				
				al_draw_text(font, al_map_rgb(255, 255, 255), ancho / 2, alto / 2 - 90, ALLEGRO_ALIGN_CENTER, txt.c_str());
				al_draw_text(font, al_map_rgb(150, 150, 150), ancho / 2, alto / 2 - 40, ALLEGRO_ALIGN_CENTER, "Presiona ENTER para confirmar");
				al_draw_text(font, al_map_rgb(150, 150, 150), ancho / 2, alto / 2 - 10, ALLEGRO_ALIGN_CENTER, "Presiona BACKSPACE para borrar");
				
				// Mostrar Top 5
				al_draw_text(font, al_map_rgb(255, 255, 0), ancho / 2, alto / 2 + 40, ALLEGRO_ALIGN_CENTER, "=== TOP 5 ===");
				
				vector<Estadistica> top5 = obtenerTop5();
				int y = 75;
				for (size_t i = 0; i < top5.size(); i++) {
					char linea[100];
					sprintf_s(linea, 100, "%d. %s - %d pts (Ronda %d)", (int)(i + 1), top5[i].nombre.c_str(), top5[i].puntuacion, top5[i].ronda);
					
					// Colores del podio
					ALLEGRO_COLOR col;
					if (i == 0) col = al_map_rgb(255, 215, 0);  // Oro
					else if (i == 1) col = al_map_rgb(192, 192, 192);  // Plata
					else if (i == 2) col = al_map_rgb(205, 127, 50);  // Bronce
					else col = al_map_rgb(200, 200, 200);
					
					al_draw_text(font, col, ancho / 2, alto / 2 + y, ALLEGRO_ALIGN_CENTER, linea);
				 y += 25;
				}
			}
			
			al_flip_display();
		}
		
		// Cerrar ventana
		if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			jugando = false;
		}
	}
	
	// Limpieza de recursos
	liberarEnemigos(enemigos);
	liberarBalas(balas);
}
