#include "sprite.h"
#include <stdlib.h>
#include "raylib.h"
#define TILE_SIZE 64.0f  // Asegúrate de usar el mismo TILE_SIZE que en final2.c

Sprite* CrearSprite(const char* ruta, int frameCount, float frameTime, Vector2 pos) {
    Sprite* s = malloc(sizeof(Sprite));
    s->texture = LoadTexture(ruta);
    s->frameCount = frameCount;
    s->frameTime = frameTime;
    s->position = pos;
    s->currentFrame = 0;
    s->timeCounter = 0.0f;
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

void DibujarSprite(Sprite* s) {
    Rectangle dest = {
        s->position.x,
        s->position.y,
        TILE_SIZE,
        TILE_SIZE
    };
    DrawTexturePro(s->texture, s->frameRec, dest, (Vector2){0, 0}, 0.0f, RAYWHITE);
}

void LiberarSprite(Sprite* s) {
    UnloadTexture(s->texture);
    free(s);
}
