#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>

#define TILE_SIZE 60
#define MAP_WIDTH 20
#define MAP_HEIGHT 16
#define GRAVITY 600.0f
#define JUMP_FORCE -400.0f

typedef struct {
    Vector2 posicion;
    Vector2 velocidad;
    bool enSuelo;
    float tiempoCoyote;
    float contadorCoyote;
    float tiempoBufferSalto;
    float contadorBufferSalto;
    bool puedeDash;
    bool estaDashing;
    float tiempoDash;
    float contadorDash;
    bool puedeDobleSalto;
    bool enParedIzquierda;
    bool enParedDerecha;
} Cubo;

int mapa[MAP_HEIGHT][MAP_WIDTH];

void CargarMapa(const char *archivo) {
    FILE *file = fopen(archivo, "r");
    if (!file) {
        printf("Error al abrir el archivo del mapa.\n");
        exit(1);
    }

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            fscanf(file, "%d,", &mapa[y][x]);
        }
    }
    fclose(file);
}

bool VerificarColision(int x, int y) {
    if (x < 0 || y < 0 || x >= MAP_WIDTH || y >= MAP_HEIGHT) return false;
    return (mapa[y][x] == 1);
}

int main(void) {
    const int anchoPantalla = 800;
    const int altoPantalla = 600;

    InitWindow(anchoPantalla, altoPantalla, "Cubo con Camara - Raylib");
    SetTargetFPS(60);

    CargarMapa("mapa.csv");

    Cubo cubo = {0};
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            if (mapa[y][x] == 9) {
                cubo.posicion = (Vector2){ x * TILE_SIZE, y * TILE_SIZE };
                mapa[y][x] = 0;
            }
        }
    }

    cubo.velocidad = (Vector2){ 0, 0 };
    cubo.enSuelo = false;
    cubo.tiempoCoyote = 0.1f;
    cubo.contadorCoyote = 0.0f;
    cubo.tiempoBufferSalto = 0.1f;
    cubo.contadorBufferSalto = 0.0f;
    cubo.puedeDash = true;
    cubo.estaDashing = false;
    cubo.tiempoDash = 0.2f;
    cubo.contadorDash = 0.0f;
    cubo.puedeDobleSalto = true;
    cubo.enParedIzquierda = false;
    cubo.enParedDerecha = false;

    const float velocidadMovimiento = 200;
    const float aceleracion = 1000.0f;
    const float friccion = 800.0f;
    const float velocidadDash = 900.0f;
    const float tiempoSaltoPared = 0.2f;
    float contadorSaltoPared = 0.0f;

    // Configurar cámara 2D
    Camera2D camara = { 0 };
    camara.target = cubo.posicion;
    camara.offset = (Vector2){ anchoPantalla / 2.0f, altoPantalla / 2.0f };
    camara.rotation = 0.0f;
    camara.zoom = 1.0f;

    while (!WindowShouldClose()) {
        float delta = GetFrameTime();

        // Actualizar contador buffer salto
        if (IsKeyPressed(KEY_SPACE))
            cubo.contadorBufferSalto = cubo.tiempoBufferSalto;
        else
            cubo.contadorBufferSalto -= delta;

        // Dash
        if (IsKeyPressed(KEY_LEFT_SHIFT) && cubo.puedeDash) {
            cubo.estaDashing = true;
            cubo.puedeDash = false;
            cubo.contadorDash = cubo.tiempoDash;
            cubo.velocidad.y = 0;
            cubo.velocidad.x = (IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT)) * velocidadDash;
        }

        if (cubo.estaDashing) {
            cubo.contadorDash -= delta;
            if (cubo.contadorDash <= 0) 
            {
                cubo.estaDashing = false;
                cubo.velocidad.y += GRAVITY * delta; // Reanudar gravedad después del dash
            }
        } else {
            // Movimiento horizontal con aceleración y fricción
            if (IsKeyDown(KEY_RIGHT))
                cubo.velocidad.x += aceleracion * delta;
            else if (IsKeyDown(KEY_LEFT))
                cubo.velocidad.x -= aceleracion * delta;
            else {
                if (cubo.velocidad.x > 0) {
                    cubo.velocidad.x -= friccion * delta;
                    if (cubo.velocidad.x < 0) cubo.velocidad.x = 0;
                } else if (cubo.velocidad.x < 0) {
                    cubo.velocidad.x += friccion * delta;
                    if (cubo.velocidad.x > 0) cubo.velocidad.x = 0;
                }
            }

            // Limitar velocidad horizontal
            if (cubo.velocidad.x > velocidadMovimiento) cubo.velocidad.x = velocidadMovimiento;
            if (cubo.velocidad.x < -velocidadMovimiento) cubo.velocidad.x = -velocidadMovimiento;

            // Aplicar gravedad
            cubo.velocidad.y += GRAVITY * delta;
        }

        // Detectar colisiones con paredes para salto en pared
        int tileIzquierda = (cubo.posicion.x - 1) / TILE_SIZE;
        int tileDerecha = (cubo.posicion.x + TILE_SIZE + 1) / TILE_SIZE;
        int tileTop = (cubo.posicion.y) / TILE_SIZE;
        int tileBottom = (cubo.posicion.y + TILE_SIZE - 1) / TILE_SIZE;

        cubo.enParedIzquierda = (VerificarColision(tileIzquierda, tileTop) || VerificarColision(tileIzquierda, tileBottom));
        cubo.enParedDerecha = (VerificarColision(tileDerecha, tileTop) || VerificarColision(tileDerecha, tileBottom));

        // Movimiento y colisiones verticales y horizontales (como antes)
        Vector2 nuevaPos = cubo.posicion;
        nuevaPos.x += cubo.velocidad.x * delta;
        nuevaPos.y += cubo.velocidad.y * delta;

        int izquierda = (nuevaPos.x) / TILE_SIZE;
        int derecha = (nuevaPos.x + TILE_SIZE - 1) / TILE_SIZE;
        int arriba = (cubo.posicion.y) / TILE_SIZE;
        int abajo = (cubo.posicion.y + TILE_SIZE - 1) / TILE_SIZE;

        if (cubo.velocidad.x > 0 && (VerificarColision(derecha, arriba) || VerificarColision(derecha, abajo))) {
            nuevaPos.x = derecha * TILE_SIZE - TILE_SIZE;
            cubo.velocidad.x = 0;
        } else if (cubo.velocidad.x < 0 && (VerificarColision(izquierda, arriba) || VerificarColision(izquierda, abajo))) {
            nuevaPos.x = (izquierda + 1) * TILE_SIZE;
            cubo.velocidad.x = 0;
        }

        izquierda = (nuevaPos.x) / TILE_SIZE;
        derecha = (nuevaPos.x + TILE_SIZE - 1) / TILE_SIZE;
        arriba = (nuevaPos.y) / TILE_SIZE;
        abajo = (nuevaPos.y + TILE_SIZE - 1) / TILE_SIZE;

        cubo.enSuelo = false;

        if (cubo.velocidad.y > 0 && (VerificarColision(izquierda, abajo) || VerificarColision(derecha, abajo))) {
            nuevaPos.y = abajo * TILE_SIZE - TILE_SIZE;
            cubo.velocidad.y = 0;
            cubo.enSuelo = true;
            cubo.puedeDobleSalto = true;
        } else if (cubo.velocidad.y < 0 && (VerificarColision(izquierda, arriba) || VerificarColision(derecha, arriba))) {
            nuevaPos.y = (arriba + 1) * TILE_SIZE;
            cubo.velocidad.y = 0;
        }

        // Salto y doble salto
        if (cubo.contadorBufferSalto > 0.0f && (cubo.contadorCoyote > 0.0f || cubo.puedeDobleSalto || cubo.enParedIzquierda || cubo.enParedDerecha)) {
            if (cubo.enParedIzquierda || cubo.enParedDerecha) {
                cubo.velocidad.y = JUMP_FORCE;
                cubo.velocidad.x = (cubo.enParedIzquierda ? velocidadMovimiento : -velocidadMovimiento);
                contadorSaltoPared = tiempoSaltoPared;
            } else {
                cubo.velocidad.y = JUMP_FORCE;
                if (!cubo.enSuelo && cubo.puedeDobleSalto) {
                    cubo.puedeDobleSalto = false;
                }
            }
            cubo.enSuelo = false;
            cubo.contadorCoyote = 0.0f;
            cubo.contadorBufferSalto = 0.0f;
        }

        if (contadorSaltoPared > 0) {
            contadorSaltoPared -= delta;
        }

        if (cubo.enSuelo) {
            cubo.contadorCoyote = cubo.tiempoCoyote;
            cubo.puedeDash = true;
        } else {
            cubo.contadorCoyote -= delta;
        }

        if (IsKeyReleased(KEY_SPACE) && cubo.velocidad.y < 0) {
            cubo.velocidad.y *= 0.5f;
        }

        cubo.posicion = nuevaPos;

        // Actualizar la posición de la cámara para que siga al cubo
        camara.target = cubo.posicion;

        BeginDrawing();
            ClearBackground(RAYWHITE);

            BeginMode2D(camara);

            for (int y = 0; y < MAP_HEIGHT; y++) {
                for (int x = 0; x < MAP_WIDTH; x++) {
                    if (mapa[y][x] == 1) {
                        DrawRectangle(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, DARKGRAY);
                    }
                }
            }

            DrawRectangleV(cubo.posicion, (Vector2){TILE_SIZE, TILE_SIZE}, RED);

            EndMode2D();

            DrawText("Flechas para mover, SPACE para saltar, SHIFT para dash", 10, 10, 20, DARKGRAY);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
