/*
 * ============================================================================
 * PROYECTO ALLEGRO - VECTOR ONSLAUGHT
 * ============================================================================
 * Juego de naves estilo Asteroids con oleadas de enemigos
 * 
 * Este archivo contiene:
 * - Inicialización de Allegro 5
 * - Bucle principal del programa
 * - Menú principal y navegación
 * - Pantalla de high scores
 * ============================================================================
 */

#include <stdio.h>
#include <cmath>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

#include "Funciones.h"
#include "juego.h"

using namespace std;

// ========== ENUMERACIONES ==========

enum OpcionMenu {
	MENU_JUGAR = 0,
	MENU_HIGH_SCORES = 1,
	MENU_SALIR = 2
};

enum EstadoApp {
	APP_MENU,
	APP_JUGANDO,
	APP_HIGH_SCORES
};

// ========== FUNCIONES DE RENDERIZADO ==========

// Dibuja el menú principal con opciones y fondo
void renderizarMenu(int opcion, ALLEGRO_FONT* fuente_grande, ALLEGRO_FONT* fuente_mediana, ALLEGRO_FONT* fuente_pequena, int ancho, int alto, float timer, ALLEGRO_BITMAP* fondo) {
	al_clear_to_color(al_map_rgb(0, 0, 0));

	// Dibujar fondo (si existe)
	if (fondo) {
		al_draw_scaled_bitmap(fondo, 0, 0, al_get_bitmap_width(fondo), al_get_bitmap_height(fondo), 0, 0, ancho, alto, 0);
	}

	// Título principal
	al_draw_text(fuente_grande, al_map_rgb(150, 150, 150), ancho / 2, alto / 2 - 250, ALLEGRO_ALIGN_CENTER, "VECTOR ONSLAUGHT");
	al_draw_text(fuente_mediana, al_map_rgb(150, 150, 150), ancho / 2, alto / 2 - 150, ALLEGRO_ALIGN_CENTER, "Survival Space Shooter");

	// Opciones del menú
	const char* texto[3] = {"JUGAR", "VER HIGH SCORES", "SALIR"};
	int y = alto / 2 - 50;
	
	for (int i = 0; i < 3; i++) {
		int yPos = y + (i * 60);
		// Color amarillo si está seleccionado, gris si no
		ALLEGRO_COLOR color = (i == opcion) ? al_map_rgb(255, 255, 0) : al_map_rgb(150, 150, 150);
		
		al_draw_text(fuente_mediana, color, ancho / 2, yPos, ALLEGRO_ALIGN_CENTER, texto[i]);
		
		// Flecha indicadora
		if (i == opcion) {
			al_draw_text(fuente_mediana, color, ancho / 2 - 120, yPos, ALLEGRO_ALIGN_CENTER, ">");
		}
	}

	// Instrucciones de control
	al_draw_text(fuente_pequena, al_map_rgb(100, 100, 100), ancho / 2, alto - 100, ALLEGRO_ALIGN_CENTER, "Usa W/S o Flechas para navegar");
	al_draw_text(fuente_pequena, al_map_rgb(100, 100, 100), ancho / 2, alto - 70, ALLEGRO_ALIGN_CENTER, "Presiona ENTER para seleccionar");

	al_flip_display();
}

