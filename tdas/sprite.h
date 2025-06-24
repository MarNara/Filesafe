#ifndef SPRITE_H
#define SPRITE_H

#include "raylib.h"

typedef struct Sprite {
    Texture2D texture;
    Rectangle frameRec;
    int frameCount;
    int currentFrame;
    float frameTime;
    float timeCounter;
    Vector2 position;
} Sprite;

Sprite* CrearSprite(const char* ruta, int frameCount, float frameTime, Vector2 pos);
void ActualizarSprite(Sprite* s, float deltaTime);
void DibujarSprite(Sprite* s);
void LiberarSprite(Sprite* s);

#endif
