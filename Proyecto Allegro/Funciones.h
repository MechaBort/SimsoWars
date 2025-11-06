/*
 * FUNCIONES.H
 * -----------
 * Estructuras, constantes y funciones auxiliares del juego
 */

#pragma once

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

using namespace std;

// ========== ENUMS ==========

enum EstadoJuego {
	JUGANDO,
	CAMBIO_RONDA,
	GAME_OVER,
	INPUT_NOMBRE
};

// ========== ESTRUCTURAS ==========

typedef struct Nave {
	float x, y;
	float vx, vy;
	float ang;
	float radio;
	bool activo;
	int tipo;
	struct Nave* siguiente;
} *PtrNave;

typedef struct Bala {
	float x, y;
	float vx, vy;
	bool activa;
	float tiempo_vida;
	struct Bala* siguiente;
} *PtrBala;

struct Estadistica {
	string nombre;
	int puntuacion;
	float tiempo;
	int ronda;
	int enemigos_eliminados;
	int proyectiles_disparados;
};

// ========== CONSTANTES ==========

const float RADIO_JUGADOR = 30.0f;
const float RADIO_DRONE = 50.0f;
const float RADIO_SEEKER = 45.0f;
const float RADIO_BALA = 5.0f;
const float VELOCIDAD_BALA = 15.0f;
const float VIDA_BALA = 180.0f;
const float CADENCIA_DISPARO = 10.0f;

const int ENEMIGOS_RONDA_INICIAL = 3;
const int INCREMENTO_POR_RONDA = 2;
const float DURACION_TRANSICION = 180.0f;

// ========== INICIALIZACIÓN ==========

// Coloca al jugador en el centro de la pantalla
void iniciarPersonaje(Nave& personaje, int x, int y) {
	personaje.x = x / 2.0f;
	personaje.y = y / 2.0f;
	personaje.vx = 0.0f;
	personaje.vy = 0.0f;
	personaje.ang = 0.0f;
	personaje.radio = RADIO_JUGADOR;
	personaje.activo = true;
	personaje.tipo = 0;  // Tipo: Jugador
	personaje.siguiente = nullptr;
}

// Reinicia la posición del jugador (entre rondas)
void resetearJugador(Nave& personaje, int x, int y) {
	personaje.x = x / 2.0f;
	personaje.y = y / 2.0f;
	personaje.vx = 0.0f;
	personaje.vy = 0.0f;
	personaje.ang = 0.0f;
}

// Crea un Drone (verde) en un borde aleatorio de la pantalla
void iniciarWandererAleatorio(Nave& monstruo, int anchoMax, int altoMax) {
	int lado = rand() % 4;  // 0=arriba, 1=derecha, 2=abajo, 3=izquierda
	
	switch (lado) {
		case 0: monstruo.x = rand() % anchoMax; monstruo.y = 100; break;
		case 1: monstruo.x = anchoMax - 100; monstruo.y = rand() % altoMax; break;
		case 2: monstruo.x = rand() % anchoMax; monstruo.y = altoMax - 100; break;
		case 3: monstruo.x = 100; monstruo.y = rand() % altoMax; break;
	}
	
	// Velocidad aleatoria en X e Y
	monstruo.vx = (rand() % 10 + 5) * (rand() % 2 == 0 ? 1 : -1);
	monstruo.vy = (rand() % 10 + 5) * (rand() % 2 == 0 ? 1 : -1);
	monstruo.ang = 0.0f;
	monstruo.radio = RADIO_DRONE;
	monstruo.activo = true;
	monstruo.tipo = 1;  // Tipo: Drone
	monstruo.siguiente = nullptr;
}

