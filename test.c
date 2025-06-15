#include "raylib.h"
#include "tdas/list.h"
#include "tdas/extra.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAPA_ANCHO 10
#define MAPA_ALTO 8

typedef enum GameScreen 
{
    MENU = 0,GAMEPLAY,PONER_NOMBRE
} GameScreen;

typedef struct personaje 
{
    char nombre[9];
    int vida;
    int ataque;
    int defensa;
    int x;
    int y;
} Personaje;

const int base_ancho = 1280;
const int base_alto = 900;

GameScreen pantallaDeJuego = MENU;
int opcionSelecionada = 0;
Personaje jugador = {.nombre = "",.vida = 100,.ataque = 10,.defensa = 5};

int mapa[MAPA_ALTO][MAPA_ANCHO];

void CargarMapa(int mapa[MAPA_ALTO][MAPA_ANCHO]) 
{
    FILE *archivo = fopen("mapa.csv", "r");
    if (archivo == NULL) 
    {
        printf("Error al abrir el archivo del mapa.\n");
        return;
    }
    
    for (int i = 0; i < MAPA_ALTO; i++) 
    {
        for (int j = 0; j < MAPA_ANCHO; j++) 
        {
            fscanf(archivo, "%d,", &mapa[i][j]);
            if (mapa[i][j] == 9)
            {
                jugador.x = j; // Guardar la posición inicial del jugador
                jugador.y = i;
            }
        }
    }
    fclose(archivo);
}

void DibujarMapa(int mapa[MAPA_ALTO][MAPA_ANCHO], float scaleX, float scaleY) {
    for (int y = 0; y < MAPA_ALTO; y++) {
        for (int x = 0; x < MAPA_ANCHO; x++) {
            Rectangle tile = { x * 64 * scaleX, y * 64 * scaleY, 64 * scaleX, 64 * scaleY };
            
            switch (mapa[y][x]) {
                case 0: DrawRectangleRec(tile, BLACK); break;         // vacío
                case 1: DrawRectangleRec(tile, GRAY); break;         // plataforma
                case 2: DrawRectangleRec(tile, RED); break;          // enemigo
                case 3: DrawRectangleRec(tile, BLUE); break;         // trampa eléctrica
                case 4: DrawRectangleRec(tile, DARKGRAY); break;     // escombro
                case 5: DrawRectangleRec(tile, ORANGE); break;       // compuerta
                case 6: DrawRectangleRec(tile, GREEN); break;        // consola
                case 7: DrawRectangleRec(tile, PURPLE); break;       // escalera
                case 8: DrawRectangleRec(tile, GOLD); break;         // objeto
                case 9: DrawRectangleRec(tile, WHITE); break;        // jugador inicio
            }
        }
    }
}

void ActualizarMenu() {
    if (IsKeyPressed(KEY_DOWN)) opcionSelecionada++;
    if (IsKeyPressed(KEY_UP)) opcionSelecionada--;
    if (opcionSelecionada > 1) opcionSelecionada = 0;
    if (opcionSelecionada < 0) opcionSelecionada = 1;

    if (IsKeyPressed(KEY_ENTER)) {
        if (opcionSelecionada == 0) pantallaDeJuego = PONER_NOMBRE;
        else if (opcionSelecionada == 1) CloseWindow();
    }
}

void ActualizarPonerNombre() {
    int key = GetCharPressed();
    int len = strlen(jugador.nombre);

    while (key > 0) {
        if (((key >= 'a') && (key <= 'z')) || ((key >= 'A') && (key <= 'Z'))) {
            if (len < 8) { // para no pasar el límite
                jugador.nombre[len] = (char)key;
                jugador.nombre[len + 1] = '\0';
                len++;
            }
        }
        key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE) && len > 0) {
        jugador.nombre[len - 1] = '\0';
    }

    if (IsKeyPressed(KEY_ENTER) && len > 0) {
        pantallaDeJuego = GAMEPLAY;
    }
}

