#pragma once

// ============================================================================
// FUNCIONES.H - Vector Onslaught
// Sistema de estructuras, listas enlazadas, hitbox y colisiones
// ============================================================================

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <allegro5/allegro.h>

// ============================================================================
// ENUMS
// ============================================================================

enum EstadoJuego {
    JUGANDO,
    CAMBIO_RONDA,
    GAME_OVER
};

// ============================================================================
// STRUCTS PRINCIPALES
// ============================================================================

// Estructura para las naves (jugador y enemigos)
typedef struct Nave {
    float x;                    
    float y;
    float vx;
    float vy;
    float ang;
    float radio;           // Radio para hitbox circular
    bool activo;           // Si está vivo/activo
    int tipo;              // 0=jugador, 1=drone, 2=seeker
    struct Nave* siguiente; // Para lista enlazada
} *PtrNave;

// Estructura para las balas
typedef struct Bala {
    float x;
    float y;
    float vx;
    float vy;
    bool activa;
    float tiempo_vida;     // Frames restantes antes de desaparecer
    struct Bala* siguiente; // Para lista enlazada
} *PtrBala;

// ============================================================================
// CONSTANTES DE JUEGO
// ============================================================================

const float RADIO_JUGADOR = 30.0f;
const float RADIO_DRONE = 50.0f;
const float RADIO_SEEKER = 45.0f;
const float RADIO_BALA = 5.0f;
const float VELOCIDAD_BALA = 15.0f;
const float VIDA_BALA = 120.0f;  // 2 segundos a 60 FPS
const float CADENCIA_DISPARO = 10.0f; // Frames entre disparos

// Constantes de oleadas
const int ENEMIGOS_RONDA_INICIAL = 3;
const int INCREMENTO_POR_RONDA = 2;

// Constantes de transición
const float DURACION_TRANSICION = 180.0f; // 3 segundos a 60 FPS

// ============================================================================
// FUNCIONES DE INICIALIZACIÓN
// ============================================================================

// Inicializa al jugador en el centro de la pantalla
void iniciarPersonaje(Nave& personaje, int x, int y) {
    personaje.x = x / 2.0f;
    personaje.y = y / 2.0f;
    personaje.vx = 0.0f;
    personaje.vy = 0.0f;
    personaje.ang = 0.0f;
    personaje.radio = RADIO_JUGADOR;
    personaje.activo = true;
    personaje.tipo = 0; // jugador
    personaje.siguiente = nullptr;
}

// Resetear jugador (mantiene activo, resetea posición y velocidad)
void resetearJugador(Nave& personaje, int x, int y) {
    personaje.x = x / 2.0f;
    personaje.y = y / 2.0f;
    personaje.vx = 0.0f;
    personaje.vy = 0.0f;
    personaje.ang = 0.0f;
    // Mantiene: radio, activo, tipo, siguiente
}

// Inicializa un enemigo tipo Wanderer (Drone) en posición aleatoria del borde
void iniciarWandererAleatorio(Nave& monstruo, int anchoMax, int altoMax) {
    // Decidir en qué borde aparece (0=arriba, 1=derecha, 2=abajo, 3=izquierda)
    int lado = rand() % 4;
    
    switch (lado) {
        case 0: // Arriba
            monstruo.x = rand() % anchoMax;
            monstruo.y = 100;
            break;
        case 1: // Derecha
            monstruo.x = anchoMax - 100;
            monstruo.y = rand() % altoMax;
            break;
        case 2: // Abajo
            monstruo.x = rand() % anchoMax;
            monstruo.y = altoMax - 100;
            break;
        case 3: // Izquierda
            monstruo.x = 100;
            monstruo.y = rand() % altoMax;
            break;
    }
    
    // Velocidad aleatoria
    monstruo.vx = (rand() % 10 + 5) * (rand() % 2 == 0 ? 1 : -1);
    monstruo.vy = (rand() % 10 + 5) * (rand() % 2 == 0 ? 1 : -1);
    monstruo.ang = 0.0f;
    monstruo.radio = RADIO_DRONE;
    monstruo.activo = true;
    monstruo.tipo = 1; // drone
    monstruo.siguiente = nullptr;
}

// Inicializa un enemigo tipo Seeker en posición aleatoria del borde
void iniciarSeekerAleatorio(Nave& monstruo, int anchoMax, int altoMax) {
    // Decidir en qué borde aparece
    int lado = rand() % 4;
    
    switch (lado) {
        case 0: // Arriba
            monstruo.x = rand() % anchoMax;
            monstruo.y = 100;
            break;
        case 1: // Derecha
            monstruo.x = anchoMax - 100;
            monstruo.y = rand() % altoMax;
            break;
        case 2: // Abajo
            monstruo.x = rand() % anchoMax;
            monstruo.y = altoMax - 100;
            break;
        case 3: // Izquierda
            monstruo.x = 100;
            monstruo.y = rand() % altoMax;
            break;
    }
    
    monstruo.vx = 5.0f;
    monstruo.vy = 5.0f;
    monstruo.ang = 0.0f;
    monstruo.radio = RADIO_SEEKER;
    monstruo.activo = true;
    monstruo.tipo = 2; // seeker
    monstruo.siguiente = nullptr;
}