// Crea un Seeker (rosa) en un borde aleatorio de la pantalla
void iniciarSeekerAleatorio(Nave& monstruo, int anchoMax, int altoMax) {
	int lado = rand() % 4;
	
	switch (lado) {
		case 0: monstruo.x = rand() % anchoMax; monstruo.y = 100; break;
		case 1: monstruo.x = anchoMax - 100; monstruo.y = rand() % altoMax; break;
		case 2: monstruo.x = rand() % anchoMax; monstruo.y = altoMax - 100; break;
		case 3: monstruo.x = 100; monstruo.y = rand() % altoMax; break;
	}
	
	monstruo.vx = 5.0f;
	monstruo.vy = 5.0f;
	monstruo.ang = 0.0f;
	monstruo.radio = RADIO_SEEKER;
	monstruo.activo = true;
	monstruo.tipo = 2;  // Tipo: Seeker
	monstruo.siguiente = nullptr;
}

// ========== MOVIMIENTO ==========

// Mueve un Drone (rebota en los bordes como pelota)
void movimientoWanderer(Nave& monstruo, int anchoMax, int altoMax) {
	if (!monstruo.activo) return;
	
	int izq = 50, der = anchoMax - 50, arr = 50, aba = altoMax - 50;
	
	// Invertir velocidad al chocar con bordes
	if (monstruo.x <= izq || monstruo.x >= der) monstruo.vx = -monstruo.vx;
	if (monstruo.y <= arr || monstruo.y >= aba) monstruo.vy = -monstruo.vy;
	
	// Aplicar movimiento
	monstruo.x += monstruo.vx;
	monstruo.y += monstruo.vy;
	
	// Forzar dentro de límites
	if (monstruo.x < izq) monstruo.x = izq;
	if (monstruo.x > der) monstruo.x = der;
	if (monstruo.y < arr) monstruo.y = arr;
	if (monstruo.y > aba) monstruo.y = aba;
}

// Mueve un Seeker (persigue al jugador)
void movimientoSeeker(Nave& monstruo, Nave& jugador, int anchoMax, int altoMax) {
	if (!monstruo.activo) return;
	
	// Calcular vector hacia el jugador
	float dx = jugador.x - monstruo.x;
	float dy = jugador.y - monstruo.y;
	float d = sqrt(dx * dx + dy * dy);  // Distancia
	
	if (d == 0.0f) return;  // Evitar división por cero
	
	// Normalizar y mover hacia el jugador (6 px/frame)
	monstruo.x += (dx / d) * 6.0f;
	monstruo.y += (dy / d) * 6.0f;
	
	// Limitar a bordes
	if (monstruo.x < 50) monstruo.x = 50;
	if (monstruo.x > anchoMax - 50) monstruo.x = anchoMax - 50;
	if (monstruo.y < 50) monstruo.y = 50;
	if (monstruo.y > altoMax - 50) monstruo.y = altoMax - 50;
}

// ========== LISTAS ENLAZADAS - ENEMIGOS ==========

// Agrega un enemigo al final de la lista
void agregarEnemigo(PtrNave& cabeza, Nave nuevoEnemigo) {
	PtrNave nuevo = new Nave;
	*nuevo = nuevoEnemigo;
	nuevo->siguiente = nullptr;
	
	if (cabeza == nullptr) {
		cabeza = nuevo;  // Primer elemento
	} else {
		// Buscar último elemento
		PtrNave temp = cabeza;
		while (temp->siguiente != nullptr) temp = temp->siguiente;
		temp->siguiente = nuevo;
	}
}

// Actualiza todos los enemigos (llama a su movimiento según tipo)
void actualizarEnemigos(PtrNave cabeza, Nave& jugador, int anchoMax, int altoMax) {
	PtrNave temp = cabeza;
	while (temp != nullptr) {
		if (temp->activo) {
			if (temp->tipo == 1) movimientoWanderer(*temp, anchoMax, altoMax);  // Drone
			else if (temp->tipo == 2) movimientoSeeker(*temp, jugador, anchoMax, altoMax);  // Seeker
		}
		temp = temp->siguiente;
	}
}

