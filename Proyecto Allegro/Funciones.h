/*
 * FUNCIONES.H
 * -----------
 * Estructuras, constantes y funciones auxiliares del juego
 */

#pragma once // Evita inclusiones multiples del archivo de cabecera

#include <cmath> // Funciones matematicas utilizadas en la logica del juego
#include <cstdlib> // Utilidades de C para generacion de aleatorios y conversiones
#include <ctime> // Permite trabajar con tiempos para semillas aleatorias
#include <fstream> // Proporciona lectura y escritura de archivos
#include <string> // Usa cadenas de texto de C++ para nombres y mensajes
#include <vector> // Coleccion dinamica utilizada para listas de estadisticas
#include <algorithm> // Funciones de ordenamiento utilizadas en estadisticas
#include <allegro5/allegro.h> // Tipos y funciones generales de Allegro
#include <allegro5/allegro_audio.h> // Control de audio en Allegro
#include <allegro5/allegro_acodec.h> // Codecs de audio necesarios para reproducir formatos diversos

using namespace std; // Facilita el acceso a tipos estandar sin prefijo std::

// ========== ENUMS ==========

enum EstadoJuego {
        JUGANDO, // El jugador esta participando activamente en una ronda
        CAMBIO_RONDA, // La partida se encuentra en transicion entre rondas
        GAME_OVER, // El jugador ha sido derrotado
        INPUT_NOMBRE // Se solicita el nombre del jugador para registrar la puntuacion
};

// ========== ESTRUCTURAS ==========

typedef struct Nave {
        float x, y; // Posicion actual del objeto en el espacio 2D
        float vx, vy; // Velocidad en los ejes X e Y respectivamente
        float ang; // Angulo de orientacion en radianes
        float radio; // Radio de colision utilizado en las detecciones circulares
        bool activo; // Indicador de si el objeto sigue participando en el juego
        int tipo; // Identificador del tipo de nave (jugador, drone, seeker)
        struct Nave* siguiente; // Puntero al siguiente elemento en la lista enlazada
} *PtrNave; // Define PtrNave como alias de puntero a Nave

typedef struct Bala {
        float x, y; // Posicion de la bala en el plano
        float vx, vy; // Componentes de velocidad de la bala
        bool activa; // Indica si la bala sigue disponible para colisiones
        float tiempo_vida; // Tiempo restante antes de que la bala expire
        struct Bala* siguiente; // Puntero al siguiente elemento de la lista enlazada
} *PtrBala; // Define PtrBala como alias de puntero a Bala

struct Estadistica {
        string nombre; // Nombre del jugador registrado
        int puntuacion; // Puntos obtenidos al finalizar la partida
        float tiempo; // Duracion de la partida en segundos
        int ronda; // Ronda maxima alcanzada
        int enemigos_eliminados; // Total de enemigos destruidos
        int proyectiles_disparados; // Cantidad de proyectiles lanzados durante la partida
};

// ========== CONSTANTES ==========

const float RADIO_JUGADOR = 30.0f; // Radio de colision utilizado para el jugador
const float RADIO_DRONE = 50.0f; // Radio de los enemigos tipo drone
const float RADIO_SEEKER = 45.0f; // Radio de los enemigos tipo seeker
const float RADIO_BALA = 5.0f; // Radio de las balas para colisiones circulares
const float VELOCIDAD_BALA = 15.0f; // Magnitud de la velocidad de las balas
const float VIDA_BALA = 180.0f; // Duracion de cada bala en frames antes de desactivarse
const float CADENCIA_DISPARO = 10.0f; // Intervalo de frames entre disparos consecutivos del jugador

const int ENEMIGOS_RONDA_INICIAL = 3; // Cantidad de enemigos presentes en la primera ronda
const int INCREMENTO_POR_RONDA = 2; // Numero adicional de enemigos que se agregan por ronda
const float DURACION_TRANSICION = 180.0f; // Tiempo en frames que dura la transicion entre rondas

// ========== INICIALIZACION ==========

