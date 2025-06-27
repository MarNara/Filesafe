#ifndef PELEA_H
#define PELEA_H

#include <stdbool.h>

typedef struct {
    char nombre[32];
    int vida;
    bool esJugador;
} Combatiente;

// Esta función "corre" la pelea y devuelve true si el jugador ganó, false si perdió o liberó al enemigo
// Puede recibir los combatientes iniciales y actualizar sus vidas tras la pelea.
bool iniciar_pelea(Combatiente *jugador, Combatiente *enemigo);

#endif // PELEA_H