// Cuenta cuántos enemigos están vivos
int contarEnemigosActivos(PtrNave cabeza) {
	int count = 0;
	PtrNave temp = cabeza;
	while (temp != nullptr) {
		if (temp->activo) count++;
		temp = temp->siguiente;
	}
	return count;
}

// Libera la memoria de toda la lista de enemigos
void liberarEnemigos(PtrNave& cabeza) {
	while (cabeza != nullptr) {
		PtrNave temp = cabeza;
		cabeza = cabeza->siguiente;
		delete temp;
	}
}

// ========== LISTAS ENLAZADAS - BALAS ==========

// Agrega una bala al final de la lista
void agregarBala(PtrBala& cabeza, Bala nuevaBala) {
	PtrBala nueva = new Bala;
	*nueva = nuevaBala;
	nueva->siguiente = nullptr;
	
	if (cabeza == nullptr) {
		cabeza = nueva;
	} else {
		PtrBala temp = cabeza;
		while (temp->siguiente != nullptr) temp = temp->siguiente;
		temp->siguiente = nueva;
	}
}

// Mueve todas las balas y reduce su tiempo de vida
void actualizarBalas(PtrBala& cabeza) {
	PtrBala temp = cabeza;
	while (temp != nullptr) {
		if (temp->activa) {
			temp->x += temp->vx;  // Mover en X
			temp->y += temp->vy;  // Mover en Y
			temp->tiempo_vida -= 1.0f;  // Reducir TTL
			if (temp->tiempo_vida <= 0.0f) temp->activa = false;  // Desactivar si expiró
		}
		temp = temp->siguiente;
	}
}

// Elimina las balas inactivas de la lista (libera memoria)
void limpiarBalas(PtrBala& cabeza) {
	// Eliminar al inicio
	while (cabeza != nullptr && !cabeza->activa) {
		PtrBala temp = cabeza;
		cabeza = cabeza->siguiente;
		delete temp;
	}
	
	// Eliminar en medio/final
	if (cabeza != nullptr) {
		PtrBala actual = cabeza;
		while (actual->siguiente != nullptr) {
			if (!actual->siguiente->activa) {
				PtrBala temp = actual->siguiente;
				actual->siguiente = temp->siguiente;
				delete temp;
			} else {
				actual = actual->siguiente;
			}
		}
	}
}

// Libera la memoria de toda la lista de balas
void liberarBalas(PtrBala& cabeza) {
	while (cabeza != nullptr) {
		PtrBala temp = cabeza;
		cabeza = cabeza->siguiente;
		delete temp;
	}
}

// Crea una bala desde la posición del jugador
void dispararBala(PtrBala& cabeza, Nave& jugador) {
	Bala nueva;
	// Posición: adelante de la nave
	nueva.x = jugador.x + sin(jugador.ang) * 30.0f;
	nueva.y = jugador.y - cos(jugador.ang) * 30.0f;
	// Velocidad: en la dirección que apunta la nave
	nueva.vx = sin(jugador.ang) * VELOCIDAD_BALA;
	nueva.vy = -cos(jugador.ang) * VELOCIDAD_BALA;
	nueva.activa = true;
	nueva.tiempo_vida = VIDA_BALA;
	nueva.siguiente = nullptr;
	agregarBala(cabeza, nueva);
}

// ========== COLISIONES ==========

// Detecta si dos círculos colisionan (distancia < suma de radios)
bool hayColision(float x1, float y1, float r1, float x2, float y2, float r2) {
	float dx = x2 - x1;
	float dy = y2 - y1;
	float distancia = sqrt(dx * dx + dy * dy);
	return distancia < (r1 + r2);
}