void iniciarPersonaje(Nave& personaje, int x, int y) {
        personaje.x = x / 2.0f; // Coloca al jugador en el centro horizontal de la pantalla
        personaje.y = y / 2.0f; // Coloca al jugador en el centro vertical de la pantalla
        personaje.vx = 0.0f; // Inicializa la velocidad horizontal en reposo
        personaje.vy = 0.0f; // Inicializa la velocidad vertical en reposo
        personaje.ang = 0.0f; // Establece la orientacion inicial apuntando hacia arriba
        personaje.radio = RADIO_JUGADOR; // Asigna el radio de colision propio del jugador
        personaje.activo = true; // Marca al jugador como activo
        personaje.tipo = 0; // Identifica el objeto como jugador
        personaje.siguiente = nullptr; // Inicializa el puntero siguiente en la lista como nulo
}

void resetearJugador(Nave& personaje, int x, int y) {
        personaje.x = x / 2.0f; // Reposiciona al jugador en el centro horizontal
        personaje.y = y / 2.0f; // Reposiciona al jugador en el centro vertical
        personaje.vx = 0.0f; // Reinicia la velocidad horizontal a cero
        personaje.vy = 0.0f; // Reinicia la velocidad vertical a cero
        personaje.ang = 0.0f; // Restablece el angulo para mirar hacia arriba
}

void iniciarWandererAleatorio(Nave& monstruo, int anchoMax, int altoMax) {
        int lado = rand() % 4; // Determina un borde aleatorio de aparicion (0-3)

        switch (lado) { // Selecciona la ubicacion segun el borde elegido
                case 0: monstruo.x = rand() % anchoMax; monstruo.y = 100; break; // Parte superior de la pantalla
                case 1: monstruo.x = anchoMax - 100; monstruo.y = rand() % altoMax; break; // Lado derecho
                case 2: monstruo.x = rand() % anchoMax; monstruo.y = altoMax - 100; break; // Parte inferior
                case 3: monstruo.x = 100; monstruo.y = rand() % altoMax; break; // Lado izquierdo
        }

        monstruo.vx = (rand() % 10 + 5) * (rand() % 2 == 0 ? 1 : -1); // Asigna velocidad horizontal aleatoria positiva o negativa
        monstruo.vy = (rand() % 10 + 5) * (rand() % 2 == 0 ? 1 : -1); // Asigna velocidad vertical aleatoria positiva o negativa
        monstruo.ang = 0.0f; // No se usa un angulo especifico para el drone
        monstruo.radio = RADIO_DRONE; // Radio de colision propio del drone
        monstruo.activo = true; // Marca al enemigo como activo
        monstruo.tipo = 1; // Identifica el tipo drone para logica especifica
        monstruo.siguiente = nullptr; // Inicializa el enlace siguiente como nulo
}

void iniciarSeekerAleatorio(Nave& monstruo, int anchoMax, int altoMax) {
        int lado = rand() % 4; // Selecciona un borde aleatorio para la aparicion

        switch (lado) { // Define la posicion inicial segun el borde elegido
                case 0: monstruo.x = rand() % anchoMax; monstruo.y = 100; break; // Borde superior
                case 1: monstruo.x = anchoMax - 100; monstruo.y = rand() % altoMax; break; // Borde derecho
                case 2: monstruo.x = rand() % anchoMax; monstruo.y = altoMax - 100; break; // Borde inferior
                case 3: monstruo.x = 100; monstruo.y = rand() % altoMax; break; // Borde izquierdo
        }

        monstruo.vx = 5.0f; // Velocidad horizontal base para el perseguidor
        monstruo.vy = 5.0f; // Velocidad vertical base para el perseguidor
        monstruo.ang = 0.0f; // No se utiliza el angulo directamente
        monstruo.radio = RADIO_SEEKER; // Radio de colision para seekers
        monstruo.activo = true; // Marca el enemigo como disponible
        monstruo.tipo = 2; // Identificador de enemigo seeker
        monstruo.siguiente = nullptr; // Siguiente elemento de la lista inicialmente nulo
}

// ========== MOVIMIENTO ==========

