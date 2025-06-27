#include "sprite.h"
#include "list.h"
#include <stdlib.h>
#include <math.h>
#include "raylib.h"

#define TILE_SIZE 64.0f

Sprite* CrearSprite(const char* ruta, int frameCount, float frameTime, Vector2 pos, SpriteTipo tipo) {
    Sprite* s = malloc(sizeof(Sprite));
    if (!s) return NULL;

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
    s->flipX = false;
    s->tipo = tipo;

    s->frameRec = (Rectangle){
        0,
        0,
        (float)s->texture.width / frameCount,
        (float)s->texture.height
    };

    return s;
}

void ActualizarSprite(Sprite* s, float deltaTime) {
    // Solo actualizar si hay más de un frame (animación)
    if (s->frameCount > 1) {
        s->timeCounter += deltaTime;
        if (s->timeCounter >= s->frameTime) {
            s->currentFrame = (s->currentFrame + 1) % s->frameCount;
            s->frameRec.x = s->currentFrame * s->frameRec.width;
            s->timeCounter = 0.0f;
        }
    }
}

<<<<<<< Updated upstream
void DibujarSprite(Sprite* s, Vector2 posicion, bool esPersonaje) {
    float escala = TILE_SIZE / s->frameRec.width;
    
    Rectangle sourceRec = s->frameRec;
    if (s->flipX) {
        sourceRec.width = -sourceRec.width;
    }

    Rectangle destRec = {
        posicion.x,
        posicion.y,
        fabs(sourceRec.width) * escala,
        sourceRec.height * escala
    };

    // Comportamiento diferente para personaje vs otros sprites
    if (!esPersonaje) {
        // Ajuste para enemigos/fuego: alinear con el suelo
        destRec.y += TILE_SIZE - destRec.height;
=======
void DibujarSprite(Sprite* s, Vector2 posicion) {
    float escala;
    Rectangle destino;

    switch (s->tipo) {
        case SPRITE_PERSONAJE:
            escala = TILE_SIZE / s->frameRec.width;
            destino = (Rectangle){
                posicion.x,
                posicion.y,
                s->frameRec.width * escala,
                s->frameRec.height * escala
            };
            break;

        case SPRITE_OBJETO:
        case SPRITE_ENEMIGO:
            escala = 0.8f;
            destino = (Rectangle){
                s->position.x + (TILE_SIZE - TILE_SIZE * escala) / 2,
                s->position.y + TILE_SIZE - TILE_SIZE * escala,
                TILE_SIZE * escala,
                TILE_SIZE * escala
            };
            break;
    }

    Rectangle origen = s->frameRec;
    if (s->flipX) {
        origen.width = -origen.width;
>>>>>>> Stashed changes
    }

    DrawTexturePro(
        s->texture,
<<<<<<< Updated upstream
        sourceRec,
        destRec,
=======
        origen,
        destino,
        (Vector2){0, 0},
        0.0f,
        WHITE
    );
}

void DibujarSpriteObjeto(Sprite* s) {
    float escala = 0.8f;
    Rectangle destino = (Rectangle){
        s->position.x + (TILE_SIZE - TILE_SIZE * escala) / 2,
        s->position.y + TILE_SIZE - TILE_SIZE * escala,
        TILE_SIZE * escala,
        TILE_SIZE * escala
    };

    DrawTexturePro(
        s->texture,
        s->frameRec,
        destino,
>>>>>>> Stashed changes
        (Vector2){0, 0},
        0.0f,
        WHITE
    );
}

void EliminarSpritePorPosicion(List* sprites, Vector2 pos) {
    if (sprites == NULL || sprites->head == NULL) return;

    Node* nodo = sprites->head;
    Node* prev = NULL;
    while (nodo) {
        Sprite* s = (Sprite*)nodo->data;
        float dx = fabsf(s->position.x - pos.x);
        float dy = fabsf(s->position.y - pos.y);

        // Tolerancia para considerar la misma posición (puedes ajustar este valor)
        if (dx < 1.0f && dy < 1.0f) {
            // Encontrado, eliminar nodo de la lista
            if (prev == NULL) {
                sprites->head = nodo->next;
            } else {
                prev->next = nodo->next;
            }
            if (nodo == sprites->tail) {
                sprites->tail = prev;
            }

            LiberarSprite(s);
            free(nodo);
            sprites->size--;
            return; // Eliminamos solo uno
        }

        prev = nodo;
        nodo = nodo->next;
    }
}

void LiberarSprite(Sprite* s) {
    if (s) {
        UnloadTexture(s->texture);
        free(s);
    }
}