// ============================================================================
// FUNCIONES DE MOVIMIENTO
// ============================================================================

// Movimiento del Wanderer (rebota en bordes)
void movimientoWanderer(Nave& monstruo, int anchoMax, int altoMax) {
    if (!monstruo.activo) return;

    // Bordes más cercanos (50 px)
    int borde_izquierdo = 50;
    int borde_derecho = anchoMax - 50;
    int borde_superior = 50;
    int borde_inferior = altoMax - 50;

    if (monstruo.x <= borde_izquierdo || monstruo.x >= borde_derecho) {
        monstruo.vx = -monstruo.vx;
    }
    if (monstruo.y <= borde_superior || monstruo.y >= borde_inferior) {
        monstruo.vy = -monstruo.vy;
    }

    monstruo.x += monstruo.vx;
    monstruo.y += monstruo.vy;

    // Forzar dentro de los límites
    if (monstruo.x < borde_izquierdo) monstruo.x = borde_izquierdo;
    if (monstruo.x > borde_derecho) monstruo.x = borde_derecho;
    if (monstruo.y < borde_superior) monstruo.y = borde_superior;
    if (monstruo.y > borde_inferior) monstruo.y = borde_inferior;
}

// Movimiento del Seeker (persigue al jugador)
void movimientoSeeker(Nave& monstruo, Nave& jugador, int anchoMax, int altoMax) {
    if (!monstruo.activo) return;

    float dx = jugador.x - monstruo.x;
    float dy = jugador.y - monstruo.y;
    float d = sqrt(dx * dx + dy * dy);
    
    if (d == 0.0f) return;
    
    float speed = 6.0f;
    monstruo.x += (dx / d) * speed;
    monstruo.y += (dy / d) * speed;

    // Limitar a bordes (50 px de margen)
    int margen = 50;
    if (monstruo.x < margen) monstruo.x = margen;
    if (monstruo.x > anchoMax - margen) monstruo.x = anchoMax - margen;
    if (monstruo.y < margen) monstruo.y = margen;
    if (monstruo.y > altoMax - margen) monstruo.y = altoMax - margen;
}

// ============================================================================
// FUNCIONES DE LISTAS ENLAZADAS - ENEMIGOS
// ============================================================================

// Agregar enemigo al final de la lista
void agregarEnemigo(PtrNave& cabeza, Nave nuevoEnemigo) {
    PtrNave nuevo = new Nave;
    *nuevo = nuevoEnemigo;
    nuevo->siguiente = nullptr;

    if (cabeza == nullptr) {
        cabeza = nuevo;
    } else {
        PtrNave temp = cabeza;
        while (temp->siguiente != nullptr) {
            temp = temp->siguiente;
        }
        temp->siguiente = nuevo;
    }
}

// Actualizar movimiento de todos los enemigos
void actualizarEnemigos(PtrNave cabeza, Nave& jugador, int anchoMax, int altoMax) {
    PtrNave temp = cabeza;
    while (temp != nullptr) {
        if (temp->activo) {
            if (temp->tipo == 1) { // Drone
                movimientoWanderer(*temp, anchoMax, altoMax);
            } else if (temp->tipo == 2) { // Seeker
                movimientoSeeker(*temp, jugador, anchoMax, altoMax);
            }
        }
        temp = temp->siguiente;
    }
}

// Contar enemigos activos
int contarEnemigosActivos(PtrNave cabeza) {
    int count = 0;
    PtrNave temp = cabeza;
    while (temp != nullptr) {
        if (temp->activo) count++;
        temp = temp->siguiente;
    }
    return count;
}

// Liberar memoria de la lista de enemigos
void liberarEnemigos(PtrNave& cabeza) {
    while (cabeza != nullptr) {
        PtrNave temp = cabeza;
        cabeza = cabeza->siguiente;
        delete temp;
    }
}

// ============================================================================
// FUNCIONES DE LISTAS ENLAZADAS - BALAS
// ============================================================================

// Agregar bala a la lista
void agregarBala(PtrBala& cabeza, Bala nuevaBala) {
    PtrBala nueva = new Bala;
    *nueva = nuevaBala;
    nueva->siguiente = nullptr;

    if (cabeza == nullptr) {
        cabeza = nueva;
    } else {
        PtrBala temp = cabeza;
        while (temp->siguiente != nullptr) {
            temp = temp->siguiente;
        }
        temp->siguiente = nueva;
    }
}

