#ifndef SPRITE_H
#define SPRITE_H

#include "raylib.h"

// Estructura que representa un sprite animado
typedef struct {
    Texture2D texture;       // Textura que contiene todos los frames de animación
    Rectangle frameRec;      // Rectángulo que define el frame actual dentro de la textura
    int frameCount;          // Número total de frames en la animación
    int currentFrame;        // Índice del frame actual que se está mostrando
    float frameTime;         // Tiempo que debe pasar para cambiar al siguiente frame
    float timeCounter;       // Contador que acumula el tiempo transcurrido desde el último cambio de frame
    Vector2 position;        // Posición del sprite en el mundo o pantalla
    bool flipX;              // Si es true, se dibuja volteado horizontalmente
} Sprite;

// Crea un sprite animado a partir de una imagen, número de frames y posición inicial
Sprite* CrearSprite(const char* ruta, int frameCount, float frameTime, Vector2 pos);

// Actualiza el sprite, cambiando de frame si corresponde según el deltaTime
void ActualizarSprite(Sprite* s, float deltaTime);

// Dibuja el sprite en pantalla en la posición indicada
void DibujarSprite(Sprite* s, Vector2 posicion);

// Libera los recursos asociados al sprite
void LiberarSprite(Sprite* s);

#endif
