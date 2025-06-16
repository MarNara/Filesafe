// grafo.h
#ifndef GRAFO_H
#define GRAFO_H

#define MAX_CONEXIONES 4

typedef struct NodoMapa {
    int id;                        // ID único del mapa
    char* archivoMapa;   
    int ancho;
    int alto;          // Ruta al archivo CSV del mapa
    struct NodoMapa* norte;        // Puntero al mapa al norte
    struct NodoMapa* sur;          // Puntero al mapa al sur
    struct NodoMapa* este;         // Puntero al mapa al este
    struct NodoMapa* oeste;        // Puntero al mapa al oeste
} NodoMapa;

NodoMapa* CrearMapa(int id, const char* archivoMapa);
void ConectarMapas(NodoMapa* origen, NodoMapa* destino, int direccion);
void LiberarGrafo(NodoMapa* grafo[], int cantidad);

#endif