#include "raylib.h"
#include "tdas/list.h"
#include "tdas/grafo.h"
#include "tdas/extra.h"
#include "tdas/movimiento.h"  // Incluimos la biblioteca de movimiento
#include "tdas/hashmap.h"
#include "tdas/sprite.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

// --- CONSTANTES DEL JUEGO ---
#define TILE_SIZE 64.0f
#define BASE_ANCHO 1280
#define BASE_ALTO 1024
#define MENSAJE_DURACION 2.5f

typedef struct {
    char nombre[20];  // "Pocion", "Fruta", etc.
    int cantidad;     // cuántos tienes
    int curacion;     // cuánto cura ese item
} Item;

// --- ENUMERACIONES Y ESTRUCTURAS ---
typedef enum GameScreen
{
    MENU = 0,
    GAMEPLAY,
    PONER_NOMBRE,
    GAME_OVER,
    COMBATE,
    INVENTARIO,
} GameScreen;

// --- VARIABLES GLOBALES ---
Sprite* spriteJugador = NULL;
NodoMapa* mapaActual = NULL;
int** mapa; // Matriz del mapa cargado dinámicamente
NodoMapa* mapaInicial = NULL;

char mensajePantalla[256] = "";
float tiempoMensaje = 0.0f;


GameScreen pantallaDeJuego = MENU;
int opcionSelecionada = 0;
int inventarioSeleccionado = 0;

Personaje jugador;  // Ahora se inicializará con InicializarPersonaje()

Camera2D camara = {0};

List* spritesActivos = NULL; // Lista de sprites activos

Texture2D texturasMapa[4]; // 0: pared, 1: suelo, 2: pared_izq, 3: pared_der

// --- FUNCION AUXILIAR PARA MENSAJES EN PANTALLA ---
void MostrarMensaje(const char* mensaje) {
    strncpy(mensajePantalla, mensaje, sizeof(mensajePantalla) - 1);
    mensajePantalla[sizeof(mensajePantalla) - 1] = '\0';
    tiempoMensaje = MENSAJE_DURACION;
}

//FUNCIONES INVENTARIO
void AgregarItem(HashMap* inventario, const char* nombre, int curacion) {
    Pair* par = searchMap(inventario, (char*)nombre);
    if (par != NULL) {
        Item* item = (Item*)par->value;
        item->cantidad++;
        char buf[100];
        snprintf(buf, sizeof(buf), "Has recibido otro %s (x%d)", nombre, item->cantidad);
        MostrarMensaje(buf);
    } else {
        Item* nuevo = malloc(sizeof(Item));
        nuevo->curacion = curacion;
        nuevo->cantidad = 1;
        strcpy(nuevo->nombre, nombre);

        insertMap(inventario, strdup(nombre), nuevo);
        char buf[100];
        snprintf(buf, sizeof(buf), "Has recibido: %s", nombre);
        MostrarMensaje(buf);
    }
}

void UsarItem(HashMap* inventario, const char* nombre, Personaje* jugador) {
    Pair* par = searchMap(inventario, (char*)nombre);
    if (par != NULL) {
        Item* item = (Item*)par->value;

        jugador->vida += item->curacion;
        if (jugador->vida > 100) jugador->vida = 100;

        char buf[100];
        snprintf(buf, sizeof(buf), "Usaste %s, curaste %d. Vida: %d", nombre, item->curacion, jugador->vida);
        MostrarMensaje(buf);

        item->cantidad--;
        if (item->cantidad <= 0) {
            free(item);
            eraseMap(inventario, (char*)nombre);
        }
    } else {
        MostrarMensaje("No tienes ese item");
    }
}

int ContarItems(HashMap* inventario) {
    int count = 0;
    Pair* par = firstMap(inventario);
    while (par != NULL) {
        count++;
        par = nextMap(inventario);
    }
    return count;
}

Pair* ObtenerItemEnPosicion(HashMap* inventario, int pos) {
    Pair* par = firstMap(inventario);
    int i = 0;
    while (par != NULL) {
        if (i == pos) return par;
        par = nextMap(inventario);
        i++;
    }
    return NULL;
}

