#include "raylib.h"
#include "tdas/list.h" // Asegúrate de que esta ruta sea correcta si usas list.h
#include "tdas/grafo.h"     // Usamos tu nuevo grafo.h
#include "tdas/extra.h" // Asegúrate de que esta ruta sea correcta si usas extra.h
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h> // Para floorf

// --- CONSTANTES DEL JUEGO ---
#define TILE_SIZE 64.0f // Cambiamos a float para cálculos de posición del jugador
#define GRAVITY 600.0f
#define JUMP_FORCE -400.0f
#define VELOCIDAD_MOVIMIENTO 200.0f // Velocidad base del jugador
#define ACELERACION 1000.0f
#define FRICCION 800.0f
#define VELOCIDAD_DASH 900.0f
#define TIEMPO_COYOTE 0.1f
#define TIEMPO_BUFFER_SALTO 0.1f
#define TIEMPO_SALTO_PARED 0.2f
#define TIEMPO_DASH 0.2f
#define VELOCIDAD_MAX_CAIDA 800.0f // Ajustada a un valor más razonable en píxeles/segundo

// --- ENUMERACIONES Y ESTRUCTURAS ---
typedef enum GameScreen
{
    MENU = 0,
    GAMEPLAY,
    PONER_NOMBRE
} GameScreen;

typedef struct personaje
{
    char nombre[9];
    int vida;
    int ataque;
    int defensa;
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
    float contadorSaltoPared;
} Personaje;

// --- VARIABLES GLOBALES ---
const int base_ancho = 1280;
const int base_alto = 1024; // Alto base para el mapa de 16 tiles (16 * 64)

NodoMapa* mapaActual = NULL;
int** mapa; // Matriz del mapa cargado dinámicamente

GameScreen pantallaDeJuego = MENU;
int opcionSelecionada = 0;
Personaje jugador = {.nombre = "", .vida = 100, .ataque = 10, .defensa = 5,
                      .posicion = {0, 0}, .velocidad = {0, 0}, .enSuelo = false,
                      .tiempoCoyote = TIEMPO_COYOTE, .contadorCoyote = 0.0f,
                      .tiempoBufferSalto = TIEMPO_BUFFER_SALTO, .contadorBufferSalto = 0.0f,
                      .puedeDash = true, .estaDashing = false, .tiempoDash = TIEMPO_DASH, .contadorDash = 0.0f,
                      .puedeDobleSalto = true, .enParedIzquierda = false, .enParedDerecha = false,
                      .contadorSaltoPared = 0.0f};

Camera2D camara = {0};

// --- FUNCIONES AUXILIARES ---

void CargarMapa(const char* nombreArchivo, int*** mapaPtr, int* ancho, int* alto)
{
    FILE* archivo = fopen(nombreArchivo, "r");
    if (!archivo) {
        printf("Error: No se pudo abrir el archivo del mapa: %s\n", nombreArchivo);
        exit(1);
    }

    // Liberar mapa anterior si existe
    if (*mapaPtr != NULL) {
        for (int y = 0; y < *alto; y++) {
            free((*mapaPtr)[y]);
        }
        free(*mapaPtr);
    }

    // Leer dimensiones del mapa desde el archivo
    fscanf(archivo, "%d\n", ancho);
    fscanf(archivo, "%d\n", alto);

    // Asignar dimensiones al NodoMapa actual (importante para ActualizarGameplay)
    if (mapaActual != NULL) {
        mapaActual->ancho = *ancho;
        mapaActual->alto = *alto;
    }

    *mapaPtr = (int**)malloc((*alto) * sizeof(int*));
    for (int y = 0; y < *alto; y++)
    {
        (*mapaPtr)[y] = (int*)malloc((*ancho) * sizeof(int));
    }

    for (int y = 0; y < *alto; y++)
    {
        for (int x = 0; x < *ancho; x++)
        {
            if (x < (*ancho - 1))
            {
                fscanf(archivo, "%d,", &(*mapaPtr)[y][x]);
            }
            else
            {
                fscanf(archivo, "%d\n", &(*mapaPtr)[y][x]);
            }

            if ((*mapaPtr)[y][x] == 9)
            {
                jugador.posicion = (Vector2){ (float)x * TILE_SIZE, (float)y * TILE_SIZE };
                (*mapaPtr)[y][x] = 0; // Set spawn point to empty tile
            }
        }
    }
    fclose(archivo);
}

