// Se incluyen las librerias
#include <stdio.h> // Funciones de E/S de C para posibles mensajes
#include <iostream> // Flujo de entrada y salida de C++

#include <allegro5/allegro.h> // Funciones de Allegro utilizadas para obtener informacion de monitor

using namespace std; // Facilita el uso de cout/cin si fuese necesario

// Se crea la estructura bicho
struct bicho {

        float x; // Posicion horizontal del ente
        float y; // Posicion vertical del ente
        float vx; // Velocidad horizontal
        float vy; // Velocidad vertical
        float rotacion; // Angulo actual del ente
        float rozamiento; // Factor de friccion aplicado en movimiento
        float accel; // Magnitud de aceleracion
        float velocidad_max; // Limite superior de velocidad permitida

};

// Esta funcion inicializa al personaje en su posicion inicial
void iniciarPersonaje(bicho& personaje, int x, int y) {

        personaje.x = 25; // Establece la coordenada X inicial del personaje
        personaje.y = y - 40; // Ubica al personaje cerca de la parte inferior de la pantalla
        personaje.rotacion = 0; // Reinicia la rotacion
        personaje.vx = 0; // Coloca la velocidad horizontal en cero
        personaje.vy = 0; // Coloca la velocidad vertical en cero
        personaje.accel = 0; // Reinicia la aceleracion
        personaje.rozamiento = 0; // Reinicia el valor de friccion
        personaje.velocidad_max = 5; // Limita la velocidad maxima permitida

}

// Esta funcion inicializa al monstruo en su posicion inicial y le asigna una velocidad
void iniciarMonstruo(bicho& monstruo, int x, int y) {

        monstruo.x = x / 2; // Coloca al monstruo en la mitad del ancho disponible
        monstruo.y = y / 2; // Coloca al monstruo en la mitad del alto disponible
        monstruo.vx = 0; // Inicializa la velocidad horizontal en cero
        monstruo.vy = 0; // Inicializa la velocidad vertical en cero

}

// Esta funcion controla el movimiento del monstruo y lo restringe a moverse mas alla de los bordes asignados
void movimientoMonstruo(bicho& monstruo) {

        ALLEGRO_MONITOR_INFO info; // Estructura para almacenar informacion del monitor
        al_get_monitor_info(0, &info); // Obtiene los limites del monitor principal
        const int X = info.x2 - info.x1; // Calcula el ancho total disponible

        int borde_izquierdo = 155; // Posicion limite izquierda para el movimiento
        int borde_derecho = X - 155; // Posicion limite derecha para el movimiento

        if (monstruo.x <= borde_izquierdo || monstruo.x >= borde_derecho) {
                monstruo.vx = -monstruo.vx; // Invierte la velocidad para simular rebote en bordes
        }
        monstruo.x += monstruo.vx; // Actualiza la posicion horizontal del monstruo
}