void movimientoWanderer(Nave& monstruo, int anchoMax, int altoMax) {
        if (!monstruo.activo) return; // Si el enemigo esta inactivo no se procesa movimiento

        int izq = 50, der = anchoMax - 50, arr = 50, aba = altoMax - 50; // Define limites internos para los rebotes

        if (monstruo.x <= izq || monstruo.x >= der) monstruo.vx = -monstruo.vx; // Invierte la velocidad horizontal al tocar un borde lateral
        if (monstruo.y <= arr || monstruo.y >= aba) monstruo.vy = -monstruo.vy; // Invierte la velocidad vertical al tocar bordes superior o inferior

        monstruo.x += monstruo.vx; // Actualiza la posicion horizontal sumando la velocidad
        monstruo.y += monstruo.vy; // Actualiza la posicion vertical sumando la velocidad

        if (monstruo.x < izq) monstruo.x = izq; // Garantiza que el enemigo no salga del area permitida a la izquierda
        if (monstruo.x > der) monstruo.x = der; // Limita la posicion a la derecha
        if (monstruo.y < arr) monstruo.y = arr; // Limita el movimiento por arriba
        if (monstruo.y > aba) monstruo.y = aba; // Limita el movimiento por abajo
}

void movimientoSeeker(Nave& monstruo, Nave& jugador, int anchoMax, int altoMax) {
        if (!monstruo.activo) return; // Evita calcular movimiento si el enemigo esta inactivo

        float dx = jugador.x - monstruo.x; // Diferencia horizontal entre enemigo y jugador
        float dy = jugador.y - monstruo.y; // Diferencia vertical entre enemigo y jugador
        float d = sqrt(dx * dx + dy * dy); // Calcula la distancia utilizando la norma euclidiana

        if (d == 0.0f) return; // Evita division por cero si ambos estan en la misma posicion

        monstruo.x += (dx / d) * 6.0f; // Normaliza el vector y multiplica por la velocidad deseada en X
        monstruo.y += (dy / d) * 6.0f; // Normaliza el vector y multiplica por la velocidad deseada en Y

        if (monstruo.x < 50) monstruo.x = 50; // Restringe la posicion izquierda
        if (monstruo.x > anchoMax - 50) monstruo.x = anchoMax - 50; // Restringe la posicion derecha
        if (monstruo.y < 50) monstruo.y = 50; // Restringe la posicion superior
        if (monstruo.y > altoMax - 50) monstruo.y = altoMax - 50; // Restringe la posicion inferior
}

// ========== LISTAS ENLAZADAS - ENEMIGOS ==========

void agregarEnemigo(PtrNave& cabeza, Nave nuevoEnemigo) {
        PtrNave nuevo = new Nave; // Reserva memoria para un nuevo nodo de enemigo
        *nuevo = nuevoEnemigo; // Copia los datos del enemigo proporcionado
        nuevo->siguiente = nullptr; // Inicializa el puntero siguiente como nulo

        if (cabeza == nullptr) { // Si la lista aun esta vacia
                cabeza = nuevo; // El nuevo nodo pasa a ser la cabeza de la lista
        } else {
                PtrNave temp = cabeza; // Comienza a recorrer la lista desde la cabeza
                while (temp->siguiente != nullptr) temp = temp->siguiente; // Avanza hasta encontrar el ultimo nodo
                temp->siguiente = nuevo; // Enlaza el nuevo nodo al final de la lista
        }
}

void actualizarEnemigos(PtrNave cabeza, Nave& jugador, int anchoMax, int altoMax) {
        PtrNave temp = cabeza; // Inicia el recorrido desde el primer enemigo
        while (temp != nullptr) { // Recorre la lista completa
                if (temp->activo) { // Solo procesa enemigos activos
                        if (temp->tipo == 1) movimientoWanderer(*temp, anchoMax, altoMax); // Los drones rebotan en los bordes
                        else if (temp->tipo == 2) movimientoSeeker(*temp, jugador, anchoMax, altoMax); // Los seekers persiguen al jugador
                }
                temp = temp->siguiente; // Avanza al siguiente enemigo
        }
}

