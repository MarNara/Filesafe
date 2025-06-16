#include "raylib.h"
#include "tdas/list.h"
#include "tdas/grafo.h"
#include "tdas/extra.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define GRAVEDAD 0.9f
#define VELOCIDAD_SALTO -10.0f
#define VELOCIDAD_MAX_CAIDA 10.0f
#define VELOCIDAD_HORIZONTAL 5.0f // Velocidad en unidades de tile por segundo. Ajusta este valor.
// Un valor de 5.0f significa que el personaje se moverá 5 tiles por segundo.
// Si tus tiles son de 64x64 píxeles, esto es 5 * 64 = 320 píxeles por segundo.

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
    float x;
    float y;
    float velocidad_Y; // Velocidad de movimiento
    float velocidad_X; // Velocidad de movimiento horizontal
    int enElSuelo; // Si está en el suelo o no
} Personaje;

const int base_ancho = 1280;
const int base_alto = 900;

NodoMapa* mapaActual = NULL; // Mapa donde estás actualmente
int** mapa; // Matriz del mapa cargado


GameScreen pantallaDeJuego = MENU;
int opcionSelecionada = 0;
Personaje jugador = {.nombre = "",.vida = 100,.ataque = 10,.defensa = 5};

void CargarMapa(const char* nombreArchivo, int*** mapaPtr, int* ancho, int* alto) 
{
    FILE* archivo = fopen(nombreArchivo, "r");

    fscanf(archivo, "%d\n", ancho);
    fscanf(archivo, "%d\n", alto);

    *mapaPtr = (int**)malloc((*alto) * sizeof(int*));
    for (int y = 0;y < *alto; y++) 
    {
        (*mapaPtr)[y] = (int*)malloc((*ancho) * sizeof(int));
    }

    for (int y = 0; y < *alto; y++) 
    {
        for (int x = 0; x < *ancho; x++)
        {
            if ( x < (*ancho - 1)) 
            {
                fscanf(archivo, "%d,", &(*mapaPtr)[y][x]); // valor por defecto si hay error
            }
            else 
            {
                fscanf(archivo, "%d\n", &(*mapaPtr)[y][x]); // último valor de la fila
            }

            if ((*mapaPtr)[y][x] == 9)
            {
                jugador.x = x;
                jugador.y = y;
                (*mapaPtr)[y][x] = 0;
            }
        }
    }
    fclose(archivo);
}

