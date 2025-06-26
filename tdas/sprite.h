#ifndef SPRITE_H
#define SPRITE_H

#include "raylib.h"
#include "list.h"

typedef struct {
    Texture2D texture;
    Rectangle frameRec;
    int frameCount;
    int currentFrame;
    float frameTime;
    float timeCounter;
    Vector2 position; // Posición del sprite
    bool flipX;
} Sprite;

Sprite* CrearSprite(const char* ruta, int frameCount, float frameTime, Vector2 pos);
void ActualizarSprite(Sprite* s, float deltaTime);
void DibujarSprite(Sprite* s, Vector2 posicion, bool esPersonaje);
void EliminarSpritePorPosicion(List* sprites, Vector2 pos);
void LiberarSprite(Sprite* s);

#endif