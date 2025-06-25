#include "sprite.h"
#include <stdlib.h>
#include <math.h>
#include "raylib.h"
#define TILE_SIZE 64.0f

Sprite* CrearSprite(const char* ruta, int frameCount, float frameTime, Vector2 pos) {
    Sprite* s = malloc(sizeof(Sprite));

    // Cargar imagen con canal alfa asegurado
    Image img = LoadImage(ruta);
    ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    s->texture = LoadTextureFromImage(img);
    UnloadImage(img);

    s->frameCount = frameCount;
    s->frameTime = frameTime;
    s->currentFrame = 0;
    s->timeCounter = 0.0f;
    s->position = pos;
    s->flipX = false; // Por defecto sin flip
    s->frameRec = (Rectangle){0, 0, (float)s->texture.width / frameCount, (float)s->texture.height};
    return s;
}

void ActualizarSprite(Sprite* s, float deltaTime) {
    s->timeCounter += deltaTime;
    if (s->timeCounter >= s->frameTime) {
        s->currentFrame = (s->currentFrame + 1) % s->frameCount;
        s->frameRec.x = s->currentFrame * s->frameRec.width;
        s->timeCounter = 0.0f;
    }
}

void DibujarSprite(Sprite* s, Vector2 posicion) {
    float escala = TILE_SIZE / s->frameRec.width;
    
    // Preparamos el rectángulo de origen considerando el flip
    Rectangle sourceRec = s->frameRec;
    if (s->flipX) {
        sourceRec.width = -sourceRec.width; // Voltear horizontalmente
    }
    
    DrawTexturePro(
        s->texture,
        sourceRec,
        (Rectangle){
            posicion.x,
            posicion.y,
            fabs(sourceRec.width) * escala, // Usamos valor absoluto
            sourceRec.height * escala
        },
        (Vector2){0, 0},
        0.0f,
        WHITE
    );
}

void LiberarSprite(Sprite* s) {
    if (s) {
        UnloadTexture(s->texture);
        free(s);
    }
}