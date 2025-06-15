#include "raylib.h"
#include "tdas/list.h"
#include "tdas/extra.h"
#include "tdas/grafo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAPA_ANCHO 40
#define MAPA_ALTO 10
#define GRAVEDAD 0.9f
#define VELOCIDAD_SALTO -10.0f
#define VELOCIDAD_MAX_CAIDA 10.0f

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
    float y;
    float velocidad_Y; // Velocidad de movimiento
    int enElSuelo; // Si está en el suelo o no
} Personaje;

const int base_ancho = 1280;
const int base_alto = 900;

NodoMapa* mapaActual = NULL; // Mapa donde estás actualmente
int mapa[MAPA_ALTO][MAPA_ANCHO]; // Matriz del mapa cargado


GameScreen pantallaDeJuego = MENU;
int opcionSelecionada = 0;
Personaje jugador = {.nombre = "", .vida = 100, .ataque = 10, .defensa = 5, .velocidad_Y = 0, .enElSuelo = 0};

void CargarMapa(const char* nombreArchivo, int mapa[MAPA_ALTO][MAPA_ANCHO]) 
{
    FILE* archivo = fopen(nombreArchivo, "r");
    if (!archivo) {
        printf("No se pudo abrir el archivo %s\n", nombreArchivo);
        return;
    }

    for (int y = 0; y < MAPA_ALTO; y++) {
        for (int x = 0; x < MAPA_ANCHO; x++) {
            if (fscanf(archivo, "%d,", &mapa[y][x]) != 1) {
                mapa[y][x] = 0; // valor por defecto si hay error
            }
        }
    }
    fclose(archivo);
}

void DibujarMapa(int mapa[MAPA_ALTO][MAPA_ANCHO], float scaleX, float scaleY)
{
    for (int y = 0; y < MAPA_ALTO; y++) {
        for (int x = 0; x < MAPA_ANCHO; x++) {
            Rectangle tile = { x * 64 * scaleX, y * 64 * scaleY, 64 * scaleX, 64 * scaleY };
            switch (mapa[y][x]) {
                case 0: DrawRectangleRec(tile, BLACK); break;         // vacío
                case 1: DrawRectangleRec(tile, GRAY); break;          // plataforma
                case 2: DrawRectangleRec(tile, RED); break;           // enemigo
                case 3: DrawRectangleRec(tile, BLUE); break;          // trampa eléctrica
                case 4: DrawRectangleRec(tile, DARKGRAY); break;      // escombro
                case 5: DrawRectangleRec(tile, ORANGE); break;        // compuerta
                case 6: DrawRectangleRec(tile, GREEN); break;         // consola
                case 7: DrawRectangleRec(tile, PURPLE); break;        // escalera
                case 8: DrawRectangleRec(tile, GOLD); break;          // objeto
                default: DrawRectangleRec(tile, BLACK); break;        // por defecto
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

void ActualizarGameplay() 
{
    float deltaTime = GetFrameTime();

    // Movimiento horizontal
    int mover_x = jugador.x;
    if (IsKeyDown(KEY_RIGHT)) mover_x++;
    if (IsKeyDown(KEY_LEFT)) mover_x--;

    // Verificar colisión horizontal
    if (mover_x >= 0 && mover_x < MAPA_ANCHO) {
        if (mapa[(int)jugador.y][mover_x] == 0) { // Solo mover si está vacío
            jugador.x = mover_x;
        }
    }

    // Gravedad y salto
    jugador.velocidad_Y += GRAVEDAD * deltaTime;
    if (jugador.velocidad_Y > VELOCIDAD_MAX_CAIDA) {
        jugador.velocidad_Y = VELOCIDAD_MAX_CAIDA;
    }

    float nuevaY = jugador.y + jugador.velocidad_Y * deltaTime;

    // Colisión vertical
    if (nuevaY >= 0 && nuevaY < MAPA_ALTO) {
        if (mapa[(int)nuevaY][jugador.x] == 0) { // Si el nuevo espacio está vacío
            jugador.y = nuevaY;
        } else {
            // Colisión con objeto sólido
            jugador.velocidad_Y = 0;
            jugador.enElSuelo = (jugador.velocidad_Y > 0); // Está en suelo si caía
        }
    }

    // Salto
    if (IsKeyPressed(KEY_UP) && jugador.enElSuelo) {
        jugador.velocidad_Y = VELOCIDAD_SALTO;
        jugador.enElSuelo = 0;
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

void DrawGameplay(float scaleX, float scaleY) 
{
  DibujarMapa(mapa, scaleX, scaleY);
    
    // Dibuja al jugador
    Rectangle playerRect = {
        jugador.x * 64 * scaleX,
        jugador.y * 64 * scaleY,
        64 * scaleX,
        64 * scaleY
    };
    
    // Cuerpo del personaje
    DrawRectangleRec(playerRect, SKYBLUE);
    
    // Cabeza (más pequeña)
    Rectangle head = {
        playerRect.x + playerRect.width * 0.25f,
        playerRect.y,
        playerRect.width * 0.5f,
        playerRect.height * 0.5f
    };
    DrawRectangleRec(head, YELLOW);
    
    // Nombre
    DrawText(jugador.nombre, 
             playerRect.x + playerRect.width/2 - MeasureText(jugador.nombre, 20)/2, 
             playerRect.y - 25, 
             20, WHITE);
}


int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(base_ancho, base_alto, "El inicio de una aventura");
    SetTargetFPS(60);

    Image image = LoadImage("base/Menu_incial.png");
    ImageResize(&image, base_ancho, base_alto);
    Texture2D Menu_inicial_imagen = LoadTextureFromImage(image);
    UnloadImage(image);

    NodoMapa* mapa1 = CrearMapa(1, "mapas/mapa1.csv");
    NodoMapa* mapa2 = CrearMapa(2, "mapas/mapa2.csv");
    NodoMapa* mapa3 = CrearMapa(3, "mapas/mapa3.csv");

    ConectarMapas(mapa1, mapa2, 2); // mapa1 Este -> mapa2
    ConectarMapas(mapa2, mapa1, 3); // mapa2 Oeste -> mapa1
    ConectarMapas(mapa2, mapa3, 2); // mapa2 Este -> mapa3
    ConectarMapas(mapa3, mapa2, 3); 

    mapaActual = mapa1; // Comenzamos en el primer mapa
    CargarMapa(mapaActual->archivoMapa, mapa);
    jugador.x = 1; // Posición inicial del jugador
    jugador.y = 1; // Posición inicial del jugador
    mapa[(int)jugador.y][jugador.x] = 9; // Marca la posición del jugador en el mapa

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