// Actualizar posición de todas las balas
void actualizarBalas(PtrBala& cabeza) {
    PtrBala temp = cabeza;
    while (temp != nullptr) {
        if (temp->activa) {
            temp->x += temp->vx;
            temp->y += temp->vy;
            temp->tiempo_vida -= 1.0f;
            
            if (temp->tiempo_vida <= 0.0f) {
                temp->activa = false;
            }
        }
        temp = temp->siguiente;
    }
}

// Eliminar balas inactivas de la lista
void limpiarBalas(PtrBala& cabeza) {
    while (cabeza != nullptr && !cabeza->activa) {
        PtrBala temp = cabeza;
        cabeza = cabeza->siguiente;
        delete temp;
    }

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

// Liberar memoria de todas las balas
void liberarBalas(PtrBala& cabeza) {
    while (cabeza != nullptr) {
        PtrBala temp = cabeza;
        cabeza = cabeza->siguiente;
        delete temp;
    }
}

// ============================================================================
// SISTEMA DE DISPARO
// ============================================================================

// Crear una bala desde la posición del jugador
void dispararBala(PtrBala& cabeza, Nave& jugador) {
    Bala nueva;
    
    // Posición inicial: punta de la nave
    nueva.x = jugador.x + sin(jugador.ang) * 30.0f;
    nueva.y = jugador.y - cos(jugador.ang) * 30.0f;
    
    // Velocidad en dirección de la nave
    nueva.vx = sin(jugador.ang) * VELOCIDAD_BALA;
    nueva.vy = -cos(jugador.ang) * VELOCIDAD_BALA;
    
    nueva.activa = true;
    nueva.tiempo_vida = VIDA_BALA;
    nueva.siguiente = nullptr;
    
    agregarBala(cabeza, nueva);
}

// ============================================================================
// DETECCIÓN DE COLISIONES (HITBOX CIRCULAR)
// ============================================================================

// Detectar colisión entre dos círculos
bool hayColision(float x1, float y1, float r1, float x2, float y2, float r2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float distancia = sqrt(dx * dx + dy * dy);
    return distancia < (r1 + r2);
}

// Verificar colisiones entre balas y enemigos
int verificarColisionesBalasEnemigos(PtrBala balas, PtrNave enemigos) {
    int enemigos_eliminados = 0;
    PtrBala bala = balas;
    while (bala != nullptr) {
        if (bala->activa) {
            PtrNave enemigo = enemigos;
            while (enemigo != nullptr) {
                if (enemigo->activo) {
                    if (hayColision(bala->x, bala->y, RADIO_BALA, enemigo->x, enemigo->y, enemigo->radio)) {
                        // Colisión detectada
                        bala->activa = false;
                        enemigo->activo = false;
                        enemigos_eliminados++;
                    }
                }
                enemigo = enemigo->siguiente;
            }
        }
        bala = bala->siguiente;
    }
    return enemigos_eliminados;
}

// Verificar colisiones entre jugador y enemigos
bool verificarColisionJugadorEnemigos(Nave& jugador, PtrNave enemigos) {
    if (!jugador.activo) return false;
    
    PtrNave enemigo = enemigos;
    while (enemigo != nullptr) {
        if (enemigo->activo) {
            if (hayColision(jugador.x, jugador.y, jugador.radio, enemigo->x, enemigo->y, enemigo->radio)) {
                return true; // Jugador murió
            }
        }
        enemigo = enemigo->siguiente;
    }
    return false;
}

// ============================================================================
// SISTEMA DE OLEADAS
// ============================================================================

// Calcular cuántos enemigos deben aparecer en una ronda
int calcularEnemigosEnRonda(int numeroRonda) {
    return ENEMIGOS_RONDA_INICIAL + (numeroRonda - 1) * INCREMENTO_POR_RONDA;
}

// Generar una oleada de enemigos (proporciones: 60% drones, 40% seekers)
void generarOleada(PtrNave& lista_enemigos, int numeroRonda, int anchoMax, int altoMax) {
    int totalEnemigos = calcularEnemigosEnRonda(numeroRonda);
    int numDrones = (totalEnemigos * 60) / 100;
    int numSeekers = totalEnemigos - numDrones;
    
    // Agregar drones
    for (int i = 0; i < numDrones; i++) {
        Nave drone;
        iniciarWandererAleatorio(drone, anchoMax, altoMax);
        agregarEnemigo(lista_enemigos, drone);
    }
    
    // Agregar seekers
    for (int i = 0; i < numSeekers; i++) {
        Nave seeker;
        iniciarSeekerAleatorio(seeker, anchoMax, altoMax);
        agregarEnemigo(lista_enemigos, seeker);
    }
}

// Limpiar enemigos muertos de la lista
void limpiarEnemigosInactivos(PtrNave& cabeza) {
    while (cabeza != nullptr && !cabeza->activo) {
        PtrNave temp = cabeza;
        cabeza = cabeza->siguiente;
        delete temp;
    }

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