void DibujarMapa(int** mapa, int ancho, int alto) // Eliminamos scaleX/Y de aquí, la cámara lo maneja
{
    for (int y = 0; y < alto; y++) {
        for (int x = 0; x < ancho; x++) {
            Rectangle tile = { x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE };

            switch (mapa[y][x]) {
                case 0: DrawRectangleRec(tile, BLACK); break;
                case 1: DrawRectangleRec(tile, GRAY); break;
                case 2: DrawRectangleRec(tile, RED); break;
                case 3: DrawRectangleRec(tile, BLUE); break;
                case 4: DrawRectangleRec(tile, DARKGRAY); break;
                case 5: DrawRectangleRec(tile, ORANGE); break;
                case 6: DrawRectangleRec(tile, GREEN); break;
                case 7: DrawRectangleRec(tile, PURPLE); break;
                case 8: DrawRectangleRec(tile, GOLD); break;
                default: DrawRectangleRec(tile, BLACK); break;
            }
        }
    }
}

// VerificarColision (adaptada para usar el mapa dinámico y TILE_SIZE flotante)
bool VerificarColision(int tileX, int tileY, int anchoMapa, int altoMapa) {
    if (tileX < 0 || tileY < 0 || tileX >= anchoMapa || tileY >= altoMapa) return false;
    return (mapa[tileY][tileX] == 1); // Solo colisiona con el tipo 1 (plataforma)
}