// Muestra el Top 5 de mejores puntuaciones
void renderizarPantallaHighScores(ALLEGRO_FONT* fuente_grande, ALLEGRO_FONT* fuente_mediana, int ancho, int alto) {
	al_clear_to_color(al_map_rgb(0, 0, 0));
	al_draw_text(fuente_grande, al_map_rgb(255, 215, 0), ancho / 2, alto / 2 - 300, ALLEGRO_ALIGN_CENTER, "TOP 5 HIGH SCORES");

	vector<Estadistica> top5 = obtenerTop5();
	
	if (top5.empty()) {
		// No hay registros aún
		al_draw_text(fuente_mediana, al_map_rgb(150, 150, 150), ancho / 2, alto / 2, ALLEGRO_ALIGN_CENTER, "No hay puntuaciones registradas aun");
	} else {
		int y = -180;
		for (size_t i = 0; i < top5.size(); i++) {
			// Colores del podio (oro, plata, bronce, gris)
			ALLEGRO_COLOR color;
			if (i == 0) color = al_map_rgb(255, 215, 0);
			else if (i == 1) color = al_map_rgb(192, 192, 192);
			else if (i == 2) color = al_map_rgb(205, 127, 50);
			else color = al_map_rgb(200, 200, 200);

			// Línea 1: Posición y nombre
			char nombre[100];
			sprintf_s(nombre, 100, "%d. %s", (int)(i + 1), top5[i].nombre.c_str());
			al_draw_text(fuente_mediana, color, ancho / 2, alto / 2 + y, ALLEGRO_ALIGN_CENTER, nombre);
			
			// Línea 2: Estadísticas detalladas
			char datos[180];
			sprintf_s(datos, 180, "Puntos: %d | Ronda: %d | Tiempo: %.1f s | Enemigos: %d | Disparos: %d", top5[i].puntuacion, top5[i].ronda, top5[i].tiempo, top5[i].enemigos_eliminados, top5[i].proyectiles_disparados);
			al_draw_text(fuente_mediana, al_map_rgb(120, 120, 120), ancho / 2, alto / 2 + y + 30, ALLEGRO_ALIGN_CENTER, datos);
			
			y += 80;
		}
	}

	al_draw_text(fuente_mediana, al_map_rgb(150, 150, 150), ancho / 2, alto - 80, ALLEGRO_ALIGN_CENTER, "Presiona ESC para volver al menu");
	al_flip_display();
}

// ========== FUNCIÓN PRINCIPAL ==========

