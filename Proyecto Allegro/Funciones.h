

// Se incluyen las librerías
#include <stdio.h>
#include <iostream>
#include <math.h>

#include <allegro5/allegro.h>

using namespace std;

// Se crea la estructura bicho
struct Nave {

	float x;
	float y;
	float vx;
	float vy;
	float ang;
	float rozamiento;
	float accel;
	float velocidad_max;

};

// Esta función inicializa al personaje en su posición inicial
void iniciarPersonaje(Nave& personaje, int x, int y) {

	
	personaje.x = x / 2;
	personaje.y = y / 2;
	personaje.vx = 0.0f;
	personaje.vy = 0.0f;
	personaje.ang = 0.0f;   // 0 rad con modelo apuntando a -Y

}

// Esta función inicializa al monstruo en su posición inicial y le asigna una velocidad
void iniciarMonstruo(Nave& monstruo, int x, int y) {

	monstruo.x = x - 300;
	monstruo.y = y - 200;
	monstruo.vx = 5;
	monstruo.vy = 5;

}

// Esta función controla el movimiento del monstruo y lo restringe a moverse más allá de los bordes asignados
void movimientoWanderer(Nave& monstruo) {

	// Se obtiene el tamaño del monitor
	ALLEGRO_MONITOR_INFO info;
	al_get_monitor_info(0, &info);
	const int X = info.x2 - info.x1;
	const int Y = info.y2 - info.y1;

	// Define los bordes izquierdo y derecho
	int borde_izquierdo = 155;
	int borde_derecho = X - 155;
	int borde_superior = 155;
	int borde_inferior = Y - 155;

	// Si el mounstruo llega a un borde, cambia su dirección
	if (monstruo.x <= borde_izquierdo || monstruo.x >= borde_derecho) {
		monstruo.vx = -monstruo.vx;
	}
	if (monstruo.y <= borde_superior || monstruo.y >= borde_inferior) {
		monstruo.vy = -monstruo.vy;
	}
	// Actualiza la posición del monstruo
	monstruo.x += monstruo.vx;
	monstruo.y += monstruo.vy;
}

void movimientoSeeker(Nave& monstruo, Nave& jugador) {

	
		float dx = jugador.x - monstruo.x, dy = jugador.y - monstruo.y;
		float d = sqrt(dx * dx + dy * dy);
		if (d == 0.0f) return;              // ya está encima
		float speed = 6.0f;                 // píxeles por tick
		monstruo.x += (dx / d) * speed;
		monstruo.y += (dy / d) * speed;
	}

	

	// Si el mounstruo llega a un borde, cambia su dirección
	//if (monstruo.x <= borde_izquierdo || monstruo.x >= borde_derecho) {
	//	monstruo.vx = -monstruo.vx;
	//}
	//if (monstruo.y <= borde_superior || monstruo.y >= borde_inferior) {
	//	monstruo.vy = -monstruo.vy;
	//}
	// Actualiza la posición del monstruo