// --- FUNCIONES DE PANTALLA ---
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
            if (len < 8) {
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

// --- LÓGICA DE JUEGO (ADAPTADA) ---
void ActualizarGameplay() // No necesita ancho/alto como parámetros, ya están en mapaActual
{
    float delta = GetFrameTime();

    // Actualizar contador buffer salto
    if (IsKeyPressed(KEY_SPACE)) {
        jugador.contadorBufferSalto = jugador.tiempoBufferSalto;
    } else {
        jugador.contadorBufferSalto -= delta;
    }

    // Dash
    if (IsKeyPressed(KEY_LEFT_SHIFT) && jugador.puedeDash) {
        jugador.estaDashing = true;
        jugador.puedeDash = false;
        jugador.contadorDash = jugador.tiempoDash;
        jugador.velocidad.y = 0;
        jugador.velocidad.x = (IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT)) * VELOCIDAD_DASH;
    }

    if (jugador.estaDashing) {
        jugador.contadorDash -= delta;
        if (jugador.contadorDash <= 0)
        {
            jugador.estaDashing = false;
            // No reanudamos gravedad aquí; la aplicamos siempre en el else branch
        }
    } else {
        // Movimiento horizontal con aceleración y fricción
        if (IsKeyDown(KEY_RIGHT)) {
            jugador.velocidad.x += ACELERACION * delta;
        } else if (IsKeyDown(KEY_LEFT)) {
            jugador.velocidad.x -= ACELERACION * delta;
        } else {
            if (jugador.velocidad.x > 0) {
                jugador.velocidad.x -= FRICCION * delta;
                if (jugador.velocidad.x < 0) jugador.velocidad.x = 0;
            } else if (jugador.velocidad.x < 0) {
                jugador.velocidad.x += FRICCION * delta;
                if (jugador.velocidad.x > 0) jugador.velocidad.x = 0;
            }
        }

        // Limitar velocidad horizontal
        if (jugador.velocidad.x > VELOCIDAD_MOVIMIENTO) jugador.velocidad.x = VELOCIDAD_MOVIMIENTO;
        if (jugador.velocidad.x < -VELOCIDAD_MOVIMIENTO) jugador.velocidad.x = -VELOCIDAD_MOVIMIENTO;

        // Aplicar gravedad
        jugador.velocidad.y += GRAVITY * delta;
    }

    // Limitar velocidad de caída (máxima)
    if (jugador.velocidad.y > VELOCIDAD_MAX_CAIDA) {
        jugador.velocidad.y = VELOCIDAD_MAX_CAIDA;
    }

    // Movimiento y colisiones verticales y horizontales (adaptado de pruebas.c)
    Vector2 nuevaPos = jugador.posicion;
    nuevaPos.x += jugador.velocidad.x * delta;
    nuevaPos.y += jugador.velocidad.y * delta;

    // --- Colisión Horizontal ---
    // Calcular tiles del jugador para la nueva posición horizontal
    int col_izq_px = (int)nuevaPos.x;
    int col_der_px = (int)(nuevaPos.x + TILE_SIZE - 1); // -1 para no sobrepasar el borde del tile
    int col_arriba_y_px = (int)jugador.posicion.y;
    int col_abajo_y_px = (int)(jugador.posicion.y + TILE_SIZE - 1);

    int col_izq_tile = (int)(col_izq_px / TILE_SIZE);
    int col_der_tile = (int)(col_der_px / TILE_SIZE);
    int col_arriba_y_tile = (int)(col_arriba_y_px / TILE_SIZE);
    int col_abajo_y_tile = (int)(col_abajo_y_px / TILE_SIZE);


    if (jugador.velocidad.x > 0) { // Moviéndose a la derecha
        // Check both top-right and bottom-right corners
        if (VerificarColision(col_der_tile, col_arriba_y_tile, mapaActual->ancho, mapaActual->alto) ||
            VerificarColision(col_der_tile, col_abajo_y_tile, mapaActual->ancho, mapaActual->alto)) {
            nuevaPos.x = col_der_tile * TILE_SIZE - TILE_SIZE; // Coloca al jugador justo al lado del bloque
            jugador.velocidad.x = 0;
        }
    } else if (jugador.velocidad.x < 0) { // Moviéndose a la izquierda
        // Check both top-left and bottom-left corners
        if (VerificarColision(col_izq_tile, col_arriba_y_tile, mapaActual->ancho, mapaActual->alto) ||
            VerificarColision(col_izq_tile, col_abajo_y_tile, mapaActual->ancho, mapaActual->alto)) {
            nuevaPos.x = (col_izq_tile + 1) * TILE_SIZE; // Coloca al jugador justo al lado del bloque
            jugador.velocidad.x = 0;
        }
    }

    // --- Colisión Vertical ---
    // Calcular tiles del jugador para la nueva posición vertical
    int col_arriba_px = (int)nuevaPos.y;
    int col_abajo_px = (int)(nuevaPos.y + TILE_SIZE - 1);
    int col_izq_x_px = (int)nuevaPos.x;
    int col_der_x_px = (int)(nuevaPos.x + TILE_SIZE - 1);

    int col_arriba_tile = (int)(col_arriba_px / TILE_SIZE);
    int col_abajo_tile = (int)(col_abajo_px / TILE_SIZE);
    int col_izq_x_tile = (int)(col_izq_x_px / TILE_SIZE);
    int col_der_x_tile = (int)(col_der_x_px / TILE_SIZE);

    jugador.enSuelo = false; // Resetear en cada frame

    if (jugador.velocidad.y >= 0) { // Cayendo o en reposo vertical (gravedad)
        // Check both bottom-left and bottom-right corners
        if (VerificarColision(col_izq_x_tile, col_abajo_tile, mapaActual->ancho, mapaActual->alto) ||
            VerificarColision(col_der_x_tile, col_abajo_tile, mapaActual->ancho, mapaActual->alto)) {
            nuevaPos.y = col_abajo_tile * TILE_SIZE - TILE_SIZE; // Se detiene encima del bloque
            jugador.velocidad.y = 0;
            jugador.enSuelo = true;
            jugador.puedeDobleSalto = true; // Reiniciar doble salto al tocar suelo
            jugador.puedeDash = true; // Reiniciar dash al tocar suelo
        }
    } else { // Subiendo (salto)
        // Check both top-left and top-right corners
        if (VerificarColision(col_izq_x_tile, col_arriba_tile, mapaActual->ancho, mapaActual->alto) ||
            VerificarColision(col_der_x_tile, col_arriba_tile, mapaActual->ancho, mapaActual->alto)) {
            nuevaPos.y = (col_arriba_tile + 1) * TILE_SIZE; // Se detiene debajo del bloque (hit ceiling)
            jugador.velocidad.y = 0;
        }
    }

    // Detectar colisiones con paredes para salto en pared
    // Usamos floorf() para ser más robustos con floats.
    // Revisar el tile *al lado* del jugador, no el que ocupa.
    jugador.enParedIzquierda = VerificarColision((int)floorf((jugador.posicion.x - 1) / TILE_SIZE), (int)floorf(jugador.posicion.y / TILE_SIZE), mapaActual->ancho, mapaActual->alto) ||
                               VerificarColision((int)floorf((jugador.posicion.x - 1) / TILE_SIZE), (int)floorf((jugador.posicion.y + TILE_SIZE - 1) / TILE_SIZE), mapaActual->ancho, mapaActual->alto);
    jugador.enParedDerecha = VerificarColision((int)floorf((jugador.posicion.x + TILE_SIZE) / TILE_SIZE), (int)floorf(jugador.posicion.y / TILE_SIZE), mapaActual->ancho, mapaActual->alto) ||
                             VerificarColision((int)floorf((jugador.posicion.x + TILE_SIZE) / TILE_SIZE), (int)floorf((jugador.posicion.y + TILE_SIZE - 1) / TILE_SIZE), mapaActual->ancho, mapaActual->alto);


    // Salto, doble salto y salto de pared
    if (jugador.contadorBufferSalto > 0.0f &&
       (jugador.contadorCoyote > 0.0f || jugador.puedeDobleSalto || jugador.enParedIzquierda || jugador.enParedDerecha)) {

        if (jugador.enParedIzquierda || jugador.enParedDerecha) {
            jugador.velocidad.y = JUMP_FORCE;
            // Impulso horizontal opuesto a la pared
            jugador.velocidad.x = (jugador.enParedIzquierda ? VELOCIDAD_MOVIMIENTO : -VELOCIDAD_MOVIMIENTO);
            jugador.contadorSaltoPared = TIEMPO_SALTO_PARED;
        } else {
            jugador.velocidad.y = JUMP_FORCE;
            if (!jugador.enSuelo && jugador.puedeDobleSalto) {
                jugador.puedeDobleSalto = false;
            }
        }
        jugador.enSuelo = false;
        jugador.contadorCoyote = 0.0f;
        jugador.contadorBufferSalto = 0.0f;
    }

    if (jugador.contadorSaltoPared > 0) {
        jugador.contadorSaltoPared -= delta;
        // Durante el salto de pared, el movimiento horizontal del usuario se anula
        if (jugador.contadorSaltoPared > 0) {
            if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT)) {
                // Anular input horizontal mientras dure el "wall jump lock"
                if (jugador.velocidad.x > 0 && IsKeyDown(KEY_LEFT)) jugador.velocidad.x = 0;
                if (jugador.velocidad.x < 0 && IsKeyDown(KEY_RIGHT)) jugador.velocidad.x = 0;
            }
        }
    }


    if (jugador.enSuelo) {
        jugador.contadorCoyote = jugador.tiempoCoyote;
        // La variable `puedeDash` ya se reinicia en la colisión vertical si `enSuelo` es true.
    } else {
        jugador.contadorCoyote -= delta;
    }

    // Salto de altura variable (mantener SPACE para saltar más alto)
    if (IsKeyReleased(KEY_SPACE) && jugador.velocidad.y < 0) {
        jugador.velocidad.y *= 0.5f;
    }

    jugador.posicion = nuevaPos;

    // Actualizar la posición de la cámara para que siga al jugador
    camara.target = jugador.posicion;

    // --- Lógica de cambio de mapa ---
    // Si el jugador llega al borde derecho del mapa
    if (jugador.posicion.x >= (mapaActual->ancho - 1) * TILE_SIZE) {
        if (mapaActual->este != NULL) { // Usamos el puntero 'este'
            // Liberar mapa actual
            for (int y = 0; y < mapaActual->alto; y++) {
                free(mapa[y]);
            }
            free(mapa);
            mapa = NULL;

            mapaActual = mapaActual->este; // Cambiar al siguiente mapa
            CargarMapa(mapaActual->archivoMapa, &mapa, &mapaActual->ancho, &mapaActual->alto);
            jugador.posicion.x = 1 * TILE_SIZE; // Posicionar en el inicio del nuevo mapa (ej. izquierda)
            // Mantener la posición Y o ajustarla si las alturas de los mapas varían mucho
            // jugador.posicion.y = (mapaActual->alto - 2) * TILE_SIZE; // Ejemplo: en el suelo del nuevo mapa
        }
    }
    // Si el jugador llega al borde izquierdo del mapa
    else if (jugador.posicion.x <= 0) {
        if (mapaActual->oeste != NULL) { // Usamos el puntero 'oeste'
             // Liberar mapa actual
            for (int y = 0; y < mapaActual->alto; y++) {
                free(mapa[y]);
            }
            free(mapa);
            mapa = NULL;

            mapaActual = mapaActual->oeste; // Cambiar al mapa anterior
            CargarMapa(mapaActual->archivoMapa, &mapa, &mapaActual->ancho, &mapaActual->alto);
            jugador.posicion.x = (mapaActual->ancho - 2) * TILE_SIZE; // Posicionar en el final del nuevo mapa (ej. derecha)
            // Mantener la posición Y o ajustarla
            // jugador.posicion.y = (mapaActual->alto - 2) * TILE_SIZE; // Ejemplo: en el suelo del nuevo mapa
        }
    }
    // Puedes añadir lógica similar para norte y sur si tienes mapas conectados verticalmente
    // else if (jugador.posicion.y <= 0) { ... mapaActual->norte ... }
    // else if (jugador.posicion.y >= (mapaActual->alto - 1) * TILE_SIZE) { ... mapaActual->sur ... }

}

