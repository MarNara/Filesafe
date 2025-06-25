#ifndef MOVIMIENTO_H
#define MOVIMIENTO_H

#include "raylib.h"
#include "hashmap.h"
#include "sprite.h"

// --- Constantes de movimiento y físicas ---
#define TILE_SIZE 64.0f

#define GRAVITY 600.0f
#define JUMP_FORCE -400.0f
#define VELOCIDAD_MOVIMIENTO 200.0f
#define ACELERACION 1000.0f
#define FRICCION 800.0f

#define VELOCIDAD_DASH 900.0f
#define TIEMPO_DASH 0.2f
#define VELOCIDAD_MAX_CAIDA 800.0f

#define TIEMPO_COYOTE 0.1f             // Tiempo para saltar tras dejar el suelo
#define TIEMPO_BUFFER_SALTO 0.1f       // Tiempo de "buffer" para entrada de salto
#define TIEMPO_SALTO_PARED 0.2f        // Tiempo tras un salto en pared

// --- Estructura del personaje ---
typedef struct {
    // Datos del jugador
    char nombre[9];
    int vida;
    int ataque;
    int defensa;

    // Movimiento y físicas
    Vector2 posicion;
    Vector2 spawn;
    Vector2 velocidad;
    Vector2 offsetVisual;

    // Estados
    bool enSuelo;
    bool puedeDash;
    bool estaDashing;
    bool puedeDobleSalto;
    bool enParedIzquierda;
    bool enParedDerecha;
    bool mirandoDerecha;

    // Timers
    float tiempoCambioDireccion;
    float tiempoCoyote;
    float contadorCoyote;
    float tiempoBufferSalto;
    float contadorBufferSalto;
    float tiempoDash;
    float contadorDash;
    float contadorSaltoPared;

    // Inventario
    HashMap *inventario;

    // Sprites
    Sprite *spriteIdle;
    Sprite *spriteJump;
    Sprite *spriteClimb;
    Sprite *spriteRun;
    Sprite *spriteActual;
} Personaje;

// --- Inicialización ---
void InicializarPersonaje(Personaje *jugador);

// --- Movimiento y físicas ---
void ProcesarEntradas(Personaje *jugador);
void AplicarFisicas(Personaje *jugador, float delta);
void ManejarDash(Personaje *jugador, float delta);
void ManejarSaltos(Personaje *jugador, float delta);

// --- Colisiones ---
void ManejarColisiones(Personaje *jugador, Vector2 nuevaPos, int anchoMapa, int altoMapa, bool (*VerificarColision)(int, int, int, int));
void ManejarParedes(Personaje *jugador, int anchoMapa, int altoMapa, bool (*VerificarColision)(int, int, int, int));

// --- Lógica de movimiento completa ---
void ActualizarMovimiento(Personaje *jugador, int anchoMapa, int altoMapa, float delta, bool (*VerificarColision)(int, int, int, int));

// --- Funciones de sprite ---
void ActualizarSpriteJugador(Personaje* jugador);

#endif // MOVIMIENTO_H