int contarEnemigosActivos(PtrNave cabeza) {
        int count = 0; // Contador de enemigos vivos inicializado en cero
        PtrNave temp = cabeza; // Recorre desde la cabeza de la lista
        while (temp != nullptr) { // Itera todos los nodos
                if (temp->activo) count++; // Incrementa el contador por cada enemigo activo
                temp = temp->siguiente; // Avanza al siguiente nodo
        }
        return count; // Devuelve el numero total de enemigos activos
}

void liberarEnemigos(PtrNave& cabeza) {
        while (cabeza != nullptr) { // Recorre la lista hasta que este vacia
                PtrNave temp = cabeza; // Almacena la referencia al nodo actual
                cabeza = cabeza->siguiente; // Avanza la cabeza al siguiente nodo
                delete temp; // Libera la memoria del nodo actual
        }
}

// ========== LISTAS ENLAZADAS - BALAS ==========

void agregarBala(PtrBala& cabeza, Bala nuevaBala) {
        PtrBala nueva = new Bala; // Reserva memoria para una bala
        *nueva = nuevaBala; // Copia los atributos de la bala proporcionada
        nueva->siguiente = nullptr; // Inicializa el enlace siguiente como nulo

        if (cabeza == nullptr) { // Si la lista esta vacia
                cabeza = nueva; // La nueva bala se convierte en la primera de la lista
        } else {
                PtrBala temp = cabeza; // Comienza a recorrer la lista
                while (temp->siguiente != nullptr) temp = temp->siguiente; // Encuentra la ultima bala
                temp->siguiente = nueva; // Agrega la nueva bala al final
        }
}

void actualizarBalas(PtrBala& cabeza) {
        PtrBala temp = cabeza; // Recorre desde la primera bala
        while (temp != nullptr) { // Itera toda la lista
                if (temp->activa) { // Solo procesa balas activas
                        temp->x += temp->vx; // Avanza la bala horizontalmente segun su velocidad
                        temp->y += temp->vy; // Avanza la bala verticalmente segun su velocidad
                        temp->tiempo_vida -= 1.0f; // Reduce el tiempo de vida por frame
                        if (temp->tiempo_vida <= 0.0f) temp->activa = false; // Desactiva la bala cuando su tiempo llega a cero
                }
                temp = temp->siguiente; // Pasa a la siguiente bala
        }
}

void limpiarBalas(PtrBala& cabeza) {
        while (cabeza != nullptr && !cabeza->activa) { // Elimina balas inactivas consecutivas al inicio
                PtrBala temp = cabeza; // Nodo temporal para liberar
                cabeza = cabeza->siguiente; // Mueve la cabeza al siguiente nodo
                delete temp; // Libera la memoria del nodo eliminado
        }

        if (cabeza != nullptr) { // Si todavia queda al menos una bala activa
                PtrBala actual = cabeza; // Comienza a recorrer desde el primer elemento activo
                while (actual->siguiente != nullptr) { // Examina el resto de la lista
                        if (!actual->siguiente->activa) { // Si la siguiente bala esta inactiva
                                PtrBala temp = actual->siguiente; // Guarda el nodo a eliminar
                                actual->siguiente = temp->siguiente; // Enlaza el nodo actual con el siguiente del eliminado
                                delete temp; // Libera la memoria del nodo removido
                        } else {
                                actual = actual->siguiente; // Avanza cuando la bala sigue activa
                        }
                }
        }
}

void liberarBalas(PtrBala& cabeza) {
        while (cabeza != nullptr) { // Recorre la lista hasta liberarla completa
                PtrBala temp = cabeza; // Nodo actual a destruir
                cabeza = cabeza->siguiente; // Avanza la cabeza al siguiente nodo
                delete temp; // Libera el nodo actual
        }
}

void dispararBala(PtrBala& cabeza, Nave& jugador) {
        Bala nueva; // Crea una instancia temporal de bala
        nueva.x = jugador.x + sin(jugador.ang) * 30.0f; // Posicion inicial desplazada hacia la punta de la nave
        nueva.y = jugador.y - cos(jugador.ang) * 30.0f; // Ajusta la posicion vertical alineada con la direccion de disparo
        nueva.vx = sin(jugador.ang) * VELOCIDAD_BALA; // Componente horizontal de la velocidad basada en el angulo de la nave
        nueva.vy = -cos(jugador.ang) * VELOCIDAD_BALA; // Componente vertical de la velocidad
        nueva.activa = true; // Marca la bala como disponible para colisionar
        nueva.tiempo_vida = VIDA_BALA; // Asigna la duracion definida para las balas
        nueva.siguiente = nullptr; // Inicializa el enlace siguiente como nulo
        agregarBala(cabeza, nueva); // Inserta la bala en la lista enlazada de proyectiles
}

