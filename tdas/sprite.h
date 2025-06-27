#ifndef SPRITE_H
#define SPRITE_H

#include "raylib.h"
<<<<<<< Updated upstream
#include "list.h"
=======
#include <stdbool.h>

typedef enum {
    SPRITE_PERSONAJE,
    SPRITE_OBJETO,
    SPRITE_ENEMIGO
} SpriteTipo;
>>>>>>> Stashed changes

typedef struct {
    Texture2D texture;
    int frameCount;
    float frameTime;
    int currentFrame;
    float timeCounter;
    Rectangle frameRec;
    Vector2 position;
    bool flipX;
    SpriteTipo tipo;
} Sprite;

// Funciones
Sprite* CrearSprite(const char* ruta, int frameCount, float frameTime, Vector2 pos, SpriteTipo tipo);
void ActualizarSprite(Sprite* s, float deltaTime);
<<<<<<< Updated upstream
void DibujarSprite(Sprite* s, Vector2 posicion, bool esPersonaje);
void EliminarSpritePorPosicion(List* sprites, Vector2 pos);
=======
void DibujarSprite(Sprite* s, Vector2 posicion);
void DibujarSpriteObjeto(Sprite* s);
>>>>>>> Stashed changes
void LiberarSprite(Sprite* s);

#endif