void DibujarMapa(int** mapa,int ancho ,int alto, float scaleX, float scaleY)
{
    for (int y = 0; y < alto; y++) {
        for (int x = 0; x < ancho; x++) {
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

void ActualizarGameplay(int** mapa, int ancho, int alto)
{
    float deltaTime = GetFrameTime();

    // --- 1. Movimiento Horizontal ---
    jugador.velocidad_X = 0.0f; // Resetear velocidad horizontal

    if (IsKeyDown(KEY_RIGHT)) {
        jugador.velocidad_X = VELOCIDAD_HORIZONTAL;
    }
    if (IsKeyDown(KEY_LEFT)) {
        jugador.velocidad_X = -VELOCIDAD_HORIZONTAL;
    }

    float proximaX = jugador.x + jugador.velocidad_X * deltaTime;

    // Colisión Horizontal:
    // Determinar el tile que el jugador intentará ocupar horizontalmente
    int tileActualX = (int)jugador.x; // Tile actual del jugador
    int tileActualY = (int)jugador.y;

    int tileProximoX_derecha = (int)(proximaX + 0.99f); // Borde derecho del jugador
    int tileProximoX_izquierda = (int)proximaX; // Borde izquierdo del jugador

    // Asegurarse de que los índices no salgan de los límites del mapa
    if (tileProximoX_derecha >= ancho) tileProximoX_derecha = ancho - 1;
    if (tileProximoX_izquierda < 0) tileProximoX_izquierda = 0;
    
    // Asumimos que el jugador es de 1 tile de alto,
    // por lo que solo necesitamos revisar el tile en la misma fila (jugador.y)
    // para colisiones horizontales simples.

    // Colisión hacia la derecha
    if (jugador.velocidad_X > 0) { // Si se mueve a la derecha
        // ¿El tile al que el borde DERECHO del jugador se movería es sólido?
        if (tileProximoX_derecha < ancho && mapa[tileActualY][tileProximoX_derecha] != 0) {
            jugador.velocidad_X = 0; // Detener movimiento horizontal
            jugador.x = (float)tileProximoX_derecha - 1.0f; // Reposicionar justo al lado del bloque
        } else {
            jugador.x = proximaX; // No hay colisión, mover libremente
        }
    }
    // Colisión hacia la izquierda
    else if (jugador.velocidad_X < 0) { // Si se mueve a la izquierda
        // ¿El tile al que el borde IZQUIERDO del jugador se movería es sólido?
        if (tileProximoX_izquierda >= 0 && mapa[tileActualY][tileProximoX_izquierda] != 0) {
            jugador.velocidad_X = 0; // Detener movimiento horizontal
            jugador.x = (float)tileProximoX_izquierda + 1.0f; // Reposicionar justo al lado del bloque
        } else {
            jugador.x = proximaX; // No hay colisión, mover libremente
        }
    }
    // Si velocidad_X es 0, no hay movimiento horizontal, no se hace nada aquí.
    

    // --- 2. Movimiento Vertical (Gravedad y Salto) ---
    jugador.velocidad_Y += GRAVEDAD * deltaTime;
    if (jugador.velocidad_Y > VELOCIDAD_MAX_CAIDA) {
        jugador.velocidad_Y = VELOCIDAD_MAX_CAIDA;
    }

    float proximaY = jugador.y + jugador.velocidad_Y * deltaTime;

    // Colisión Vertical:
    // Determinar el tile que el jugador intentará ocupar verticalmente
    int tileProximoY_abajo = (int)(proximaY + 0.99f); // Borde inferior del jugador
    int tileProximoY_arriba = (int)proximaY; // Borde superior del jugador

    // Asegurarse de que los índices no salgan de los límites del mapa
    if (tileProximoY_abajo >= alto) tileProximoY_abajo = alto - 1;
    if (tileProximoY_arriba < 0) tileProximoY_arriba = 0;


    // Colisión hacia abajo (caída)
    if (jugador.velocidad_Y >= 0) { // Cayendo o en reposo vertical
        // ¿El tile al que el borde INFERIOR del jugador se movería es sólido?
        // Revisar el tile directamente debajo, y también el tile actual horizontalmente
        // Esto asume que el jugador es de 1 tile de ancho.
        // Si tienes un ancho de jugador real que abarca 2 tiles, tendrías que comprobar ambos.
        if (tileProximoY_abajo < alto && mapa[tileProximoY_abajo][(int)jugador.x] != 0) {
            jugador.velocidad_Y = 0; // Detener la caída
            jugador.y = (float)tileProximoY_abajo; // Reposicionar justo encima del bloque
            jugador.enElSuelo = 1; // Está en el suelo
        } else {
            jugador.y = proximaY; // No hay colisión, seguir cayendo
            jugador.enElSuelo = 0; // No está en el suelo (a menos que haya chocado con el techo y se detenga ahí)
        }
    }
    // Colisión hacia arriba (salto)
    else { // Subiendo
        // ¿El tile al que el borde SUPERIOR del jugador se movería es sólido (techo)?
        if (tileProximoY_arriba >= 0 && mapa[tileProximoY_arriba][(int)jugador.x] != 0) {
            jugador.velocidad_Y = 0; // Detener el salto
            jugador.y = (float)(tileProximoY_arriba + 1); // Reposicionar justo debajo del bloque
            jugador.enElSuelo = 0; // No está en el suelo
        } else {
            jugador.y = proximaY; // No hay colisión, seguir subiendo
            jugador.enElSuelo = 0;
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

void DrawGameplay(int** mapa, int ancho, int alto,float scaleX, float scaleY) 
{
  DibujarMapa(mapa,mapaActual->ancho, mapaActual->alto, scaleX, scaleY);
    
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
    SetTargetFPS(20);

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
    CargarMapa(mapaActual->archivoMapa, &mapa, &mapaActual->ancho, &mapaActual->alto);

    while (!WindowShouldClose()) {
        int pantalla_ancho = GetScreenWidth();
        int pantalla_alto = GetScreenHeight();
        float scaleX = (float)pantalla_ancho / base_ancho;
        float scaleY = (float)pantalla_alto / base_alto;

        // Actualización
        switch (pantallaDeJuego) {
            case MENU:
                ActualizarMenu(); 
                break;
            case PONER_NOMBRE:
                ActualizarPonerNombre();
                break;
            case GAMEPLAY:
                ActualizarGameplay(mapa,mapaActual->ancho, mapaActual->alto); 
                break;
        }

        // Dibujo
        BeginDrawing();
        ClearBackground((Color){ 50, 50, 50, 255 });

        switch (pantallaDeJuego) {
            case MENU: DrawMenu(Menu_inicial_imagen, scaleX, scaleY); break;
            case PONER_NOMBRE: DrawNameInput(scaleX, scaleY); break;
            case GAMEPLAY: DrawGameplay(mapa,mapaActual->ancho, mapaActual->alto,scaleX, scaleY); break;
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