int main() {
	srand((unsigned int)time(NULL));

	// Inicializar Allegro y módulos
	if (!al_init()) {
		al_show_native_message_box(NULL, "Error", "Error", "No se pudo iniciar Allegro", NULL, 0);
		return -1;
	}
	
	al_init_font_addon();
	al_init_ttf_addon();
	al_init_image_addon();
	al_init_primitives_addon();
	al_install_keyboard();
	al_uninstall_mouse();  // No se usa mouse
	al_install_audio();
	al_init_acodec_addon();
	al_reserve_samples(16);  // 16 canales de audio (1 música + 15 efectos)

	// Obtener resolución del monitor
	ALLEGRO_MONITOR_INFO info;
	al_get_monitor_info(0, &info);
	int ancho = info.x2 - info.x1;
	int alto = info.y2 - info.y1;

	// Crear pantalla completa
	ALLEGRO_DISPLAY* pantalla = al_create_display(ancho, alto);
	if (!pantalla) {
		al_show_native_message_box(NULL, "Error", "Error", "No se pudo crear la pantalla", NULL, 0);
		return -1;
	}

	// Cargar fuentes (3 tamaños)
	ALLEGRO_FONT* font_grande = al_load_ttf_font("MONSTER.ttf", 72, 0);
	if (!font_grande) {
		al_show_native_message_box(pantalla, "Error", "Error", "No se pudo cargar la fuente grande", NULL, 0);
		return -1;
	}
	
	ALLEGRO_FONT* font_mediana = al_load_ttf_font("MONSTER.ttf", 24, 0);
	if (!font_mediana) {
		al_show_native_message_box(pantalla, "Error", "Error", "No se pudo cargar la fuente mediana", NULL, 0);
		return -1;
	}
	
	ALLEGRO_FONT* font_pequena = al_load_ttf_font("MONSTER.ttf", 16, 0);
	if (!font_pequena) {
		al_show_native_message_box(pantalla, "Error", "Error", "No se pudo cargar la fuente pequeña", NULL, 0);
		return -1;
	}

	// Cargar imágenes de fondo
	ALLEGRO_BITMAP* fondo_menu = al_load_bitmap("Imagenes/menu.png");
	if (!fondo_menu) {
		al_show_native_message_box(pantalla, "Advertencia", "Aviso", "No se pudo cargar la imagen de fondo del menu", NULL, ALLEGRO_MESSAGEBOX_WARN);
	}
	
	ALLEGRO_BITMAP* fondo_gameplay = al_load_bitmap("Imagenes/gameplay.png");
	if (!fondo_gameplay) {
		al_show_native_message_box(pantalla, "Advertencia", "Aviso", "No se pudo cargar la imagen de fondo del gameplay", NULL, ALLEGRO_MESSAGEBOX_WARN);
	}

	// Cargar audio e iniciar música del menú
	cargarAudio();
	tocarMusica(musica_menu, 0.5f);

	// Crear timer (60 FPS) y cola de eventos
	ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);
	ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();

	al_register_event_source(queue, al_get_keyboard_event_source());
	al_register_event_source(queue, al_get_display_event_source(pantalla));
	al_register_event_source(queue, al_get_timer_event_source(timer));

	// Variables del programa
	EstadoApp app = APP_MENU;  // Estado inicial
	int opcion = 0;  // Opción seleccionada (0-2)
	float timer_anim = 0.0f;  // Timer para animaciones

	// Bucle principal
	al_start_timer(timer);
	bool running = true;
	
	while (running) {
		ALLEGRO_EVENT ev;
		al_wait_for_event(queue, &ev);

		// Input del menú
		if (app == APP_MENU && ev.type == ALLEGRO_EVENT_KEY_DOWN) {
			if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
				running = false;  // Salir del programa
			}
			
			// Navegar hacia arriba
			if (ev.keyboard.keycode == ALLEGRO_KEY_W || ev.keyboard.keycode == ALLEGRO_KEY_UP) {
				opcion--;
				if (opcion < 0) opcion = 2;  // Wrap around
			}
			
			// Navegar hacia abajo
			if (ev.keyboard.keycode == ALLEGRO_KEY_S || ev.keyboard.keycode == ALLEGRO_KEY_DOWN) {
				opcion++;
				if (opcion >= 3) opcion = 0;  // Wrap around
			}
			
			// Seleccionar opción
			if (ev.keyboard.keycode == ALLEGRO_KEY_ENTER) {
				if (opcion == 0) {
					// JUGAR: Iniciar partida
					iniciarJuego(ancho, alto, font_mediana, timer, queue, fondo_gameplay);
					tocarMusica(musica_menu, 0.5f);  // Reanudar música del menú
				} else if (opcion == 1) {
					// VER HIGH SCORES
					app = APP_HIGH_SCORES;
				} else if (opcion == 2) {
					// SALIR
					running = false;
				}
			}
		}
		
		// Input de high scores
		if (app == APP_HIGH_SCORES && ev.type == ALLEGRO_EVENT_KEY_DOWN && ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
			app = APP_MENU;  // Volver al menú
		}

		// Renderizado (60 FPS)
		if (ev.type == ALLEGRO_EVENT_TIMER && ev.timer.source == timer) {
			timer_anim += 1.0f / 60.0f;

			if (app == APP_MENU) {
				renderizarMenu(opcion, font_grande, font_mediana, font_pequena, ancho, alto, timer_anim, fondo_menu);
			} else if (app == APP_HIGH_SCORES) {
				renderizarPantallaHighScores(font_grande, font_mediana, ancho, alto);
			}
		}

		// Cerrar ventana
		if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			running = false;
		}
	}

	// Limpieza de recursos
	limpiarAudio();
	if (fondo_menu) al_destroy_bitmap(fondo_menu);
	if (fondo_gameplay) al_destroy_bitmap(fondo_gameplay);
	al_destroy_font(font_grande);
	al_destroy_font(font_mediana);
	al_destroy_font(font_pequena);
	al_destroy_event_queue(queue);
	al_destroy_timer(timer);
	al_destroy_display(pantalla);
	
	return 0;
}