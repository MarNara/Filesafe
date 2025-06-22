#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "tdas/list.h"
#include "tdas/stack.h"

typedef enum { MENU_ATAQUE, MENU_MOCHILA, MENU_CAMBIAR_ARMA, MENU_LIBERAR } MenuOpcion;
typedef enum { TURNO_JUGADOR, TURNO_ENEMIGO } Turno;

typedef struct {
    char nombre[32];
    int nivel;
    int vida;
    int ataque;
    int defensa;
} Personaje;

typedef struct {
    char nombre[32];
    int bonificacion_ataque;
} Arma;

Personaje jugador = {"Jugador", 10, 100, 20, 5};
Personaje *enemigo = NULL; // Puntero al enemigo actual
List *colaEnemigos; // Cola de enemigos

Arma armas[2] = { {"Espada", 5}, {"Lanza", 10} };
int armaActual = 0;
MenuOpcion opcionActual = MENU_ATAQUE;
Turno turno = TURNO_JUGADOR;

char mensaje[128] = "";
bool enCombate = true;

// Carga enemigos desde el CSV y los inserta en la cola
void CargarEnemigosCSV(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error al abrir %s\n", filename);
        exit(1);
    }
    char line[128];
    fgets(line, sizeof(line), file); // Saltar encabezado

    while (fgets(line, sizeof(line), file)) {
        Personaje *nuevo = malloc(sizeof(Personaje));
        sscanf(line, "%[^,],%d,%d,%d,%d",
               nuevo->nombre,
               &nuevo->nivel,
               &nuevo->vida,
               &nuevo->ataque,
               &nuevo->defensa);
        list_pushBack(colaEnemigos, nuevo);
    }
    fclose(file);
}

// Trae el siguiente enemigo de la cola
void SiguienteEnemigo() {
    enemigo = list_popFront(colaEnemigos);
    if (!enemigo) {
        snprintf(mensaje, sizeof(mensaje), "¡Ganaste el combate!");
        enCombate = false;
    }
}

void EnemigoAtaca() {
    int danio = enemigo->ataque - jugador.defensa;
    if (danio < 1) danio = 1;
    jugador.vida -= danio;
    snprintf(mensaje, sizeof(mensaje), "%s ataca y causa %d daño!", enemigo->nombre, danio);
    if (jugador.vida < 0) jugador.vida = 0;
}

int main(void) {
    const int screenWidth = 800, screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Combate RPG Mejorado");
    SetTargetFPS(60);
    srand(time(NULL));

    colaEnemigos = list_create();
    CargarEnemigosCSV("enemigos.csv");
    SiguienteEnemigo(); // Primer enemigo

    while (!WindowShouldClose()) {
        if (enCombate) {
            if (turno == TURNO_JUGADOR) {
                if (IsKeyPressed(KEY_RIGHT)) opcionActual = (opcionActual + 1) % 4;
                if (IsKeyPressed(KEY_LEFT)) opcionActual = (opcionActual + 3) % 4;
                
                if (IsKeyPressed(KEY_ENTER)) {
                    switch (opcionActual) {
                        case MENU_ATAQUE: {
                            int danio = jugador.ataque + armas[armaActual].bonificacion_ataque - enemigo->defensa;
                            if (danio < 1) danio = 1;
                            enemigo->vida -= danio;
                            snprintf(mensaje, sizeof(mensaje), "Atacas con %s y causas %d daño!", armas[armaActual].nombre, danio);
                            if (enemigo->vida <= 0) {
                                enemigo->vida = 0;
                                free(enemigo);
                                SiguienteEnemigo();
                            }
                            turno = TURNO_ENEMIGO;
                            break;
                        }
                        case MENU_MOCHILA:
                            snprintf(mensaje, sizeof(mensaje), "Tu mochila está vacía.");
                            break;
                        case MENU_CAMBIAR_ARMA:
                            armaActual = (armaActual + 1) % 2;
                            snprintf(mensaje, sizeof(mensaje), "Cambiaste a %s!", armas[armaActual].nombre);
                            break;
                        case MENU_LIBERAR:
                            if (enemigo->vida < 30) {
                                snprintf(mensaje, sizeof(mensaje), "¡Liberaste al enemigo!");
                                free(enemigo);
                                SiguienteEnemigo();
                            } else {
                                snprintf(mensaje, sizeof(mensaje), "¡El enemigo es muy fuerte aún!");
                            }
                            break;
                    }
                }
            } else if (turno == TURNO_ENEMIGO) {
                EnemigoAtaca();
                turno = TURNO_JUGADOR;
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (enemigo) {
            // Enemigo
            DrawText(TextFormat("%s Nv%d", enemigo->nombre, enemigo->nivel), 500, 50, 20, RED);
            DrawRectangle(500, 80, enemigo->vida, 10, RED);
            DrawText(TextFormat("Vida: %d", enemigo->vida), 500, 100, 20, BLACK);
            DrawText(TextFormat("Atq: %d Def: %d", enemigo->ataque, enemigo->defensa), 500, 130, 20, BLACK);
            DrawCircle(550, 200, 40, LIGHTGRAY); 
        }

        // Jugador
        DrawText(TextFormat("%s Nv%d", jugador.nombre, jugador.nivel), 50, 350, 20, BLUE);
        DrawRectangle(50, 380, jugador.vida, 10, GREEN);
        DrawText(TextFormat("Vida: %d", jugador.vida), 50, 400, 20, BLACK);
        DrawText(TextFormat("Atq: %d Def: %d", jugador.ataque, jugador.defensa), 50, 430, 20, BLACK);
        DrawText(TextFormat("Arma: %s", armas[armaActual].nombre), 50, 460, 20, DARKGRAY);
        DrawCircle(100, 500, 40, GRAY);

        DrawText(mensaje, 50, 520, 20, BLACK);

        const char *opciones[] = {"Atacar", "Mochila", "Cambiar Arma", "Liberar"};
        for (int i = 0; i < 4; i++) {
            Color color = (i == opcionActual) ? ORANGE : DARKGRAY;
            DrawText(opciones[i], 200 + i * 120, 550, 20, color);
        }

        if (!enCombate)
            DrawText("Combate Finalizado", screenWidth / 2 - 100, screenHeight / 2, 20, GREEN);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
