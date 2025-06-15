#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum GameScreen {
    MENU = 0,GAMEPLAY
} GameScreen;

int main()
{
    const int baseWidth = 1280;
    const int baseHeight = 900;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);


    InitWindow(baseWidth,baseHeight,"El inicio de una aventura");
    SetTargetFPS(60);
    
    Image image = LoadImage("base/Menu_inicio.png");
    ImageResize(&image, baseWidth, baseHeight);
    Texture2D menuBackground = LoadTextureFromImage(image);
    UnloadImage(image);


    GameScreen screen = MENU;
    int opcionSelecionada = 0;

    while (!WindowShouldClose())
    {
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();

        // Calcula escalas en tiempo real
        float scaleX = (float)screenWidth / baseWidth;
        float scaleY = (float)screenHeight / baseHeight;
        if (screen == MENU)
        {
            if (IsKeyPressed(KEY_DOWN)) opcionSelecionada++;
            if (IsKeyPressed(KEY_UP)) opcionSelecionada--;
            
            if (opcionSelecionada > 1) opcionSelecionada = 0;
            if (opcionSelecionada < 0) opcionSelecionada = 1;
            
            if (IsKeyPressed(KEY_ENTER))
            {
                if (opcionSelecionada == 0) screen = GAMEPLAY;
                else if (opcionSelecionada == 1) CloseWindow();

            }
        }
        else if (screen == GAMEPLAY)
        {
            if (IsKeyPressed(KEY_P)) screen = MENU;
            
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        if (screen == MENU)
        {
            DrawTextureEx(menuBackground, (Vector2){0, 0}, 0.0f, scaleX, WHITE);
            DrawText("MENU PRINCIPAL", scaleX * 480, scaleY * 200, scaleY * 40, BLACK);
            DrawText("Iniciar Juego", scaleX * 540, scaleY * 300, scaleY * 30, opcionSelecionada == 0 ? RED : BLACK);
            DrawText("Salir",scaleX * 540, scaleY * 350, scaleY * 30, opcionSelecionada == 1 ? RED : BLACK);

            DrawText("Usa Flechas ARRIBA/ABAJO y ENTER para elegir", scaleX * 360, scaleY * 500, scaleY * 20, GRAY);
        }
        else if (screen == GAMEPLAY)
        {
            DrawText("JUEGO INICIADO!",scaleX * 500, scaleY *400, scaleY * 40, DARKGREEN);
            DrawText("Presiona P para volver al menu", scaleX * 360, scaleY *500, scaleY * 20, GRAY);
        }

        EndDrawing();

    }

    CloseWindow();
    return 0;
}