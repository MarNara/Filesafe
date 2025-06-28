#ifndef MOVIMIENTO_H
#define MOVIMIENTO_H

#include "raylib.h"
#include "hashmap.h"
#include "sprite.h"

// --- Constantes relacionadas al movimiento y físicas del personaje ---
#define TILE_SIZE 64.0f                 // Tamaño de un "tile" del mapa (usado en colisiones y escala)

#define GRAVITY 600.0f                  // Aceleración constante hacia abajo
#define JUMP_FORCE -400.0f             // Fuerza aplicada al iniciar un salto
#define VELOCIDAD_MOVIMIENTO 200.0f    // Velocidad máxima horizontal al correr
#define ACELERACION 1000.0f            // Aceleración horizontal al presionar teclas
#define FRICCION 800.0f                // Resistencia que desacelera al dejar de moverse

#define VELOCIDAD_DASH 900.0f          // Velocidad horizontal durante un dash
#define TIEMPO_DASH 0.2f               // Duración del dash
#define VELOCIDAD_MAX_CAIDA 800.0f     // Límite de velocidad al caer

#define TIEMPO_COYOTE 0.1f             // Tiempo extra para permitir salto después de dejar el suelo
#define TIEMPO_BUFFER_SALTO 0.1f       // Tiempo para guardar una pulsación de salto temprana
#define TIEMPO_SALTO_PARED 0.2f        // Tiempo para controlar impulso tras saltar desde una pared

// --- Estructura que representa al personaje jugable ---
typedef struct {
    // Información general
    char nombre[9];        // Nombre del personaje
    int vida;              // Salud actual
    int ataque;            // Valor de ataque
    int defensa;           // Valor de defensa

    // Posición y movimiento
    Vector2 posicion;      // Posición actual en el mapa
    Vector2 spawn;         // Posición de aparición o reinicio
    Vector2 velocidad;     // Velocidad actual
    Vector2 offsetVisual;  // Offset visual para alinear el sprite con la hitbox

    // Estados físicos
    bool enSuelo;              // Está tocando el suelo
    bool puedeDash;            // Puede hacer dash
    bool estaDashing;          // Está actualmente en un dash
    bool puedeDobleSalto;      // Tiene disponible un doble salto
    bool enParedIzquierda;     // Está tocando una pared a la izquierda
    bool enParedDerecha;       // Está tocando una pared a la derecha
    bool mirandoDerecha;       // Dirección en la que está mirando

    // Timers de mecánicas
    float tiempoCambioDireccion; 
    float tiempoCoyote;          // Duración total del "coyote time"
    float contadorCoyote;        // Temporizador que se va agotando desde que deja el suelo
    float tiempoBufferSalto;     // Tiempo total para aceptar un salto anticipado
    float contadorBufferSalto;   // Temporizador para registrar un salto antes de estar en el suelo
    float tiempoDash;            // Duración total del dash
    float contadorDash;          // Temporizador del dash actual
    float contadorSaltoPared;    // Temporizador del impulso tras salto en pared

    // Inventario del personaje
    HashMap *inventario;         // Almacena ítems que el jugador recolecta

    // Sprites del personaje según su estado
    Sprite* spriteIdleStart;     // Animación idle al quedarse quieto
    Sprite* spriteIdleWalk;      // Animación al caminar
    Sprite* spriteJumpUp;        // Animación al iniciar un salto
    Sprite* spriteJumpDown;      // Animación al caer
    Sprite* spriteClimb;         // Animación de escalada en pared
    Sprite* spriteRun;           // Animación de carrera o dash
    Sprite* spriteActual;        // Puntero al sprite que se está usando actualmente
} Personaje;

// --- Funciones de inicialización ---
void InicializarPersonaje(Personaje *jugador);

// --- Funciones de movimiento y físicas ---
void ProcesarEntradas(Personaje *jugador);           // Captura y traduce teclas en acciones
void AplicarFisicas(Personaje *jugador, float delta); // Aplica aceleración, fricción, gravedad
void ManejarDash(Personaje *jugador, float delta);    // Controla el tiempo del dash
void ManejarSaltos(Personaje *jugador, float delta);  // Maneja salto normal, doble y en pared

// --- Funciones de detección de colisiones ---
void ManejarColisiones(Personaje *jugador, Vector2 nuevaPos, int anchoMapa, int altoMapa,
                       bool (*VerificarColision)(int, int, int, int)); // Detecta y resuelve colisiones
void ManejarParedes(Personaje *jugador, int anchoMapa, int altoMapa,
                    bool (*VerificarColision)(int, int, int, int)); // Detecta contacto con paredes

// --- Lógica general del movimiento por frame ---
void ActualizarMovimiento(Personaje *jugador, int anchoMapa, int altoMapa, float delta,
                          bool (*VerificarColision)(int, int, int, int));

// --- Actualización visual del sprite según el estado actual ---
void ActualizarSpriteJugador(Personaje* jugador);

#endif // MOVIMIENTO_H
