#ifndef PELEA_H
#define PELEA_H

#include <stdbool.h>
#include "hashmap.h" // Asegúrate de que hashmap.h esté incluido y que Pair y HashMap estén definidos.

// Definición de Combatiente
typedef struct {
    char nombre[32];
    int vida;
    bool esJugador;
} Combatiente;

// Enum para las opciones del menú de combate
typedef enum {
    OPCION_LUCHAR = 0,
    OPCION_MOCHILA,
    OPCION_LIBERAR,
    OPCION_TOTAL
} OpcionMenu;

// Esta función "corre" la pelea y devuelve true si el jugador ganó, false si perdió o liberó al enemigo
// Puede recibir los combatientes iniciales y actualizar sus vidas tras la pelea.
bool iniciar_pelea(Combatiente *jugador, Combatiente *enemigo, HashMap* inventario);

// DECLARACIÓN ACTUALIZADA DE mostrarInventario para el uso en combate
// Ahora incluye un puntero a la selección del menú de combate para permitir regresar a él.
void mostrarInventario(HashMap* inventario, Combatiente* jugador, bool* mostrando, bool* turnoJugador, OpcionMenu* seleccionMenuPelea);

#endif // PELEA_H