// ========== COLISIONES ==========

bool hayColision(float x1, float y1, float r1, float x2, float y2, float r2) {
        float dx = x2 - x1; // Diferencia horizontal entre los centros de los circulos
        float dy = y2 - y1; // Diferencia vertical entre los centros de los circulos
        float distancia = sqrt(dx * dx + dy * dy); // Distancia euclidiana entre ambos centros
        return distancia < (r1 + r2); // Retorna verdadero si los radios se superponen
}

int verificarColisionesBalasEnemigos(PtrBala balas, PtrNave enemigos) {
        int muertos = 0; // Contador de enemigos eliminados durante la comprobacion
        PtrBala bala = balas; // Recorre la lista de balas

        while (bala != nullptr) { // Itera por todas las balas
                if (bala->activa) { // Solo revisa balas activas
                        PtrNave enemigo = enemigos; // Recorre la lista de enemigos para cada bala
                        while (enemigo != nullptr) { // Itera cada enemigo
                                if (enemigo->activo) { // Solo toma en cuenta enemigos vivos
                                        if (hayColision(bala->x, bala->y, RADIO_BALA, enemigo->x, enemigo->y, enemigo->radio)) { // Comprueba superposicion
                                                bala->activa = false; // Desactiva la bala al impactar
                                                enemigo->activo = false; // Marca al enemigo como destruido
                                                muertos++; // Incrementa el numero de bajas registradas
                                        }
                                }
                                enemigo = enemigo->siguiente; // Avanza al siguiente enemigo en la lista
                        }
                }
                bala = bala->siguiente; // Avanza a la siguiente bala
        }
        return muertos; // Devuelve el total de enemigos eliminados en esta iteracion
}

bool verificarColisionJugadorEnemigos(Nave& jugador, PtrNave enemigos) {
        if (!jugador.activo) return false; // Si el jugador ya esta inactivo se omite la comprobacion

        PtrNave enemigo = enemigos; // Empieza a recorrer la lista de enemigos
        while (enemigo != nullptr) { // Itera por todos los enemigos
                if (enemigo->activo) { // Solo revisa los que siguen vivos
                        if (hayColision(jugador.x, jugador.y, jugador.radio, enemigo->x, enemigo->y, enemigo->radio)) { // Comprueba colision circular con el jugador
                                return true; // Devuelve verdadero en cuanto encuentra una colision
                        }
                }
                enemigo = enemigo->siguiente; // Avanza al siguiente enemigo
        }
        return false; // Si recorre toda la lista sin colisiones retorna falso
}

// ========== OLEADAS ==========

int calcularEnemigosEnRonda(int numeroRonda) {
        return ENEMIGOS_RONDA_INICIAL + (numeroRonda - 1) * INCREMENTO_POR_RONDA; // Aplica la progresion aritmetica de enemigos
}

void generarOleada(PtrNave& lista_enemigos, int numeroRonda, int anchoMax, int altoMax) {
        int total = calcularEnemigosEnRonda(numeroRonda); // Determina cuantos enemigos debe tener la ronda actual
        int drones = (total * 60) / 100; // Calcula un 60 por ciento del total para drones
        int seekers = total - drones; // El resto de enemigos son seekers

        for (int i = 0; i < drones; i++) { // Genera cada drone requerido
                Nave drone; // Crea un objeto temporal para inicializarlo
                iniciarWandererAleatorio(drone, anchoMax, altoMax); // Inicializa la posicion del drone
                agregarEnemigo(lista_enemigos, drone); // Inserta el drone en la lista de enemigos
        }

        for (int i = 0; i < seekers; i++) { // Genera cada seeker necesario
                Nave seeker; // Objeto temporal para inicializarlo
                iniciarSeekerAleatorio(seeker, anchoMax, altoMax); // Posiciona al seeker en un borde aleatorio
                agregarEnemigo(lista_enemigos, seeker); // Lo agrega a la lista enlazada
        }
}

