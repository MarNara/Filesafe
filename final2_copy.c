#include "raylib.h"
#include "tdas/list.h"
#include "tdas/grafo.h"
#include "tdas/extra.h"
#include "tdas/movimiento.h"
#include "tdas/hashmap.h"
#include "tdas/sprite.h"
#include "tdas/sprites_mapa.h"
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
    char nombre[20];
    int cantidad;
    int curacion;
} Item;

// --- ENUMERACIONES Y ESTRUCTURAS ---
typedef enum GameScreen {
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
int** mapa;
NodoMapa* mapaInicial = NULL;

char mensajePantalla[256] = "";
float tiempoMensaje = 0.0f;

NodoMapa** todosLosMapas = NULL;
int cantidadMapas = 0;
HashMap* frutasRecolectadas = NULL;

GameScreen pantallaDeJuego = MENU;
int opcionSelecionada = 0;
int inventarioSeleccionado = 0;

Personaje jugador;
Camera2D camara = {0};

List* spritesActivos = NULL;
Texture2D texturasMapa[4];

// --- FUNCION AUXILIAR PARA MENSAJES EN PANTALLA ---
void MostrarMensaje(const char* mensaje) {
    strncpy(mensajePantalla, mensaje, sizeof(mensajePantalla) - 1);
    mensajePantalla[sizeof(mensajePantalla) - 1] = '\0';
    tiempoMensaje = MENSAJE_DURACION;
}

// --- FUNCIONES INVENTARIO ---
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

void freeList(void *value) {
    if (value == NULL) return;
    
    List *list = (List*)value;
    Node *current = list->head;
    while (current != NULL) {
        Node *next = current->next;
        free(current->data);
        free(current);
        current = next;
    }
    free(list);
}

void LimpiarInventario(HashMap* inventario) {
    if (!inventario) return;

    Pair* par = firstMap(inventario);
    while (par) {
        free(par->value);
        par = nextMap(inventario);
    }

    par = firstMap(inventario);
    while (par != NULL) {
        eraseMap(inventario, par->key);
        par = firstMap(inventario);
    }
}

// --- FUNCIONES AUXILIARES ---

void CargarMapa(const char* nombreArchivo, int*** mapaPtr, int* ancho, int* alto, Personaje* jugador) {
    FILE* archivo = fopen(nombreArchivo, "r");
    if (!archivo) {
        printf("Error: No se pudo abrir el archivo del mapa: %s\n", nombreArchivo);
        exit(1);
    }

    if (*mapaPtr != NULL) {
        for (int y = 0; y < *alto; y++) free((*mapaPtr)[y]);
        free(*mapaPtr);
    }

    fscanf(archivo, "%d\n", ancho);
    fscanf(archivo, "%d\n", alto);

    if (mapaActual != NULL) {
        mapaActual->ancho = *ancho;
        mapaActual->alto = *alto;
    }

    *mapaPtr = (int**)malloc((*alto) * sizeof(int*));
    for (int y = 0; y < *alto; y++) {
        (*mapaPtr)[y] = (int*)malloc((*ancho) * sizeof(int));
    }

    for (int y = 0; y < *alto; y++) {
        for (int x = 0; x < *ancho; x++) {
            if (x < (*ancho - 1)) fscanf(archivo, "%d,", &(*mapaPtr)[y][x]);
            else fscanf(archivo, "%d\n", &(*mapaPtr)[y][x]);

            if ((*mapaPtr)[y][x] == 9) {
                (*jugador).posicion = (Vector2){ (float)x * TILE_SIZE, (float)y * TILE_SIZE };
                (*jugador).spawn = (*jugador).posicion;
                (*mapaPtr)[y][x] = 0;
            }
            Pair* frutasMapa = searchMap(frutasRecolectadas, (char*)nombreArchivo);
            if (frutasMapa) {
                List* posiciones = (List*)frutasMapa->value;
                Node* current = posiciones->head;
                while (current) {
                    Vector2* pos = (Vector2*)current->data;
                    (*mapaPtr)[(int)pos->y][(int)pos->x] = 0;
                    current = current->next;
                }
            }
        }
    }
    fclose(archivo);

    // Procesar sprites del mapa (fuego, enemigos, etc.)
    ProcesarMapaParaSprites(*mapaPtr, *alto, *ancho);

    // Añadir botiquines (tile 4) como sprites especiales
    for (int y = 0; y < *alto; y++) {
        for (int x = 0; x < *ancho; x++) {
            if ((*mapaPtr)[y][x] == 4) {
                Vector2 pos = { 
                    x * TILE_SIZE + TILE_SIZE/2, 
                    y * TILE_SIZE + TILE_SIZE/2 - 10  // Ajuste vertical para alinear con el suelo
                };
                Sprite* botiquin = CrearSprite("sprites/botiquin.png", 1, 0.2f, pos, SPRITE_OBJETO);
                list_pushBack(spritesActivos, botiquin);
            }
        }
    }
}

void DibujarMapa(int** mapa, int ancho, int alto) {
    for (int y = 0; y < alto; y++) {
        for (int x = 0; x < ancho; x++) {
            int tipo = mapa[y][x];
            Rectangle destino = { x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE };

            switch (tipo) {
                case 0: case 1: case -1: case -2: {
                    int index = (tipo == 0) ? 0 : (tipo == 1) ? 1 : (tipo == -1) ? 2 : 3;
                    Texture2D textura = texturasMapa[index];
                    Rectangle origen = { 0, 0, (float)textura.width, (float)textura.height };
                    DrawTexturePro(textura, origen, destino, (Vector2){0, 0}, 0.0f, WHITE);
                    break;
                }
                case 2: DrawRectangleRec(destino, RED); break;
                case 3: DrawRectangleRec(destino, BLUE); break;
                //case 4: ya no se dibuja aquí, se maneja con sprite
                case 5: DrawRectangleRec(destino, ORANGE); break;
                case 6: DrawRectangleRec(destino, GREEN); break;
                case 7: DrawRectangleRec(destino, PURPLE); break;
                case 8: DrawRectangleRec(destino, GOLD); break;
                default: DrawRectangleRec(destino, BLACK); break;
            }
        }
    }
}

bool VerificarColision(int tileX, int tileY, int anchoMapa, int altoMapa) {
    if (tileX < 0 || tileY < 0 || tileX >= anchoMapa || tileY >= altoMapa) 
        return false;
    
    int tipo = mapa[tileY][tileX];
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

    if (IsKeyPressed(KEY_BACKSPACE) && len > 0) jugador.nombre[len - 1] = '\0';
    if (IsKeyPressed(KEY_ENTER) && len > 0) pantallaDeJuego = GAMEPLAY;
}

void ActualizarInventario() {
    int totalItems = ContarItems(jugador.inventario);

    if (IsKeyPressed(KEY_DOWN)) inventarioSeleccionado++;
    if (IsKeyPressed(KEY_UP)) inventarioSeleccionado--;
    if (inventarioSeleccionado >= totalItems) inventarioSeleccionado = 0;
    if (inventarioSeleccionado < 0) inventarioSeleccionado = totalItems - 1;

    if (IsKeyPressed(KEY_ENTER)) {
        Pair* par = ObtenerItemEnPosicion(jugador.inventario, inventarioSeleccionado);
        if (par != NULL) {
            UsarItem(jugador.inventario, par->key, &jugador);
            int nuevosItems = ContarItems(jugador.inventario);
            if (inventarioSeleccionado >= nuevosItems) inventarioSeleccionado = nuevosItems - 1;
        }
    }

    if (IsKeyPressed(KEY_I)) pantallaDeJuego = GAMEPLAY;
}

// --- LÓGICA DE JUEGO ---
void LimpiarListaSprites(List* lista) {
    if (lista == NULL) return;
    Node* current = lista->head;
    while (current != NULL) {
        Node* next = current->next;
        Sprite* s = (Sprite*)current->data;
        LiberarSprite(s);
        free(current);
        current = next;
    }
    lista->head = NULL;
    lista->tail = NULL;
    lista->size = 0;
}

void ActualizarGameplay() {
    int tileX = (int)(jugador.posicion.x / TILE_SIZE);
    int tileY = (int)(jugador.posicion.y / TILE_SIZE);

    float delta = GetFrameTime();
    if (tiempoMensaje > 0.0f) {   
        tiempoMensaje -= GetFrameTime();
        if (tiempoMensaje < 0.0f) tiempoMensaje = 0.0f;
    }

    ActualizarMovimiento(&jugador, mapaActual->ancho, mapaActual->alto, delta, VerificarColision);

    if (jugador.vida <= 0) pantallaDeJuego = GAME_OVER;
    
    camara.target = jugador.posicion;

    // Cambio de mapa (derecha)
    if (jugador.posicion.x >= (mapaActual->ancho - 1) * TILE_SIZE) {
        if (mapaActual->este != NULL) {
            LimpiarListaSprites(spritesActivos);
            
            for (int y = 0; y < mapaActual->alto; y++) free(mapa[y]);
            free(mapa);
            mapa = NULL;

            mapaActual = mapaActual->este;
            CargarMapa(mapaActual->archivoMapa, &mapa, &mapaActual->ancho, &mapaActual->alto, &jugador);
            jugador.posicion.x = 1 * TILE_SIZE;
        }
    }
    // Cambio de mapa (izquierda)
    else if (jugador.posicion.x <= 0) {
        if (mapaActual->oeste != NULL) {
            LimpiarListaSprites(spritesActivos);
            
            for (int y = 0; y < mapaActual->alto; y++) free(mapa[y]);
            free(mapa);
            mapa = NULL;

            mapaActual = mapaActual->oeste;
            CargarMapa(mapaActual->archivoMapa, &mapa, &mapaActual->ancho, &mapaActual->alto, &jugador);
            jugador.posicion.x = (mapaActual->ancho - 2) * TILE_SIZE;
        }
    }
    
    int tileActual = mapa[tileY][tileX];

    if(tileActual == 6 || tileActual == 7 || tileActual == 8) jugador.vida -= 2;
    if(tileActual == 6) DrawText("¡Electricidad!", 10, 70, 20, RED);
    if(tileActual == 7) DrawText("¡Sierra peligrosa!", 10, 70, 20, RED);
    if(tileActual == 8) DrawText("¡Fuego!", 10, 70, 20, RED);

    if (tileActual == 4) {
        Vector2* posFruta = malloc(sizeof(Vector2));
        posFruta->x = tileX;
        posFruta->y = tileY;
        
        Pair* par = searchMap(frutasRecolectadas, (char*)mapaActual->archivoMapa);
        if (!par) {
            List* lista = list_create();
            list_pushBack(lista, posFruta);
            insertMap(frutasRecolectadas, strdup(mapaActual->archivoMapa), lista);
        } else {
            List* lista = (List*)par->value;
            list_pushBack(lista, posFruta);
        }
        
        AgregarItem(jugador.inventario, "Botiquin", 10);
        mapa[tileY][tileX] = 0;
        
        Vector2 posSprite = { tileX * TILE_SIZE + TILE_SIZE/2, tileY * TILE_SIZE + TILE_SIZE/2 };
        EliminarSpritePorPosicion(spritesActivos, posSprite);
    }

    if (IsKeyPressed(KEY_F)) UsarItem(jugador.inventario, "Botiquin", &jugador);
    if (IsKeyPressed(KEY_I)) {
        pantallaDeJuego = INVENTARIO;
        inventarioSeleccionado = 0;
    }
}

void ActualizarGameOver() {
    if (IsKeyPressed(KEY_ENTER)) {
        pantallaDeJuego = MENU;
        jugador.vida = 100;
        mapaActual = mapaInicial;  
        LimpiarInventario(jugador.inventario);
        jugador.inventario = createMap(100);
        
        for (int i = 0; i < cantidadMapas; i++) {
            free(todosLosMapas[i]->archivoMapa);
            free(todosLosMapas[i]);
            
            if (i == 0) todosLosMapas[i] = CrearMapa(1, "mapas/mapa1.csv");
            else if (i == 1) todosLosMapas[i] = CrearMapa(2, "mapas/mapa2.csv");
            else todosLosMapas[i] = CrearMapa(3, "mapas/mapa3.csv");
        }
        
        ConectarMapas(todosLosMapas[0], todosLosMapas[1], 2);
        ConectarMapas(todosLosMapas[1], todosLosMapas[0], 3);
        ConectarMapas(todosLosMapas[1], todosLosMapas[2], 2);
        ConectarMapas(todosLosMapas[2], todosLosMapas[1], 3);
        
        mapaInicial = todosLosMapas[0];
        mapaActual = mapaInicial;

        if (frutasRecolectadas != NULL) cleanMap(frutasRecolectadas, freeList);
        frutasRecolectadas = createMap(100);
        
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
    DrawText("I: salir", (int)x, (int)(y + lineHeight * totalItems + 30), 15, LIGHTGRAY);
}

void DrawNameInput(float scaleX, float scaleY) {
    DrawText("INGRESA TU NOMBRE:", (int)(scaleX * 400), (int)(scaleY * 200), (int)(scaleY * 30), WHITE);
    DrawText(jugador.nombre, (int)(scaleX * 400), (int)(scaleY * 250), (int)(scaleY * 30), PINK);
    DrawText("Presiona ENTER para continuar", (int)(scaleX * 400), (int)(scaleY * 300), (int)(scaleY * 20), WHITE);
}

void DrawGameplay(float scaleX, float scaleY) {
    BeginMode2D(camara);
        DibujarMapa(mapa, mapaActual->ancho, mapaActual->alto);
        
        Node* nodo = spritesActivos->head;
        while (nodo) {
            Sprite* s = (Sprite*)nodo->data;
            ActualizarSprite(s, GetFrameTime());
            DibujarSprite(s, s->position);
            nodo = nodo->next;
        }

        ActualizarSprite(jugador.spriteActual, GetFrameTime());
        DibujarSprite(jugador.spriteActual, 
              (Vector2){jugador.posicion.x, jugador.posicion.y + jugador.offsetVisual.y});

        DrawText(jugador.nombre,
                 (int)(jugador.posicion.x + TILE_SIZE / 2 - MeasureText(jugador.nombre, 20) / 2),
                 (int)(jugador.posicion.y - 25),
                 20, WHITE);
    EndMode2D();

    int minimapaAncho = 200;
    int minimapaAlto = 200;
    int offsetX = GetScreenWidth() - minimapaAncho - 10;
    int offsetY = 10;

    DrawRectangle(offsetX - 2, offsetY - 2, minimapaAncho + 4, minimapaAlto + 4, DARKGRAY);
    DrawRectangle(offsetX, offsetY, minimapaAncho, minimapaAlto, BLACK);

    float tileWidth = (float)minimapaAncho / mapaActual->ancho;
    float tileHeight = (float)minimapaAlto / mapaActual->alto;

    for (int y = 0; y < mapaActual->alto; y++) {
        for (int x = 0; x < mapaActual->ancho; x++) {
            int tile = mapa[y][x];
            Color color = BLANK;

            if (tile == 1 || tile == -1 || tile == -2) color = GRAY;
            else if (tile == 4) color = GREEN;
            else if (tile == 6 || tile == 7 || tile == 8) color = RED;

            if (color.a != 0) DrawRectangle(offsetX + x * tileWidth, offsetY + y * tileHeight, tileWidth, tileHeight, color);
        }
    }

    int playerTileX = (int)(jugador.posicion.x / TILE_SIZE);
    int playerTileY = (int)(jugador.posicion.y / TILE_SIZE);
    DrawRectangle(offsetX + playerTileX * tileWidth, offsetY + playerTileY * tileHeight, tileWidth, tileHeight, BLUE);

    DrawText(TextFormat("Vida: %d", jugador.vida), (int)(10 * scaleX), (int)(10 * scaleY), (int)(20 * scaleY), WHITE);
    DrawText(TextFormat("Pos: %.1f, %.1f", jugador.posicion.x/TILE_SIZE, jugador.posicion.y/TILE_SIZE), (int)(10 * scaleX), (int)(40 * scaleY), (int)(20 * scaleY), WHITE);
    DrawText("Flechas para mover, SPACE para saltar, SHIFT para dash", (int)(10 * scaleX), (int)(GetScreenHeight() - 30 * scaleY), (int)(20 * scaleY), DARKGRAY);

    if (tiempoMensaje > 0.0f) DrawText(mensajePantalla, 10, 60, 20, YELLOW);
}

void DrawGameOver(float scaleX, float scaleY) {
    DrawText("GAME OVER", (int)(scaleX * 500), (int)(scaleY * 300), (int)(scaleY * 50), RED);
    DrawText("Presiona ENTER para volver al menu", (int)(scaleX * 400), (int)(scaleY * 400), (int)(scaleY * 30), WHITE);
}

void personaje_sprites_main(Personaje* jugador) {
    (*jugador).spriteIdleStart = CrearSprite("sprites/personaje/prota_idle_start.png", 1, 0.2f, (*jugador).posicion, SPRITE_PERSONAJE);
    (*jugador).spriteIdleWalk = CrearSprite("sprites/personaje/prota_idle_walk.png", 2, 0.2f, (*jugador).posicion, SPRITE_PERSONAJE);
    (*jugador).spriteJumpUp = CrearSprite("sprites/personaje/prota_jump_up.png", 1, 0.1f, (*jugador).posicion, SPRITE_PERSONAJE);
    (*jugador).spriteJumpDown = CrearSprite("sprites/personaje/prota_jump_down.png", 1, 0.1f, (*jugador).posicion, SPRITE_PERSONAJE);
    (*jugador).spriteClimb = CrearSprite("sprites/personaje/prota_climb.png", 1, 0.2f, (*jugador).posicion, SPRITE_PERSONAJE);
    (*jugador).spriteRun  = CrearSprite("sprites/personaje/prota_run.png", 1, 0.12f, (*jugador).posicion, SPRITE_PERSONAJE);
    (*jugador).spriteActual = (*jugador).spriteIdleStart;
}

void sprites_mundo() {
    spritesActivos = list_create();
    
    texturasMapa[0] = LoadTexture("sprites/Bloques/pared_morada_ladrillo/pared.JPG");
    texturasMapa[1] = LoadTexture("sprites/Bloques/suelo.jpeg");
    texturasMapa[2] = LoadTexture("sprites/Bloques/pared_morada_ladrillo/pared_izq.jpeg");
    texturasMapa[3] = LoadTexture("sprites/Bloques/pared_morada_ladrillo/pared_der.jpeg");
}

// --- MAIN FUNCTION ---
int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(BASE_ANCHO, BASE_ALTO, "FILESAFE");
    SetTargetFPS(60);

    sprites_mundo();

    Image image = LoadImage("Menus/Menu_incial.png");
    Texture2D Menu_inicial_imagen = LoadTextureFromImage(image);
    UnloadImage(image);

    frutasRecolectadas = createMap(10);

    camara.target = jugador.posicion;
    camara.offset = (Vector2){ BASE_ANCHO / 2.0f, BASE_ALTO / 2.0f };
    camara.rotation = 0.0f;
    camara.zoom = 1.0f;

    cantidadMapas = 3;
    todosLosMapas = malloc(sizeof(NodoMapa*) * cantidadMapas);
    
    todosLosMapas[0] = CrearMapa(1, "mapas/mapa1.csv");
    todosLosMapas[1] = CrearMapa(2, "mapas/mapa2.csv");
    todosLosMapas[2] = CrearMapa(3, "mapas/mapa3.csv");
    
    mapaInicial = todosLosMapas[0];
    
    ConectarMapas(todosLosMapas[0], todosLosMapas[1], 2);
    ConectarMapas(todosLosMapas[1], todosLosMapas[0], 3);
    ConectarMapas(todosLosMapas[1], todosLosMapas[2], 2);
    ConectarMapas(todosLosMapas[2], todosLosMapas[1], 3);
    
    mapaActual = mapaInicial;

    InicializarPersonaje(&jugador);
    personaje_sprites_main(&jugador);
    CargarMapa(mapaActual->archivoMapa, &mapa, &mapaActual->ancho, &mapaActual->alto, &jugador);

    while (!WindowShouldClose()) {
        int pantalla_ancho = GetScreenWidth();
        int pantalla_alto = GetScreenHeight();
        float scaleX = (float)pantalla_ancho / BASE_ANCHO;
        float scaleY = (float)pantalla_alto / BASE_ALTO;

        camara.offset = (Vector2){ pantalla_ancho / 2.0f, pantalla_alto / 2.0f };

        switch (pantallaDeJuego) {
            case MENU: ActualizarMenu(); break;
            case PONER_NOMBRE: ActualizarPonerNombre(); break;
            case GAMEPLAY: ActualizarGameplay(); break;
            case GAME_OVER: ActualizarGameOver(); break;
            case INVENTARIO: ActualizarInventario(); break;
        }

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

    UnloadTexture(Menu_inicial_imagen);
    
    if (mapa != NULL) {
        for (int y = 0; y < mapaActual->alto; y++) free(mapa[y]);
        free(mapa);
    }
    
    if (todosLosMapas) {
        for (int i = 0; i < cantidadMapas; i++) {
            free(todosLosMapas[i]->archivoMapa);
            free(todosLosMapas[i]);
        }
        free(todosLosMapas);
    }
    
    if (frutasRecolectadas) cleanMap(frutasRecolectadas, freeList);
    
    Node* nodo = spritesActivos->head;
    while (nodo) {
        LiberarSprite((Sprite*)nodo->data);
        Node* next = nodo->next;
        free(nodo);
        nodo = next;
    }
    free(spritesActivos);
    for (int i = 0; i < 4; i++) UnloadTexture(texturasMapa[i]);

    CloseWindow();
    return 0;
}