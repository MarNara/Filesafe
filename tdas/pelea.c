#include "raylib.h"
#include "hashmap.h"
#include "pelea.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
static double ultimaAccion = 0;

typedef enum {
    OPCION_LUCHAR = 0,
    OPCION_MOCHILA,
    OPCION_LIBERAR,
    OPCION_TOTAL
} OpcionMenu;

typedef struct {
    char nombre[20];
    int cantidad;
    int cura;
} Item;

Item *crearObjeto(const char *nombre, int cura) {
    Item *item = malloc(sizeof(Item));
    if (item == NULL) {
        perror("Fallo al asignar memoria para Item");
        exit(EXIT_FAILURE);
    }
    strcpy(item->nombre, nombre);
    item->cura = cura;
    return item;
}

void jugadorAtaca(Combatiente *jugador, Combatiente *enemigo) {
    enemigo->vida -= 10;
    if (enemigo->vida < 0) enemigo->vida = 0;
}

void enemigoAtaca(Combatiente *enemigo, Combatiente *jugador) {
    jugador->vida -= 5;
    if (jugador->vida < 0) jugador->vida = 0;
}

void mostrarInventario(HashMap* inventario, Combatiente* jugador, bool* mostrando, bool* turnoJugador) {
    static int seleccion = 0;
    static bool enterBloqueado = false;
    int totalItems = 0;

    Rectangle rectInventario = {200, 150, 400, 300};
    Rectangle botonCerrar = {rectInventario.x + rectInventario.width - 30, rectInventario.y + 10, 20, 20};

    DrawRectangleRec(rectInventario, LIGHTGRAY);
    DrawRectangleLinesEx(rectInventario, 2, GRAY);
    DrawText("Inventario", 320, 170, 25, BLACK);

    if (firstMap(inventario) == NULL) {
        DrawText("No tienes objetos.", rectInventario.x + 120, 250, 20, DARKGRAY);
    }

    DrawRectangleRec(botonCerrar, RED);
    DrawText("X", botonCerrar.x + 5, botonCerrar.y + 2, 18, WHITE);

    // Contar ítems
    for (Pair* p = firstMap(inventario); p != NULL; p = nextMap(inventario)) {
        totalItems++;
    }

    if (seleccion >= totalItems) seleccion = 0;

    if (IsKeyPressed(KEY_DOWN)) seleccion = (seleccion + 1) % totalItems;
    if (IsKeyPressed(KEY_UP)) seleccion = (seleccion - 1 + totalItems) % totalItems;

    if (IsKeyPressed(KEY_DELETE)) {
        *mostrando = false;
        enterBloqueado = false;
        return;
    }

    int y = 220;
    int i = 0;
    Pair* par = firstMap(inventario);
    while (par != NULL) {
        Item* item = (Item*)par->value;
        char nombreConCantidad[64];
        snprintf(nombreConCantidad, sizeof(nombreConCantidad), "%s x%d", par->key, item->cantidad);

        Rectangle slot = {rectInventario.x + 20, y, 200, 30};
        DrawRectangleRec(slot, i == seleccion ? GRAY : LIGHTGRAY);
        DrawRectangleLinesEx(slot, 1, DARKGRAY);
        DrawText(nombreConCantidad, slot.x + 5, slot.y + 5, 18, BLACK);

        if (i == seleccion) {
            if (IsKeyDown(KEY_ENTER)) {
                if (!enterBloqueado) {
                    jugador->vida += item->cura;
                    if (jugador->vida > 100) jugador->vida = 100;

                    item->cantidad--;
                    if (item->cantidad <= 0) {
                        free(item);
                        eraseMap(inventario, par->key);
                    }

                    *mostrando = false;
                    *turnoJugador = false;
                    seleccion = 0;
                    enterBloqueado = true;
                    return;
                }
            } else {
                enterBloqueado = false;
            }
        }

        y += 40;
        par = nextMap(inventario);
        i++;
    }
}


