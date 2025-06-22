// movimiento.h
#ifndef MOVIMIENTO_H
#define MOVIMIENTO_H

#include "raylib.h"
#include "hashmap.h"

// Constantes de movimiento
#define TILE_SIZE 64.0f
#define GRAVITY 600.0f
#define JUMP_FORCE -400.0f
#define VELOCIDAD_MOVIMIENTO 200.0f
#define ACELERACION 1000.0f
#define FRICCION 800.0f
#define VELOCIDAD_DASH 900.0f
#define TIEMPO_COYOTE 0.1f
#define TIEMPO_BUFFER_SALTO 0.1f
#define TIEMPO_SALTO_PARED 0.2f
#define TIEMPO_DASH 0.2f
#define VELOCIDAD_MAX_CAIDA 800.0f

typedef struct {
    char nombre[9];
    int vida;
    int ataque;
    int defensa;
    Vector2 posicion;
    Vector2 spawn;
    Vector2 velocidad;
    HashMap *inventario; // Inventario del personaje
    bool enSuelo;
    float tiempoCoyote;
    float contadorCoyote;
    float tiempoBufferSalto;
    float contadorBufferSalto;
    bool puedeDash;
    bool estaDashing;
    float tiempoDash;
    float contadorDash;
    bool puedeDobleSalto;
    bool enParedIzquierda;
    bool enParedDerecha;
    float contadorSaltoPared;
} Personaje;

// Funciones de inicialización
void InicializarPersonaje(Personaje *jugador);

// Funciones de movimiento
void ProcesarEntradas(Personaje *jugador);
void AplicarFisicas(Personaje *jugador, float delta);
void ManejarDash(Personaje *jugador, float delta);
void ManejarColisiones(Personaje *jugador, Vector2 nuevaPos, int anchoMapa, int altoMapa, bool (*VerificarColision)(int, int, int, int));
void ManejarParedes(Personaje *jugador, int anchoMapa, int altoMapa, bool (*VerificarColision)(int, int, int, int));
void ManejarSaltos(Personaje *jugador, float delta);

// Función principal de actualización
void ActualizarMovimiento(Personaje *jugador, int anchoMapa, int altoMapa, float delta, bool (*VerificarColision)(int, int, int, int));

#endif // MOVIMIENTO_H