void limpiarEnemigosInactivos(PtrNave& cabeza) {
        while (cabeza != nullptr && !cabeza->activo) { // Elimina nodos inactivos consecutivos desde el inicio
                PtrNave temp = cabeza; // Nodo temporal para liberar memoria
                cabeza = cabeza->siguiente; // Avanza la cabeza al siguiente nodo
                delete temp; // Libera el nodo inactivo
        }

        if (cabeza != nullptr) { // Si aun queda al menos un enemigo activo
                PtrNave actual = cabeza; // Comienza el recorrido desde la cabeza
                while (actual->siguiente != nullptr) { // Revisa los siguientes nodos
                        if (!actual->siguiente->activo) { // Si el siguiente nodo esta inactivo
                                PtrNave temp = actual->siguiente; // Nodo a eliminar
                                actual->siguiente = temp->siguiente; // Salta el nodo inactivo en la lista
                                delete temp; // Libera la memoria del nodo eliminado
                        } else {
                                actual = actual->siguiente; // Avanza cuando el nodo esta activo
                        }
                }
        }
}

// ========== PERSISTENCIA ==========

void guardarEstadisticas(const Estadistica& stats) {
        ofstream archivo("estadisticas.txt", ios::app); // Abre el archivo en modo de anexado
        if (archivo.is_open()) { // Comprueba que el archivo se abrio correctamente
                archivo << stats.nombre << "|" << stats.puntuacion << "|" << stats.tiempo << "|" << stats.ronda << "|" << stats.enemigos_eliminados << "|" << stats.proyectiles_disparados << "\n"; // Escribe los datos separados por tuberias
                archivo.close(); // Cierra el archivo para asegurar la escritura en disco
        }
}

vector<Estadistica> leerEstadisticas() {
        vector<Estadistica> lista; // Contenedor donde se almacenaran las estadisticas leidas
        ifstream archivo("estadisticas.txt"); // Abre el archivo en modo lectura

        if (archivo.is_open()) { // Comprueba que el archivo exista y se pueda abrir
                string linea; // Variable temporal para almacenar cada linea del archivo
                while (getline(archivo, linea)) { // Lee el archivo linea a linea
                        Estadistica stat; // Objeto temporal para cargar los datos desglosados
                        size_t pos1 = linea.find('|'); // Ubica el primer delimitador
                        size_t pos2 = linea.find('|', pos1 + 1); // Busca el segundo delimitador
                        size_t pos3 = linea.find('|', pos2 + 1); // Busca el tercer delimitador
                        size_t pos4 = linea.find('|', pos3 + 1); // Busca el cuarto delimitador
                        size_t pos5 = linea.find('|', pos4 + 1); // Busca el quinto delimitador si existe

                        if (pos1 != string::npos && pos2 != string::npos && pos3 != string::npos && pos4 != string::npos) { // Verifica que los delimitadores principales esten presentes
                                stat.nombre = linea.substr(0, pos1); // Extrae el nombre del jugador
                                stat.puntuacion = stoi(linea.substr(pos1 + 1, pos2 - pos1 - 1)); // Convierte la seccion de puntuacion a entero
                                stat.tiempo = stof(linea.substr(pos2 + 1, pos3 - pos2 - 1)); // Convierte la seccion de tiempo a flotante
                                stat.ronda = stoi(linea.substr(pos3 + 1, pos4 - pos3 - 1)); // Convierte la seccion de ronda a entero

                                string enemigos_str = (pos5 != string::npos) ? linea.substr(pos4 + 1, pos5 - pos4 - 1) : linea.substr(pos4 + 1); // Obtiene la seccion de enemigos eliminados
                                stat.enemigos_eliminados = stoi(enemigos_str); // Convierte el campo de enemigos a entero

                                stat.proyectiles_disparados = (pos5 != string::npos) ? stoi(linea.substr(pos5 + 1)) : 0; // Convierte el campo de proyectiles si existe, de lo contrario deja 0

                                lista.push_back(stat); // Agrega la estadistica a la coleccion
                        }
                }
                archivo.close(); // Cierra el archivo una vez terminado el proceso de lectura
        }

        sort(lista.begin(), lista.end(), [](const Estadistica& a, const Estadistica& b) {
                return a.puntuacion > b.puntuacion; // Ordena de mayor a menor puntuacion utilizando una lambda
        });

        return lista; // Devuelve la lista ordenada de estadisticas
}

