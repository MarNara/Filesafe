#include "raylib.h"
#include "hashmap.h"
#include "pelea.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
// static double ultimaAccion = 0; // Esta variable no se usa actualmente.

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

// Función auxiliar para contar ítems en el HashMap
int ContarItemsInventario(HashMap* inventario) {
    int count = 0;
    Pair* par = firstMap(inventario);
    while (par != NULL) {
        count++;
        par = nextMap(inventario);
    }
    return count;
}

// Función auxiliar para obtener un ítem por su posición
Pair* ObtenerItemEnPosicionInventario(HashMap* inventario, int pos) {
    Pair* par = firstMap(inventario);
    int i = 0;
    while (par != NULL) {
        if (i == pos) return par;
        par = nextMap(inventario);
        i++;
    }
    return NULL;
}

// Se añadió 'seleccionMenuPelea' para permitir que el inventario afecte la selección del menú principal de combate.
void mostrarInventario(HashMap* inventario, Combatiente* jugador, bool* mostrando, bool* turnoJugador, OpcionMenu* seleccionMenuPelea) {
    static int seleccionItemInventario = 0; // Renombrado para evitar conflicto y ser más específico.
    static bool enterBloqueado = false;
    int totalItems = ContarItemsInventario(inventario); // Usa función auxiliar.

    Rectangle rectInventario = {200, 150, 400, 300};
    // El botón cerrar ha sido retirado, ahora se usa la tecla 'M' para salir.

    DrawRectangleRec(rectInventario, LIGHTGRAY);
    DrawRectangleLinesEx(rectInventario, 2, GRAY);
    DrawText("Inventario", 320, 170, 25, BLACK);

    if (totalItems == 0) {
        DrawText("No tienes objetos.", rectInventario.x + 120, 250, 20, DARKGRAY);
        // Si no hay ítems, la única opción es salir con 'M'.
        if (IsKeyPressed(KEY_M)) {
            *mostrando = false;
            enterBloqueado = false;
            *turnoJugador = true; // No se consume el turno si no se usa un ítem.
            return;
        }
        return;
    }

    // Asegura que la selección sea válida si los ítems cambian.
    if (seleccionItemInventario >= totalItems) {
        seleccionItemInventario = totalItems - 1;
        if (seleccionItemInventario < 0) seleccionItemInventario = 0;
    }

    if (IsKeyPressed(KEY_DOWN)) seleccionItemInventario = (seleccionItemInventario + 1) % totalItems;
    if (IsKeyPressed(KEY_UP)) seleccionItemInventario = (seleccionItemInventario - 1 + totalItems) % totalItems;

    // Permite salir del inventario con la tecla 'M'.
    if (IsKeyPressed(KEY_M)) {
        *mostrando = false;
        enterBloqueado = false;
        *seleccionMenuPelea = OPCION_MOCHILA; // Vuelve a la opción Mochila en el menú principal.
        *turnoJugador = true; // No se consume el turno.
        return;
    }

    int y = 220;
    // Itera y dibuja los ítems del inventario.
    for (int i = 0; i < totalItems; i++) {
        Pair* par = ObtenerItemEnPosicionInventario(inventario, i);
        if (par == NULL) continue;

        Item* item = (Item*)par->value;
        char nombreConCantidad[64];
        snprintf(nombreConCantidad, sizeof(nombreConCantidad), "%s x%d (Cura: %d)", item->nombre, item->cantidad, item->cura);

        Rectangle slot = {rectInventario.x + 20, y, 360, 30}; // Aumentado el ancho para mostrar la cura.
        DrawRectangleRec(slot, i == seleccionItemInventario ? GRAY : LIGHTGRAY);
        DrawRectangleLinesEx(slot, 1, DARKGRAY);
        DrawText(nombreConCantidad, slot.x + 5, slot.y + 5, 18, BLACK);

        if (i == seleccionItemInventario) {
            if (IsKeyPressed(KEY_N)) { // <-- Aquí solo cambio la tecla para usar el ítem a 'N'
                if (!enterBloqueado) {
                    jugador->vida += item->cura;
                    if (jugador->vida > 100) jugador->vida = 100;

                    item->cantidad--;
                    if (item->cantidad <= 0) {
                        free(item); // Libera la memoria del ítem.
                        eraseMap(inventario, par->key); // Elimina el ítem del HashMap.
                        // Ajusta la selección si el total de ítems cambia.
                        totalItems = ContarItemsInventario(inventario);
                        if (seleccionItemInventario >= totalItems && totalItems > 0) {
                            seleccionItemInventario = totalItems - 1;
                        } else if (totalItems == 0) {
                            seleccionItemInventario = 0;
                        }
                    }

                    *mostrando = false; // Cierra el inventario.
                    *turnoJugador = false; // Consume el turno del jugador.
                    enterBloqueado = true;
                    *seleccionMenuPelea = OPCION_LUCHAR; // Regresa a la opción Luchar del menú principal.
                    return;
                }
            } else {
                enterBloqueado = false;
            }
        }
        y += 40;
    }
    // Instrucciones de uso en el inventario actualizadas.
    DrawText("N: usar item", rectInventario.x + 20, rectInventario.y + rectInventario.height - 40, 15, BLACK);
    DrawText("M: volver al combate", rectInventario.x + 20, rectInventario.y + rectInventario.height - 20, 15, BLACK);
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
    OpcionMenu seleccion = OPCION_LUCHAR; // Variable para la selección del menú de combate.

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

    while (!WindowShouldClose()) { // Se quitó la condición IsKeyPressed(KEY_ESCAPE) para manejo externo.
        int width = GetScreenWidth();
        int height = GetScreenHeight();
        float scaleX = (float)width / SCREEN_WIDTH;
        float scaleY = (float)height / SCREEN_HEIGHT;
        cam.zoom = (scaleX < scaleY) ? scaleX : scaleY;

        // Permite salir de la batalla con ESC una vez que ha terminado.
        if (!batallaActiva && IsKeyPressed(KEY_ESCAPE)) {
            break;
        }

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
                turnoJugador = !animandoJugador; // Cambia el turno después de la animación.
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
                            } else {
                                // El enemigo no está lo suficientemente débil. El turno del jugador no cambia.
                            }
                            break;
                    }
                }
            } else { // Turno del enemigo
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
        ClearBackground(BLACK);
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
            mostrarInventario(inventario, jugador, &mostrandoInventario, &turnoJugador, &seleccion);
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
    if (enemigoLiberado) return true;
    return false;
}