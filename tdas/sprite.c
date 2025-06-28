#include "sprite.h" 
#include <stdlib.h>
#include <math.h>
#include "raylib.h"

// Definimos el tamaño base que ocupará cada sprite en pantalla
#define TILE_SIZE 64.0f

// Esta función crea e inicializa un nuevo sprite
Sprite* CrearSprite(const char* ruta, int frameCount, float frameTime, Vector2 pos) {
    // Reservamos memoria para la estructura del sprite
    Sprite* s = malloc(sizeof(Sprite));

    // Cargamos la imagen desde la ruta indicada y nos aseguramos de que tenga canal alfa (transparencia)
    Image img = LoadImage(ruta);
    ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8); // Asegura formato con alpha
    s->texture = LoadTextureFromImage(img); // Convertimos la imagen a textura
    UnloadImage(img); // Ya no necesitamos la imagen original

    // Inicializamos los datos del sprite
    s->frameCount = frameCount;     // Cantidad total de cuadros de animación
    s->frameTime = frameTime;       // Tiempo entre cuadros
    s->currentFrame = 0;            // Empezamos desde el primer frame
    s->timeCounter = 0.0f;          // Temporizador para controlar la animación
    s->position = pos;              // Posición inicial del sprite en el mapa
    s->flipX = false;               // Por defecto no se voltea horizontalmente

    // Definimos el rectángulo que representa el frame actual de la animación
    s->frameRec = (Rectangle){
        0, 0,
        (float)s->texture.width / frameCount, // Ancho por frame
        (float)s->texture.height              // Alto completo
    };

    return s;
}

// Esta función actualiza la animación del sprite según el tiempo transcurrido
void ActualizarSprite(Sprite* s, float deltaTime) {
    s->timeCounter += deltaTime;

    // Si ha pasado suficiente tiempo, cambiamos al siguiente frame
    if (s->timeCounter >= s->frameTime) {
        s->currentFrame = (s->currentFrame + 1) % s->frameCount; // Loop de animación
        s->frameRec.x = s->currentFrame * s->frameRec.width;     // Ajustamos el frame visible
        s->timeCounter = 0.0f;                                   // Reiniciamos el contador
    }
}

// Esta función dibuja el sprite en pantalla, aplicando escala y volteo si corresponde
void DibujarSprite(Sprite* s, Vector2 posicion) {
    float escala = TILE_SIZE / s->frameRec.width;

    // Si el sprite debe estar volteado horizontalmente, ajustamos el ancho
    Rectangle sourceRec = s->frameRec;
    if (s->flipX) {
        sourceRec.width = -sourceRec.width;
    }

    // Dibujamos el sprite usando coordenadas y escala personalizadas
    DrawTexturePro(
        s->texture,
        sourceRec,
        (Rectangle){
            posicion.x,
            posicion.y,
            fabs(sourceRec.width) * escala, // Valor absoluto para evitar efectos visuales incorrectos al voltear
            sourceRec.height * escala
        },
        (Vector2){0, 0}, // Punto de origen para rotación
        0.0f,            // Sin rotación
        WHITE            // Color base (sin tintado)
    );
}

// Libera la memoria y recursos asociados al sprite
void LiberarSprite(Sprite* s) {
    if (s) {
        UnloadTexture(s->texture); // Liberamos la textura de la GPU
        free(s);                   // Liberamos la memoria del sprite
    }
}