// Revisa colisiones entre balas y enemigos, retorna cantidad de muertos
int verificarColisionesBalasEnemigos(PtrBala balas, PtrNave enemigos) {
	int muertos = 0;
	PtrBala bala = balas;
	
	// Por cada bala activa
	while (bala != nullptr) {
		if (bala->activa) {
			PtrNave enemigo = enemigos;
			// Revisar contra cada enemigo
			while (enemigo != nullptr) {
				if (enemigo->activo) {
					if (hayColision(bala->x, bala->y, RADIO_BALA, enemigo->x, enemigo->y, enemigo->radio)) {
						bala->activa = false;  // Destruir bala
						enemigo->activo = false;  // Destruir enemigo
						muertos++;
					}
				}
				enemigo = enemigo->siguiente;
			}
		}
		bala = bala->siguiente;
	}
	return muertos;
}

// Revisa si el jugador choca con algún enemigo
bool verificarColisionJugadorEnemigos(Nave& jugador, PtrNave enemigos) {
	if (!jugador.activo) return false;
	
	PtrNave enemigo = enemigos;
	while (enemigo != nullptr) {
		if (enemigo->activo) {
			if (hayColision(jugador.x, jugador.y, jugador.radio, enemigo->x, enemigo->y, enemigo->radio)) {
				return true;  // Colisión detectada
			}
		}
		enemigo = enemigo->siguiente;
	}
	return false;
}

// ========== OLEADAS ==========

// Calcula cuántos enemigos en una ronda: 3, 5, 7, 9...
int calcularEnemigosEnRonda(int numeroRonda) {
	return ENEMIGOS_RONDA_INICIAL + (numeroRonda - 1) * INCREMENTO_POR_RONDA;
}

// Genera una oleada de enemigos (60% Drones, 40% Seekers)
void generarOleada(PtrNave& lista_enemigos, int numeroRonda, int anchoMax, int altoMax) {
	int total = calcularEnemigosEnRonda(numeroRonda);
	int drones = (total * 60) / 100;
	int seekers = total - drones;
	
	// Crear Drones
	for (int i = 0; i < drones; i++) {
		Nave drone;
		iniciarWandererAleatorio(drone, anchoMax, altoMax);
		agregarEnemigo(lista_enemigos, drone);
	}
	
	// Crear Seekers
	for (int i = 0; i < seekers; i++) {
		Nave seeker;
		iniciarSeekerAleatorio(seeker, anchoMax, altoMax);
		agregarEnemigo(lista_enemigos, seeker);
	}
}

// Elimina enemigos muertos de la lista (libera memoria)
void limpiarEnemigosInactivos(PtrNave& cabeza) {
	// Eliminar al inicio
	while (cabeza != nullptr && !cabeza->activo) {
		PtrNave temp = cabeza;
		cabeza = cabeza->siguiente;
		delete temp;
	}
	
	// Eliminar en medio/final
	if (cabeza != nullptr) {
		PtrNave actual = cabeza;
		while (actual->siguiente != nullptr) {
			if (!actual->siguiente->activo) {
				PtrNave temp = actual->siguiente;
				actual->siguiente = temp->siguiente;
				delete temp;
			} else {
				actual = actual->siguiente;
			}
		}
	}
}

// ========== PERSISTENCIA ==========

// Guarda las estadísticas de la partida en archivo
void guardarEstadisticas(const Estadistica& stats) {
	ofstream archivo("estadisticas.txt", ios::app);
	if (archivo.is_open()) {
		// Formato: nombre|puntos|tiempo|ronda|enemigos|disparos
		archivo << stats.nombre << "|" << stats.puntuacion << "|" << stats.tiempo << "|" << stats.ronda << "|" << stats.enemigos_eliminados << "|" << stats.proyectiles_disparados << "\n";
		archivo.close();
	}
}