void LimpiarInventario(HashMap* inventario) {
    if (!inventario) return;

    Pair* par = firstMap(inventario);
    while (par) {
        free(par->value);   // Liberar el item apuntado
        par = nextMap(inventario);
    }
    // No tienes función clearMap, así que borras todos keys:
    // La forma es iterar mientras haya elementos y borrar uno por uno:

    par = firstMap(inventario);
    while (par != NULL) {
        eraseMap(inventario, par->key);
        par = firstMap(inventario);
    }
}

// --- FUNCIONES AUXILIARES ---

void CargarMapa(const char* nombreArchivo, int*** mapaPtr, int* ancho, int* alto, Personaje* jugador)
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

    // Asignar dimensiones al NodoMapa actual
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
                (*jugador).posicion = (Vector2){ (float) x * TILE_SIZE, (float) y * TILE_SIZE };
                (*jugador).spawn = (*jugador).posicion; // Guardar posición inicial (spawn)
                (*mapaPtr)[y][x] = 0; // Set spawn point to empty tile
            }
        }
    }

    fclose(archivo);
}

void DibujarMapa(int** mapa, int ancho, int alto)
{
    for (int y = 0; y < alto; y++) {
        for (int x = 0; x < ancho; x++) {
            int tipo = mapa[y][x];
            Rectangle destino = { x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE };

            switch (tipo) {
                case 0:  // Pared
                case 1:  // Suelo
                case -1: // Pared izquierda
                case -2: // Pared derecha
                {
                    int index;
                    if (tipo == 0) index = 0;
                    else if (tipo == 1) index = 1;
                    else if (tipo == -1) index = 2;
                    else index = 3;

                    Texture2D textura = texturasMapa[index];
                    Rectangle origen = { 0, 0, (float)textura.width, (float)textura.height };
                    DrawTexturePro(textura, origen, destino, (Vector2){0, 0}, 0.0f, WHITE);
                    break;
                }
                case 2: DrawRectangleRec(destino, RED); break;
                case 3: DrawRectangleRec(destino, BLUE); break;
                case 4: DrawRectangleRec(destino, DARKGRAY); break;
                case 5: DrawRectangleRec(destino, ORANGE); break;
                case 6: DrawRectangleRec(destino, GREEN); break;
                case 7: DrawRectangleRec(destino, PURPLE); break;
                case 8: DrawRectangleRec(destino, GOLD); break;
                default: DrawRectangleRec(destino, BLACK); break;
            }
        }
    }
}