void ActualizarGameplay() {
    int mover_x = jugador.x;
    int mover_y = jugador.y;
    if (IsKeyDown(KEY_RIGHT)) mover_x++;
    if (IsKeyDown(KEY_LEFT)) mover_x--;
    if (IsKeyDown(KEY_DOWN)) mover_y++;
    if (IsKeyDown(KEY_UP)) mover_y--;

    if (mover_x >= 0 && mover_x < MAPA_ANCHO && mover_y >= 0 && mover_y < MAPA_ALTO) 
    {
        if (mapa[mover_y][mover_x] != 1 && mapa[mover_y][mover_x] != 4 && mapa[mover_y][mover_x] != 5) { // solo se puede mover a espacios vacíos o plataformas
            mapa[jugador.y][jugador.x] = 0; // limpiar la posición anterior
            jugador.x = mover_x;
            jugador.y = mover_y;
            mapa[jugador.y][jugador.x] = 9; // actualizar la posición del jugador
        }
    }
}

void DrawMenu(Texture2D menuTexture, float scaleX, float scaleY) {
    DrawTextureEx(menuTexture, (Vector2){0, 0}, 0.0f, scaleX, WHITE);
    DrawText("MENU PRINCIPAL", scaleX * 480, scaleY * 200, scaleY * 40, WHITE);
    DrawText("Iniciar Juego", scaleX * 540, scaleY * 300, scaleY * 30, opcionSelecionada == 0 ? RED : WHITE);
    DrawText("Salir", scaleX * 540, scaleY * 350, scaleY * 30, opcionSelecionada == 1 ? RED : WHITE);
    DrawText("Usa Flechas ARRIBA/ABAJO y ENTER para elegir", scaleX * 360, scaleY * 500, scaleY * 20, WHITE);
}

void DrawNameInput(float scaleX, float scaleY) {
    DrawText("INGRESA TU NOMBRE:", scaleX * 400, scaleY * 200, scaleY * 30, WHITE);
    DrawText(jugador.nombre, scaleX * 400, scaleY * 250, scaleY * 30, PINK);
    DrawText("Presiona ENTER para continuar", scaleX * 400, scaleY * 300, scaleY * 20, WHITE);
}

void DrawGameplay(float scaleX, float scaleY) {
    DrawText("JUEGO INICIADO!", scaleX * 500, scaleY * 400, scaleY * 40, DARKGREEN);
    DrawText(TextFormat("Jugador: %s", jugador.nombre), scaleX * 500, scaleY * 450, scaleY * 20, DARKGREEN);
    DrawText("Presiona P para volver al menu", scaleX * 360, scaleY * 500, scaleY * 20, DARKGREEN);
    DibujarMapa(mapa, scaleX, scaleY);
}



int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(base_ancho, base_alto, "El inicio de una aventura");
    SetTargetFPS(20);

    Image image = LoadImage("base/Menu_incial.png");
    ImageResize(&image, base_ancho, base_alto);
    Texture2D Menu_inicial_imagen = LoadTextureFromImage(image);
    UnloadImage(image);

    CargarMapa(mapa);

    while (!WindowShouldClose()) {
        int pantalla_ancho = GetScreenWidth();
        int pantalla_alto = GetScreenHeight();
        float scaleX = (float)pantalla_ancho / base_ancho;
        float scaleY = (float)pantalla_alto / base_alto;

        // Actualización
        switch (pantallaDeJuego) {
            case MENU: ActualizarMenu(); break;
            case PONER_NOMBRE: ActualizarPonerNombre(); break;
            case GAMEPLAY: ActualizarGameplay(); break;
        }

        // Dibujo
        BeginDrawing();
        ClearBackground((Color){ 50, 50, 50, 255 });

        switch (pantallaDeJuego) {
            case MENU: DrawMenu(Menu_inicial_imagen, scaleX, scaleY); break;
            case PONER_NOMBRE: DrawNameInput(scaleX, scaleY); break;
            case GAMEPLAY: DrawGameplay(scaleX, scaleY); break;
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
