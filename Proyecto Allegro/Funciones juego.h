

// Se incluyen las librerías
#include <stdio.h>
#include <iostream>

#include <allegro5/allegro.h>

using namespace std;

// Se crea la estructura bicho
struct bicho {

	float x;
	float y;
	float vx;
	float vy;
	float rotacion;
	float rozamiento;
	float accel;
	float velocidad_max;

};

// Esta función inicializa al personaje en su posición inicial
void iniciarPersonaje(bicho& personaje, int x, int y) {

	personaje.x = 25;
	personaje.y = y - 40;
	personaje.rotacion = 0;
	personaje.vx = 0;
	personaje.vy = 0;
	personaje.accel = 0;
	personaje.rozamiento = 0;
	personaje.velocidad_max = 5;

}

// Esta función inicializa al monstruo en su posición inicial y le asigna una velocidad
void iniciarMonstruo(bicho& monstruo, int x, int y) {

	monstruo.x = x / 2;
	monstruo.y = y / 2;
	monstruo.vx = 0;
	monstruo.vy = 0;

}

// Esta función controla el movimiento del monstruo y lo restringe a moverse más allá de los bordes asignados
void movimientoMonstruo(bicho& monstruo) {

	// Se obtiene el tamaño del monitor
	ALLEGRO_MONITOR_INFO info;
	al_get_monitor_info(0, &info);
	const int X = info.x2 - info.x1;

	// Define los bordes izquierdo y derecho
	int borde_izquierdo = 155;
	int borde_derecho = X - 155;

	// Si el mounstruo llega a un borde, cambia su dirección
	if (monstruo.x <= borde_izquierdo || monstruo.x >= borde_derecho) {
		monstruo.vx = -monstruo.vx;
	}
	// Actualiza la posición del monstruo
	monstruo.x += monstruo.vx;
}