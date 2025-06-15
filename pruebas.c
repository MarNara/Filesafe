#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>

#define MAPA_ANCHO 10
#define MAPA_ALTO 8

int base_ancho = 800;
int base_alto = 600;

// Estructura para el jugador
typedef struct {
    int x;
    int y;
} Jugador;

// Variables globales
typedef enum Pantalla { MENU, GAMEPLAY } Pantalla;
Pantalla pantallaDeJuego = GAMEPLAY;

// Funciones
void CargarMapa(int mapa[MAPA_ALTO][MAPA_ANCHO], Jugador *jugador);
void DibujarMapa(int mapa[MAPA_ALTO][MAPA_ANCHO], float scaleX, float scaleY, Jugador jugador);
void ActualizarGameplay(int mapa[MAPA_ALTO][MAPA_ANCHO], Jugador *jugador);

int main() {
    InitWindow(base_ancho, base_alto, "Juego con mapa CSV y jugador");

    int mapa[MAPA_ALTO][MAPA_ANCHO];
    Jugador jugador;

    CargarMapa(mapa, &jugador);

    float scaleX = 1.0f;
    float scaleY = 1.0f;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // Actualización
        switch (pantallaDeJuego) {
            case GAMEPLAY: ActualizarGameplay(mapa, &jugador); break;
        }

        // Dibujo
        BeginDrawing();
        ClearBackground(RAYWHITE);

        switch (pantallaDeJuego) {
            case GAMEPLAY: DibujarMapa(mapa, scaleX, scaleY, jugador); break;
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

void CargarMapa(int mapa[MAPA_ALTO][MAPA_ANCHO], Jugador *jugador) {
    FILE *archivo = fopen("mapa.csv", "r");
    if (archivo == NULL) {
        printf("Error al abrir el archivo\n");
        exit(1);
    }

    for (int i = 0; i < MAPA_ALTO; i++) {
        for (int j = 0; j < MAPA_ANCHO; j++) {
            fscanf(archivo, "%d,", &mapa[i][j]);
            if (mapa[i][j] == 9) {
                jugador->x = j;
                jugador->y = i;
            }
        }
    }

    fclose(archivo);
}

void ActualizarGameplay(int mapa[MAPA_ALTO][MAPA_ANCHO], Jugador *jugador) {
    int nuevaX = jugador->x;
    int nuevaY = jugador->y;

    if (IsKeyPressed(KEY_RIGHT)) nuevaX++;
    if (IsKeyPressed(KEY_LEFT))  nuevaX--;
    if (IsKeyPressed(KEY_UP))    nuevaY--;
    if (IsKeyPressed(KEY_DOWN))  nuevaY++;

    // Evitar salir del mapa
    if (nuevaX >= 0 && nuevaX < MAPA_ANCHO && nuevaY >= 0 && nuevaY < MAPA_ALTO) {
        int tile = mapa[nuevaY][nuevaX];
        // Colisiones con suelo (1), escombro (4), puerta cerrada (5)
        if (tile != 1 && tile != 4 && tile != 5) {
            jugador->x = nuevaX;
            jugador->y = nuevaY;
        }
    }
}

void DibujarMapa(int mapa[MAPA_ALTO][MAPA_ANCHO], float scaleX, float scaleY, Jugador jugador) {
    int tileWidth = (base_ancho / MAPA_ANCHO) * scaleX;
    int tileHeight = (base_alto / MAPA_ALTO) * scaleY;

    for (int i = 0; i < MAPA_ALTO; i++) {
        for (int j = 0; j < MAPA_ANCHO; j++) {
            Color color;
            switch(mapa[i][j]) {
                case 0: color = GRAY; break;        // Aire
                case 1: color = DARKGRAY; break;    // Suelo
                case 2: color = RED; break;         // Enemigo
                case 3: color = YELLOW; break;      // Trampa
                case 4: color = BROWN; break;       // Escombro
                case 5: color = BLUE; break;        // Puerta cerrada
                case 6: color = GREEN; break;       // Consola
                case 7: color = PURPLE; break;      // Escalera
                case 8: color = SKYBLUE; break;     // Item
                case 9: color = BLACK; break;       // Punto inicial
                default: color = WHITE; break;
            }
            DrawRectangle(j * tileWidth, i * tileHeight, tileWidth, tileHeight, color);
        }
    }

    // Dibuja jugador encima
    DrawRectangle(jugador.x * tileWidth, jugador.y * tileHeight, tileWidth, tileHeight, MAGENTA);
}