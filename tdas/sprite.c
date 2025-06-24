#include "sprite.h"
#include <stdlib.h>
#include "raylib.h"
#define TILE_SIZE 64.0f  // Asegúrate de usar el mismo TILE_SIZE que en final2.c

Sprite* CrearSprite(const char* ruta, int frameCount, float frameTime, Vector2 pos) {
    Sprite* s = malloc(sizeof(Sprite));

    // Cargar imagen con canal alfa asegurado
    Image img = LoadImage(ruta);
    ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8); // Formato con canal alfa
    s->texture = LoadTextureFromImage(img);
    UnloadImage(img);

    s->frameCount = frameCount;
    s->frameTime = frameTime;
    s->currentFrame = 0;
    s->timeCounter = 0.0f;
    s->position = pos;
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
    DrawTexturePro(
        s->texture,
        s->frameRec,
        (Rectangle){
            posicion.x,
            posicion.y,
            s->frameRec.width * escala,
            s->frameRec.height * escala
        },
        (Vector2){0, 0},
        0.0f,
        RAYWHITE
    );
}

void LiberarSprite(Sprite* s) {
    if (s) {
        UnloadTexture(s->texture);
        free(s);
    }
}