// --- FUNCIONES DE DIBUJO ---
void DrawMenu(Texture2D menuTexture, float scaleX, float scaleY) {
    DrawTextureEx(menuTexture, (Vector2){0, 0}, 0.0f, scaleX, WHITE);
    DrawText("MENU PRINCIPAL", (int)(scaleX * 480), (int)(scaleY * 200), (int)(scaleY * 40), WHITE);
    DrawText("Iniciar Juego", (int)(scaleX * 540), (int)(scaleY * 300), (int)(scaleY * 30), opcionSelecionada == 0 ? RED : WHITE);
    DrawText("Salir", (int)(scaleX * 540), (int)(scaleY * 350), (int)(scaleY * 30), opcionSelecionada == 1 ? RED : WHITE);
    DrawText("Usa Flechas ARRIBA/ABAJO y ENTER para elegir", (int)(scaleX * 360), (int)(scaleY * 500), (int)(scaleY * 20), WHITE);
}

void DrawNameInput(float scaleX, float scaleY) {
    DrawText("INGRESA TU NOMBRE:", (int)(scaleX * 400), (int)(scaleY * 200), (int)(scaleY * 30), WHITE);
    DrawText(jugador.nombre, (int)(scaleX * 400), (int)(scaleY * 250), (int)(scaleY * 30), PINK);
    DrawText("Presiona ENTER para continuar", (int)(scaleX * 400), (int)(scaleY * 300), (int)(scaleY * 20), WHITE);
}

