// grafo.c
#include "grafo.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

NodoMapa* CrearMapa(int id, const char* archivo) {
    NodoMapa* nuevoMapa = malloc(sizeof(NodoMapa));
    nuevoMapa->id = id;
    nuevoMapa->archivoMapa = strdup(archivo);
    nuevoMapa->norte = NULL;
    nuevoMapa->sur = NULL;
    nuevoMapa->este = NULL;
    nuevoMapa->oeste = NULL;
    return nuevoMapa;
}

void ConectarMapas(NodoMapa* origen, NodoMapa* destino, int direccion) {
    switch (direccion) {
        case 0: origen->norte = destino; break;
        case 1: origen->sur = destino; break;
        case 2: origen->este = destino; break;
        case 3: origen->oeste = destino; break;
    }
}


void LiberarGrafo(NodoMapa* grafo[], int cantidad) {
    for (int i = 0; i < cantidad; i++) {
        free(grafo[i]);
    }
}