bool iniciar_pelea(Combatiente *jugador, Combatiente *enemigo, HashMap* inventario) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Combate");
    MaximizeWindow();
    Texture2D texFondo = LoadTexture("sprites/personaje/escenario_de_combate.png");
    Texture2D texPersonaje = LoadTexture("sprites/personaje/combate de lado.png");
    Texture2D texEnemigo = LoadTexture("sprites/enemigo_pelea.png");

    SetTargetFPS(60);

    Camera2D cam = {0};
    cam.zoom = 1.0f;

    bool batallaActiva = true;
    bool mostrandoInventario = false;
    bool enemigoLiberado = false;
    OpcionMenu seleccion = OPCION_LUCHAR;

    bool turnoJugador = true;
    bool animandoAtaque = false;
    bool animandoJugador = false;
    int animFrame = 0;
    float animOffsetPlayerX = 0;
    float animOffsetEnemyX = 0;

    const float ANCHO_JUGADOR = 90.0f;
    const float ALTO_JUGADOR = 150.0f;
    const float ANCHO_ENEMIGO = 150.0f;
    const float ALTO_ENEMIGO = 150.0f;

    while (!WindowShouldClose() && !IsKeyPressed(KEY_ESCAPE)) {
        int width = GetScreenWidth();
        int height = GetScreenHeight();
        float scaleX = (float)width / SCREEN_WIDTH;
        float scaleY = (float)height / SCREEN_HEIGHT;
        cam.zoom = (scaleX < scaleY) ? scaleX : scaleY;

        if (animandoAtaque) {
            animFrame++;
            float avance = 30.0f;
            int duracionTotal = 30;

            if (animFrame <= duracionTotal / 2) {
                if (animandoJugador)
                    animOffsetPlayerX = animFrame * avance;
                else
                    animOffsetEnemyX = -animFrame * avance;
            } else if (animFrame <= duracionTotal) {
                if (animandoJugador)
                    animOffsetPlayerX = (duracionTotal - animFrame) * avance;
                else
                    animOffsetEnemyX = -(duracionTotal - animFrame) * avance;
            } else {
                if (animandoJugador)
                    jugadorAtaca(jugador, enemigo);
                else
                    enemigoAtaca(enemigo, jugador);

                animandoAtaque = false;
                animOffsetPlayerX = 0;
                animOffsetEnemyX = 0;
                turnoJugador = !animandoJugador;
            }
        }

        if (batallaActiva && !mostrandoInventario && !animandoAtaque) {
            if (turnoJugador) {
                if (IsKeyPressed(KEY_DOWN)) seleccion = (seleccion + 1) % OPCION_TOTAL;
                if (IsKeyPressed(KEY_UP)) seleccion = (seleccion - 1 + OPCION_TOTAL) % OPCION_TOTAL;

                if (IsKeyPressed(KEY_ENTER)) {
                    switch (seleccion) {
                        case OPCION_LUCHAR:
                            animandoAtaque = true;
                            animandoJugador = true;
                            animFrame = 0;
                            break;
                        case OPCION_MOCHILA:
                            mostrandoInventario = true;
                            break;
                        case OPCION_LIBERAR:
                            if (enemigo->vida <= 10) {
                                enemigoLiberado = true;
                                batallaActiva = false;
                            }
                            if (!enemigoLiberado) {
                                turnoJugador = true;
                            } else {
                                turnoJugador = false;
                            }
                            break;
                    }
                }
            } else {
                if (batallaActiva) {
                    animandoAtaque = true;
                    animandoJugador = false;
                    animFrame = 0;
                }
            }
        }

        if (jugador->vida <= 0 || enemigo->vida <= 0 || enemigoLiberado) {
            batallaActiva = false;
        }

        BeginDrawing();
        DrawTexturePro(
            texFondo,
            (Rectangle){0, 0, texFondo.width, texFondo.height},
            (Rectangle){0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()},
            (Vector2){0, 0},
            0.0f,
            WHITE
        );

        BeginMode2D(cam);

        Rectangle rectJugador = {100 + animOffsetPlayerX, 220, ANCHO_JUGADOR, ALTO_JUGADOR};
        Rectangle rectEnemigo = {600 + animOffsetEnemyX, 220, ANCHO_ENEMIGO, ALTO_ENEMIGO};

        DrawTexturePro(texPersonaje, (Rectangle){0, 0, texPersonaje.width, texPersonaje.height}, rectJugador, (Vector2){0, 0}, 0.0f, WHITE);
        DrawTexturePro(texEnemigo, (Rectangle){0, 0, texEnemigo.width, texEnemigo.height}, rectEnemigo, (Vector2){0, 0}, 0.0f, WHITE);

        DrawText(TextFormat("%s - Vida: %d", jugador->nombre, jugador->vida), 100, 100, 20, BLUE);
        DrawText(TextFormat("%s - Vida: %d", enemigo->nombre, enemigo->vida), 500, 100, 20, RED);

        if (!mostrandoInventario && batallaActiva && !animandoAtaque) {
            DrawRectangle(80, 380, 250, 170, LIGHTGRAY);
            DrawRectangleLines(80, 380, 250, 170, GRAY);
            DrawText("Elige tu accion:", 100, 400, 20, BLACK);
            DrawText("LUCHAR", 100, 440, 20, seleccion == OPCION_LUCHAR ? RED : BLACK);
            DrawText("MOCHILA", 100, 480, 20, seleccion == OPCION_MOCHILA ? RED : BLACK);
            DrawText("LIBERAR", 100, 520, 20, seleccion == OPCION_LIBERAR ? RED : BLACK);
        }

        if (mostrandoInventario) {
            mostrarInventario(inventario, jugador, &mostrandoInventario, &turnoJugador);
        }

        if (!batallaActiva) {
            if (jugador->vida <= 0)
                DrawText("Has perdido!", 300, 300, 30, RED);
            else if (enemigo->vida <= 0)
                DrawText("Has ganado!", 300, 300, 30, GREEN);
            else if (enemigoLiberado)
                DrawText("¡Has liberado a esta persona!", 225, 300, 25, WHITE);

            DrawText("Presiona ESC para salir...", 250, 400, 20, DARKGRAY);
        }

        EndMode2D();
        EndDrawing();
    }

    UnloadTexture(texFondo);
    UnloadTexture(texPersonaje);
    UnloadTexture(texEnemigo);
    CloseWindow();

    if (enemigo->vida <= 0) return true;
    if (jugador->vida <= 0) return false;
    return false;
}