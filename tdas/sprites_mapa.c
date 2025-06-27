#include "sprite.h"
#include "list.h"
#include "sprites_mapa.h"  // Asegúrate de que defines la estructura del mapa aquí
#include <stdlib.h>
#include <math.h>
#define TILE_SIZE 64.0f

extern List* spritesActivos; // Lista compartida global

void LimpiarSpritesActivos() {
    if (spritesActivos) {
        Node* nodo = spritesActivos->head;
        while (nodo) {
            LiberarSprite((Sprite*)nodo->data);
            nodo = nodo->next;
        }
        list_clean(spritesActivos);
    }
}

Sprite* CrearSpriteDesdeCodigo(int codigo, Vector2 posicionTile) {
    Vector2 posicion = {
        posicionTile.x + TILE_SIZE/2,  // Centrado en X
        posicionTile.y + TILE_SIZE/2   // Centrado en Y
    };

    switch (codigo) {
        case 4: // Botiquín
            posicion.y -= 12;  // Ajuste vertical: súbelo un poco
            return CrearSprite("sprites/botiquin.png", 1, 0.1f, posicion, SPRITE_OBJETO);
        case 5: // Enemigo
            posicion.y -= 15;  // Ajuste vertical para el enemigo
            return CrearSprite("sprites/enemigo_pelea.png", 1, 0.15f, posicion, SPRITE_ENEMIGO);
        case 8: // Fuego
            posicion.y -= 12;  // Ajuste vertical para el fuego
            return CrearSprite("sprites/fuego.png", 1, 0.1f, posicion, SPRITE_OBJETO);
        //case 6: // Electricidad
            //return CrearSprite("sprites/electricidad.png", 1, 0.1f, posicion, SPRITE_OBJETO);

        default:
            return NULL;
    }
}

void ProcesarMapaParaSprites(int** matrizMapa, int filas, int columnas) {
    // Limpia lista anterior
    LimpiarSpritesActivos();

    if (!spritesActivos)
        spritesActivos = list_create();

    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            int tile = matrizMapa[i][j];
            Vector2 pos = {j * TILE_SIZE, i * TILE_SIZE};

            Sprite* nuevoSprite = CrearSpriteDesdeCodigo(tile, pos);
            if (nuevoSprite)
                list_pushBack(spritesActivos, nuevoSprite);
        }
    }
}

void EliminarSpritePorPosicion(List* lista, Vector2 pos) {
    if (lista == NULL || lista->head == NULL) return;

    Node* current = lista->head;
    Node* prev = NULL;

    while (current != NULL) {
        Sprite* s = (Sprite*)current->data;
        if (fabs(s->position.x - pos.x) < 1 && fabs(s->position.y - pos.y) < 1) {
            Node* next = current->next;

            if (prev == NULL) {
                lista->head = next;
                if (next == NULL) lista->tail = NULL;
            } else {
                prev->next = next;
                if (next == NULL) lista->tail = prev;
            }

            LiberarSprite(s);
            free(current);
            lista->size--;
            break;
        }
        prev = current;
        current = current->next;
    }
}