vector<Estadistica> obtenerTop5() {
        vector<Estadistica> todas = leerEstadisticas(); // Recupera todas las estadisticas disponibles
        vector<Estadistica> top5; // Vector que almacenara solo las mejores cinco
        int limite = (todas.size() < 5) ? todas.size() : 5; // Determina cuantas entradas agregar segun el tamano disponible
        for (int i = 0; i < limite; i++) { // Recorre las primeras posiciones
                top5.push_back(todas[i]); // Copia cada estadistica seleccionada al vector top5
        }
        return top5; // Devuelve el subconjunto de mejores resultados
}

// ========== AUDIO ==========

ALLEGRO_SAMPLE* musica_menu = NULL; // Apuntador al sample de musica del menu principal
ALLEGRO_SAMPLE* musica_gameplay = NULL; // Apuntador al sample de musica durante el gameplay
ALLEGRO_SAMPLE* musica_gameover = NULL; // Apuntador al sample de la pantalla de game over
ALLEGRO_SAMPLE* sfx_disparo = NULL; // Efecto de sonido para disparos del jugador
ALLEGRO_SAMPLE* sfx_explosion = NULL; // Efecto de sonido para destruccion de enemigos
ALLEGRO_SAMPLE* sfx_muerte = NULL; // Efecto de sonido para la muerte del jugador

ALLEGRO_SAMPLE_ID id_musica_actual; // Identificador del sample actualmente en reproduccion
bool hay_musica_sonando = false; // Bandera que indica si hay musica activa

void cargarAudio() {
        musica_menu = al_load_sample("musica/Menu.ogg"); // Carga el archivo de musica del menu
        musica_gameplay = al_load_sample("musica/fight.ogg"); // Carga la musica de fondo del gameplay
        sfx_disparo = al_load_sample("musica/shoot.wav"); // Carga el efecto de disparo
        sfx_explosion = al_load_sample("musica/enemyexp.wav"); // Carga el efecto de explosion de enemigos
        sfx_muerte = al_load_sample("musica/playerexp.flac"); // Carga el efecto de muerte del jugador
}

void tocarMusica(ALLEGRO_SAMPLE* musica, float volumen) {
        if (hay_musica_sonando) al_stop_sample(&id_musica_actual); // Detiene cualquier sample que estuviera en reproduccion
        if (musica) hay_musica_sonando = al_play_sample(musica, volumen, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, &id_musica_actual); // Reproduce la musica en bucle con el volumen especificado
}

void pararMusica() {
        if (hay_musica_sonando) { // Solo actua si realmente hay musica sonando
                al_stop_sample(&id_musica_actual); // Detiene el sample actual
                hay_musica_sonando = false; // Actualiza la bandera para indicar que ya no hay musica
        }
}

void tocarSonido(ALLEGRO_SAMPLE* sonido, float volumen) {
        if (sonido) al_play_sample(sonido, volumen, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL); // Reproduce el sonido indicado una unica vez
}

void limpiarAudio() {
        pararMusica(); // Garantiza que la musica se detenga antes de liberar recursos
        if (musica_menu) al_destroy_sample(musica_menu); // Libera el sample del menu
        if (musica_gameplay) al_destroy_sample(musica_gameplay); // Libera el sample del gameplay
        if (musica_gameover) al_destroy_sample(musica_gameover); // Libera el sample del game over si fue cargado
        if (sfx_disparo) al_destroy_sample(sfx_disparo); // Libera el efecto de disparo
        if (sfx_explosion) al_destroy_sample(sfx_explosion); // Libera el efecto de explosion
        if (sfx_muerte) al_destroy_sample(sfx_muerte); // Libera el efecto de muerte
}