void DrawGameplay(float scaleX, float scaleY)
{
    // Dibuja el mapa (usando la cámara)
    BeginMode2D(camara);
        DibujarMapa(mapa, mapaActual->ancho, mapaActual->alto);

        // Dibuja al jugador
        Rectangle playerRect = {
            jugador.posicion.x,
            jugador.posicion.y,
            TILE_SIZE,
            TILE_SIZE
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
                 (int)(playerRect.x + playerRect.width / 2 - MeasureText(jugador.nombre, 20) / 2),
                 (int)(playerRect.y - 25),
                 20, WHITE);

    EndMode2D();

    // UI elements not affected by camera (e.g., instructions, stats)
    DrawText(TextFormat("Vida: %d", jugador.vida), (int)(10 * scaleX), (int)(10 * scaleY), (int)(20 * scaleY), WHITE);
    DrawText(TextFormat("Pos: %.1f, %.1f", jugador.posicion.x/TILE_SIZE, jugador.posicion.y/TILE_SIZE), (int)(10 * scaleX), (int)(40 * scaleY), (int)(20 * scaleY), WHITE);
    DrawText("Flechas para mover, SPACE para saltar, SHIFT para dash", (int)(10 * scaleX), (int)(GetScreenHeight() - 30 * scaleY), (int)(20 * scaleY), DARKGRAY);
}