// Lee todas las estadísticas del archivo y las ordena por puntuación
vector<Estadistica> leerEstadisticas() {
	vector<Estadistica> lista;
	ifstream archivo("estadisticas.txt");
	
	if (archivo.is_open()) {
		string linea;
		while (getline(archivo, linea)) {
			Estadistica stat;
			// Buscar posiciones de los delimitadores '|'
			size_t pos1 = linea.find('|');
			size_t pos2 = linea.find('|', pos1 + 1);
			size_t pos3 = linea.find('|', pos2 + 1);
			size_t pos4 = linea.find('|', pos3 + 1);
			size_t pos5 = linea.find('|', pos4 + 1);
			
			if (pos1 != string::npos && pos2 != string::npos && pos3 != string::npos && pos4 != string::npos) {
				// Extraer cada campo
				stat.nombre = linea.substr(0, pos1);
				stat.puntuacion = stoi(linea.substr(pos1 + 1, pos2 - pos1 - 1));
				stat.tiempo = stof(linea.substr(pos2 + 1, pos3 - pos2 - 1));
				stat.ronda = stoi(linea.substr(pos3 + 1, pos4 - pos3 - 1));
				
				string enemigos_str = (pos5 != string::npos) ? linea.substr(pos4 + 1, pos5 - pos4 - 1) : linea.substr(pos4 + 1);
				stat.enemigos_eliminados = stoi(enemigos_str);
				
				stat.proyectiles_disparados = (pos5 != string::npos) ? stoi(linea.substr(pos5 + 1)) : 0;
				
				lista.push_back(stat);
			}
		}
		archivo.close();
	}
	
	// Ordenar de mayor a menor puntuación
	sort(lista.begin(), lista.end(), [](const Estadistica& a, const Estadistica& b) {
		return a.puntuacion > b.puntuacion;
	});
	
	return lista;
}

// Obtiene las 5 mejores puntuaciones
vector<Estadistica> obtenerTop5() {
	vector<Estadistica> todas = leerEstadisticas();
	vector<Estadistica> top5;
	int limite = (todas.size() < 5) ? todas.size() : 5;
	for (int i = 0; i < limite; i++) {
		top5.push_back(todas[i]);
	}
	return top5;
}

// ========== AUDIO ==========

ALLEGRO_SAMPLE* musica_menu = NULL;
ALLEGRO_SAMPLE* musica_gameplay = NULL;
ALLEGRO_SAMPLE* musica_gameover = NULL;
ALLEGRO_SAMPLE* sfx_disparo = NULL;
ALLEGRO_SAMPLE* sfx_explosion = NULL;
ALLEGRO_SAMPLE* sfx_muerte = NULL;

ALLEGRO_SAMPLE_ID id_musica_actual;
bool hay_musica_sonando = false;

// Carga todos los archivos de audio
void cargarAudio() {
	musica_menu = al_load_sample("musica/Menu.ogg");
	musica_gameplay = al_load_sample("musica/fight.ogg");
	sfx_disparo = al_load_sample("musica/shoot.wav");
	sfx_explosion = al_load_sample("musica/enemyexp.wav");
	sfx_muerte = al_load_sample("musica/playerexp.flac");
}

// Reproduce una música en loop (detiene la anterior si existe)
void tocarMusica(ALLEGRO_SAMPLE* musica, float volumen) {
	if (hay_musica_sonando) al_stop_sample(&id_musica_actual);
	if (musica) hay_musica_sonando = al_play_sample(musica, volumen, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, &id_musica_actual);
}

// Detiene la música actual
void pararMusica() {
	if (hay_musica_sonando) {
		al_stop_sample(&id_musica_actual);
		hay_musica_sonando = false;
	}
}

// Reproduce un efecto de sonido (NULL permite múltiples instancias)
void tocarSonido(ALLEGRO_SAMPLE* sonido, float volumen) {
	if (sonido) al_play_sample(sonido, volumen, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
}

// Libera la memoria de todos los archivos de audio
void limpiarAudio() {
	pararMusica();
	if (musica_menu) al_destroy_sample(musica_menu);
	if (musica_gameplay) al_destroy_sample(musica_gameplay);
	if (sfx_disparo) al_destroy_sample(sfx_disparo);
	if (sfx_explosion) al_destroy_sample(sfx_explosion);
	if (sfx_muerte) al_destroy_sample(sfx_muerte);
}