// Función de verificación de colisiones
bool VerificarColision(int tileX, int tileY, int anchoMapa, int altoMapa) {
    if (tileX < 0 || tileY < 0 || tileX >= anchoMapa || tileY >= altoMapa) 
        return false;
    
    int tipo = mapa[tileY][tileX];
    // Solo colisiona con el tipo 1, -1 y -2 (plataforma y paredes)
    return (tipo == 1 || tipo == -1 || tipo == -2);
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

void ActualizarInventario() {
    int totalItems = ContarItems(jugador.inventario);

    if (IsKeyPressed(KEY_DOWN)) {
        inventarioSeleccionado++;
        if (inventarioSeleccionado >= totalItems) inventarioSeleccionado = 0;
    }
    if (IsKeyPressed(KEY_UP)) {
        inventarioSeleccionado--;
        if (inventarioSeleccionado < 0) inventarioSeleccionado = totalItems - 1;
    }

    if (IsKeyPressed(KEY_ENTER)) {
        // Usar el item seleccionado
        Pair* par = ObtenerItemEnPosicion(jugador.inventario, inventarioSeleccionado);
        if (par != NULL) {
            UsarItem(jugador.inventario, par->key, &jugador);
            // Si el item se acabó, ajusta el índice para que no se salga
            int nuevosItems = ContarItems(jugador.inventario);
            if (inventarioSeleccionado >= nuevosItems) inventarioSeleccionado = nuevosItems - 1;
        }
    }

    if (IsKeyPressed(KEY_I)) {
        pantallaDeJuego = GAMEPLAY; // Salir del inventario
    }
}

// --- LÓGICA DE JUEGO ---
void ActualizarGameplay()
{
    int tileX = (int)(jugador.posicion.x / TILE_SIZE);
    int tileY = (int)(jugador.posicion.y / TILE_SIZE);

    float delta = GetFrameTime();
    if (tiempoMensaje > 0.0f) 
    {   
    tiempoMensaje -= GetFrameTime();
   if (tiempoMensaje < 0.0f) 
        tiempoMensaje = 0.0f;
    }

    // Usamos la función de la biblioteca para manejar todo el movimiento
    ActualizarMovimiento(&jugador, mapaActual->ancho, mapaActual->alto, delta, VerificarColision);

    if (jugador.vida <= 0) {
    pantallaDeJuego = GAME_OVER;
    }
    // Actualizar la posición de la cámara para que siga al jugador
    camara.target = jugador.posicion;

    // --- Lógica de cambio de mapa ---
    // Si el jugador llega al borde derecho del mapa
    if (jugador.posicion.x >= (mapaActual->ancho - 1) * TILE_SIZE) {
        if (mapaActual->este != NULL) {
            // Liberar mapa actual
            for (int y = 0; y < mapaActual->alto; y++) {
                free(mapa[y]);
            }
            free(mapa);
            mapa = NULL;

            mapaActual = mapaActual->este;
            CargarMapa(mapaActual->archivoMapa, &mapa, &mapaActual->ancho, &mapaActual->alto, &jugador);
            jugador.posicion.x = 1 * TILE_SIZE;
        }
    }
    // Si el jugador llega al borde izquierdo del mapa
    else if (jugador.posicion.x <= 0) {
        if (mapaActual->oeste != NULL) {
            for (int y = 0; y < mapaActual->alto; y++) {
                free(mapa[y]);
            }
            free(mapa);
            mapa = NULL;

            mapaActual = mapaActual->oeste;
            CargarMapa(mapaActual->archivoMapa, &mapa, &mapaActual->ancho, &mapaActual->alto, &jugador);
            jugador.posicion.x = (mapaActual->ancho - 2) * TILE_SIZE;
        }
    }
    int tileActual = mapa[tileY][tileX];

    if(tileActual == 6 || tileActual == 7 || tileActual == 8) {
        jugador.vida -= 2; // o cualquier daño que estimes
    }
    if(tileActual == 6) DrawText("¡Electricidad!", 10, 70, 20, RED);
    if(tileActual == 7) DrawText("¡Sierra peligrosa!", 10, 70, 20, RED);
    if(tileActual == 8) DrawText("¡Fuego!", 10, 70, 20, RED);

    if (tileActual == 4) { // Fruta
        AgregarItem(jugador.inventario, "Fruta", 10); // Cura 10
        mapa[tileY][tileX] = 0;
    }

    if (IsKeyPressed(KEY_F)) 
    { // Pulsar F para curarse
        UsarItem(jugador.inventario, "Fruta", &jugador);
    }

    if (IsKeyPressed(KEY_I))   
    {  // Por ejemplo, tecla I abre el inventario
        pantallaDeJuego = INVENTARIO;
        inventarioSeleccionado = 0;  // Reiniciar selección
    }
    
}

void ActualizarGameOver() {
    if (IsKeyPressed(KEY_ENTER)) {
        pantallaDeJuego = MENU; // Vuelve al menú
        jugador.vida = 100; // Vida máxima
        jugador.posicion = jugador.spawn; // Volver al punto de inicio
        // Limpiar inventario si quieres:
        mapaActual = mapaInicial;  
        LimpiarInventario(jugador.inventario);         // Volver a cargar el mapa actual:
        jugador.inventario = createMap(100);
        CargarMapa(mapaActual->archivoMapa, &mapa, &mapaActual->ancho, &mapaActual->alto, &jugador);
    }
}

// --- FUNCIONES DE DIBUJO ---
void DrawMenu(Texture2D menuTexture, float scaleX, float scaleY) {
    DrawTextureEx(menuTexture, (Vector2){0, 0}, 0.0f, scaleX, WHITE);
    
    DrawText("MENU PRINCIPAL", (int)(scaleX * 480), (int)(scaleY * 200), (int)(scaleY * 40), WHITE);
    DrawText("Iniciar Juego", (int)(scaleX * 540), (int)(scaleY * 300), (int)(scaleY * 30), opcionSelecionada == 0 ? RED : WHITE);
    DrawText("Salir", (int)(scaleX * 540), (int)(scaleY * 350), (int)(scaleY * 30), opcionSelecionada == 1 ? RED : WHITE);
    DrawText("Usa Flechas ARRIBA/ABAJO y ENTER para elegir", (int)(scaleX * 360), (int)(scaleY * 500), (int)(scaleY * 20), WHITE);
    /*1er:
    2do:
    3ro:
    4to: parametro modificación del tamaño del texto
    5to: color*/
}

void DibujarInventarioInteractivo(float x, float y, float lineHeight) {
    int totalItems = ContarItems(jugador.inventario);

    DrawRectangle((int)x - 10, (int)y - 10, 250, (int)(lineHeight * (totalItems > 0 ? totalItems : 1) + 20), (Color){0, 0, 0, 180});

    if (totalItems == 0) {
        DrawText("Inventario vacio", (int)x, (int)y, 20, GRAY);
        return;
    }

    for (int i = 0; i < totalItems; i++) {
        Pair* par = ObtenerItemEnPosicion(jugador.inventario, i);
        Item* item = (Item*)par->value;

        Color color = (i == inventarioSeleccionado) ? RED : WHITE;

        char texto[100];
        snprintf(texto, sizeof(texto), "%s x%d (Cura: %d)", item->nombre, item->cantidad, item->curacion);
        DrawText(texto, (int)x, (int)(y + lineHeight * i), 20, color);
    }

    DrawText("ENTER: usar item", (int)x, (int)(y + lineHeight * totalItems + 10), 15, LIGHTGRAY);
    DrawText("ESC: salir", (int)x, (int)(y + lineHeight * totalItems + 30), 15, LIGHTGRAY);
}

void DrawNameInput(float scaleX, float scaleY) {
    DrawText("INGRESA TU NOMBRE:", (int)(scaleX * 400), (int)(scaleY * 200), (int)(scaleY * 30), WHITE);
    DrawText(jugador.nombre, (int)(scaleX * 400), (int)(scaleY * 250), (int)(scaleY * 30), PINK);
    DrawText("Presiona ENTER para continuar", (int)(scaleX * 400), (int)(scaleY * 300), (int)(scaleY * 20), WHITE);
}

void DrawGameplay(float scaleX, float scaleY) {
    BeginMode2D(camara);
        DibujarMapa(mapa, mapaActual->ancho, mapaActual->alto);
        
        // Dibujar sprites externos (enemigos, fuego, etc.)
        Node* nodo = spritesActivos->head;
        while (nodo) {
            Sprite* s = (Sprite*)nodo->data;
            // Actualizar posición si es necesario
            ActualizarSprite(s, GetFrameTime());
            DibujarSprite(s, s->position); // Usar la posición del sprite
            nodo = nodo->next;
        }

        // Dibujar sprite del jugador
        ActualizarSprite(jugador.spriteActual, GetFrameTime());
        DibujarSprite(jugador.spriteActual, 
              (Vector2){jugador.posicion.x, jugador.posicion.y + jugador.offsetVisual.y});

        // Dibujar nombre del jugador
        DrawText(jugador.nombre,
                 (int)(jugador.posicion.x + TILE_SIZE / 2 - MeasureText(jugador.nombre, 20) / 2),
                 (int)(jugador.posicion.y - 25),
                 20, WHITE);
    EndMode2D();

        // --- MINI MAPA ---
    int minimapaAncho = 200;  // tamaño minimapa
    int minimapaAlto = 200;
    int offsetX = GetScreenWidth() - minimapaAncho - 10;  // Esquina superior derecha
    int offsetY = 10;

    // fondo del minimapa
    DrawRectangle(offsetX - 2, offsetY - 2, minimapaAncho + 4, minimapaAlto + 4, DARKGRAY);  // borde
    DrawRectangle(offsetX, offsetY, minimapaAncho, minimapaAlto, BLACK);  // fondo negro

    // tamaño de cada celda en el minimapa
    float tileWidth = (float)minimapaAncho / mapaActual->ancho;
    float tileHeight = (float)minimapaAlto / mapaActual->alto;

    for (int y = 0; y < mapaActual->alto; y++) {
        for (int x = 0; x < mapaActual->ancho; x++) {
            int tile = mapa[y][x];
            Color color = BLANK;

            if (tile == 1 || tile == -1 || tile == -2) color = GRAY;     // plataformas y paredes
            else if (tile == 4) color = GREEN;                           // fruta
            else if (tile == 6 || tile == 7 || tile == 8) color = RED;   // peligros

            if (color.a != 0) {
                DrawRectangle(offsetX + x * tileWidth, offsetY + y * tileHeight, tileWidth, tileHeight, color);
            }
        }
    }

    // dibuja al jugador como un punto rojo
    int playerTileX = (int)(jugador.posicion.x / TILE_SIZE);
    int playerTileY = (int)(jugador.posicion.y / TILE_SIZE);
    DrawRectangle(offsetX + playerTileX * tileWidth, offsetY + playerTileY * tileHeight, tileWidth, tileHeight, BLUE);


    DrawText(TextFormat("Vida: %d", jugador.vida), (int)(10 * scaleX), (int)(10 * scaleY), (int)(20 * scaleY), WHITE);
    DrawText(TextFormat("Pos: %.1f, %.1f", jugador.posicion.x/TILE_SIZE, jugador.posicion.y/TILE_SIZE), (int)(10 * scaleX), (int)(40 * scaleY), (int)(20 * scaleY), WHITE);
    DrawText("Flechas para mover, SPACE para saltar, SHIFT para dash", (int)(10 * scaleX), (int)(GetScreenHeight() - 30 * scaleY), (int)(20 * scaleY), DARKGRAY);

    // Dibujo del mensaje en pantalla
    if (tiempoMensaje > 0.0f) {
        DrawText(mensajePantalla, 10, 60, 20, YELLOW);
    }
}

void personaje_sprites_main(Personaje* jugador)
{
    // Inicializar el jugador usando la función de la biblioteca
    
    (*jugador).spriteIdleStart = CrearSprite("sprites/personaje/prota_idle_start.png", 1, 0.2f, (*jugador).posicion);
    (*jugador).spriteIdleWalk = CrearSprite("sprites/personaje/prota_idle_walk.png", 2, 0.2f, (*jugador).posicion);
    (*jugador).spriteJumpUp = CrearSprite("sprites/personaje/prota_jump_up.png", 1, 0.1f, (*jugador).posicion);
    (*jugador).spriteJumpDown = CrearSprite("sprites/personaje/prota_jump_down.png", 1, 0.1f, (*jugador).posicion);
    (*jugador).spriteClimb = CrearSprite("sprites/personaje/prota_climb.png", 1, 0.2f, (*jugador).posicion);
    (*jugador).spriteRun  = CrearSprite("sprites/personaje/prota_run.png", 1, 0.12f, (*jugador).posicion);
    (*jugador).spriteActual = (*jugador).spriteIdleStart;

}

void DrawGameOver(float scaleX, float scaleY) {
    DrawText("GAME OVER", (int)(scaleX * 500), (int)(scaleY * 300), (int)(scaleY * 50), RED);
    DrawText("Presiona ENTER para volver al menu", (int)(scaleX * 400), (int)(scaleY * 400), (int)(scaleY * 30), WHITE);
}

void sprites_mundo()
{
    spritesActivos = list_create();

    Sprite* enemigo = CrearSprite("sprites/enemigo.png", 1, 0.15f, (Vector2){ 8 * TILE_SIZE + TILE_SIZE / 2, 6 * TILE_SIZE + TILE_SIZE / 2 });
    list_pushBack(spritesActivos, enemigo);

    Sprite* fuego = CrearSprite("sprites/fuego.png", 1, 0.1f, 
        (Vector2){ 10 * TILE_SIZE + TILE_SIZE, 7 * TILE_SIZE + TILE_SIZE});
    list_pushBack(spritesActivos, fuego);

    
    texturasMapa[0] = LoadTexture("sprites/Bloques/pared_morada_ladrillo/pared.JPG");        // para tile 0
    texturasMapa[1] = LoadTexture("sprites/Bloques/suelo.jpeg");        // para tile 1
    texturasMapa[2] = LoadTexture("sprites/Bloques/pared_morada_ladrillo/pared_izq.jpeg");    // para tile -1
    texturasMapa[3] = LoadTexture("sprites/Bloques/pared_morada_ladrillo/pared_der.jpeg");    // para tile -2

    
}
// --- MAIN FUNCTION ---
int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(BASE_ANCHO, BASE_ALTO, "FILESAFE");
    //MaximizeWindow();
    SetTargetFPS(60);

    sprites_mundo();

    // Cargar imagen de menú
    Image image = LoadImage("Menus/Menu_incial.png");
    Texture2D Menu_inicial_imagen = LoadTextureFromImage(image);
    UnloadImage(image);

    // Inicializar la cámara
    camara.target = jugador.posicion;
    camara.offset = (Vector2){ BASE_ANCHO / 2.0f, BASE_ALTO / 2.0f };
    camara.rotation = 0.0f;
    camara.zoom = 1.0f;

    // Crear el grafo de mapas
    NodoMapa* mapa1 = CrearMapa(1, "mapas/mapa1.csv");
    NodoMapa* mapa2 = CrearMapa(2, "mapas/mapa2.csv");
    NodoMapa* mapa3 = CrearMapa(3, "mapas/mapa3.csv");

    mapaInicial = mapa1;
    // Conectar mapas
    ConectarMapas(mapa1, mapa2, 2);
    ConectarMapas(mapa2, mapa1, 3);
    ConectarMapas(mapa2, mapa3, 2);
    ConectarMapas(mapa3, mapa2, 3);

    mapaActual = mapa1;

    
    InicializarPersonaje(&jugador);
    personaje_sprites_main(&jugador);
    CargarMapa(mapaActual->archivoMapa, &mapa, &mapaActual->ancho, &mapaActual->alto, &jugador);
    
    

    // Lista para llevar un registro de todos los nodos del grafo
    NodoMapa* todosLosMapas[] = {mapa1, mapa2, mapa3};
    int cantidadMapas = 3;

    while (!WindowShouldClose()) {
        int pantalla_ancho = GetScreenWidth();
        int pantalla_alto = GetScreenHeight();
        float scaleX = (float)pantalla_ancho / BASE_ANCHO;
        float scaleY = (float)pantalla_alto / BASE_ALTO;

        // Actualizar offset de la cámara
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
                ActualizarGameplay();
                break;
            case GAME_OVER:
                ActualizarGameOver();
                break;
            case INVENTARIO:
                ActualizarInventario();
                break;
        }

        // Dibujo
        BeginDrawing();
        ClearBackground((Color){ 50, 50, 50, 255 });

        switch (pantallaDeJuego) {
            case MENU: DrawMenu(Menu_inicial_imagen, scaleX, scaleY); break;
            case PONER_NOMBRE: DrawNameInput(scaleX, scaleY); break;
            case GAMEPLAY: DrawGameplay(scaleX, scaleY); break;
            case GAME_OVER: DrawGameOver(scaleX, scaleY); break;
            case INVENTARIO: DibujarInventarioInteractivo(50, 50, 30); break;
        }

        EndDrawing();
    }

    // Liberar recursos
    UnloadTexture(Menu_inicial_imagen);
    
    // Liberar memoria del mapa dinámico
    if (mapa != NULL) {
        for (int y = 0; y < mapaActual->alto; y++) {
            free(mapa[y]);
        }
        free(mapa);
        mapa = NULL;
    }
    
    // Liberar nodos del grafo
    for (int i = 0; i < cantidadMapas; i++) {
        free(todosLosMapas[i]->archivoMapa);
    }
    LiberarGrafo(todosLosMapas, cantidadMapas);

    Node* nodo = spritesActivos->head;
    while (nodo) {
        LiberarSprite((Sprite*)nodo->data);
        nodo = nodo->next;
    }
    list_clean(spritesActivos);
    free(spritesActivos);
    for (int i = 0; i < 4; i++) {
    UnloadTexture(texturasMapa[i]);
    }

    CloseWindow();
    return 0;
}