// --- MAIN FUNCTION ---
int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(base_ancho, base_alto, "El inicio de una aventura");
    SetTargetFPS(60);

    // Cargar imagen de menú
    Image image = LoadImage("base/Menu_incial.png"); // Asegúrate de que la ruta sea correcta
    Texture2D Menu_inicial_imagen = LoadTextureFromImage(image);
    UnloadImage(image);

    // Inicializar la cámara
    camara.target = jugador.posicion;
    camara.offset = (Vector2){ base_ancho / 2.0f, base_alto / 2.0f };
    camara.rotation = 0.0f;
    camara.zoom = 1.0f;

    // Crear el grafo de mapas
    // Asegúrate de que estos archivos CSV existan y contengan las dimensiones correctas.
    NodoMapa* mapa1 = CrearMapa(1, "mapas/mapa1.csv"); // Usar el mapa de 20x16 para empezar
    NodoMapa* mapa2 = CrearMapa(2, "mapas/mapa2.csv"); // Asegúrate de crear este archivo
    NodoMapa* mapa3 = CrearMapa(3, "mapas/mapa3.csv"); // Asegúrate de crear este archivo

    // Conectar mapas usando los nuevos punteros directos
    ConectarMapas(mapa1, mapa2, 2); // mapa1 Este -> mapa2
    ConectarMapas(mapa2, mapa1, 3); // mapa2 Oeste -> mapa1
    ConectarMapas(mapa2, mapa3, 2); // mapa2 Este -> mapa3
    ConectarMapas(mapa3, mapa2, 3);

    mapaActual = mapa1; // Comenzamos en el primer mapa
    CargarMapa(mapaActual->archivoMapa, &mapa, &mapaActual->ancho, &mapaActual->alto);

    // Lista para llevar un registro de todos los nodos del grafo para su posterior liberación
    // Esto es necesario porque LiberarGrafo espera un array.
    // Podrías usar una List* de tu tdas/list.h si lo tienes implementado para nodos de grafo.
    // Por simplicidad, aquí los agregamos manualmente, asumiendo un número fijo de mapas.
    NodoMapa* todosLosMapas[] = {mapa1, mapa2, mapa3};
    int cantidadMapas = 3;


    while (!WindowShouldClose()) {
        int pantalla_ancho = GetScreenWidth();
        int pantalla_alto = GetScreenHeight();
        float scaleX = (float)pantalla_ancho / base_ancho;
        float scaleY = (float)pantalla_alto / base_alto;

        // Actualizar offset de la cámara para que siempre esté centrada en la ventana actual
        camara.offset = (Vector2){ pantalla_ancho / 2.0f, pantalla_alto / 2.0f };

        // Actualización
        switch (pantallaDeJuego) {
            case MENU:
                ActualizarMenu();
                break;
            case PONER_NOMBRE:
                ActualizarPonerNombre();
                break;
            case GAMEPLAY:
                ActualizarGameplay(); // Ya no necesita pasar mapa, ancho, alto
                break;
        }

        // Dibujo
        BeginDrawing();
        ClearBackground((Color){ 50, 50, 50, 255 });

        switch (pantallaDeJuego) {
            case MENU: DrawMenu(Menu_inicial_imagen, scaleX, scaleY); break;
            case PONER_NOMBRE: DrawNameInput(scaleX, scaleY); break;
            case GAMEPLAY: DrawGameplay(scaleX, scaleY); break; // Ya no necesita pasar mapa, ancho, alto
        }

        EndDrawing();
    }

    // Liberar recursos
    UnloadTexture(Menu_inicial_imagen);
    // Liberar memoria del mapa dinámico si aún está cargado
    if (mapa != NULL) {
        for (int y = 0; y < mapaActual->alto; y++) {
            free(mapa[y]);
        }
        free(mapa);
        mapa = NULL;
    }
    // Liberar nodos del grafo usando tu función LiberarGrafo y strdup para archivoMapa
    for (int i = 0; i < cantidadMapas; i++) {
        free(todosLosMapas[i]->archivoMapa); // Liberar la memoria asignada por strdup
    }
    LiberarGrafo(todosLosMapas, cantidadMapas); // Ahora LiberarGrafo solo libera los nodos

    CloseWindow();
    return 0;
}