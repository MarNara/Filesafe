#include "tdas/list.h"
#include "tdas/extra.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_FIL 1000
#define MAX_COL 1000

// Variables globales
char playerName[50];
int gameRunning = 0;

//crear estructuras
typedef struct{
    char nombre[10];
    int vida;
    int posicion;
    List* inventario;
}Jugador;

typedef struct{
    char nombre[10];
    int posicion;
    int vida;
    int ataque;
}NPC;

typedef struct{
    char nombre[10];
    char tipo[100];
    int poder; //solo aplica para armas
    int posicion;
}Item;

typedef struct{
    int id[200];
    char nombre[10];
    int matriz_mapa[MAX_FIL][MAX_COL];
    List* npc;
    List* items;
}Escenario;

typedef struct{
    List* escenarios;
    Jugador* personaje_del_jugador;
    Escenario* escenario_actual;
}Juego;
// Prototipos
void elegirNombre();
void menuInicial();
void menuJuego();
void menuLucha();

void elegirNombre() {
    printf("\n--- Elegir Nombre ---\n");
    printf("Ingrese su nombre: ");
    scanf("%s", playerName);
    printf("¡Bienvenido, %s!\n\n", playerName);
}

void menuInicial() {
    int opcion;
    do {
        printf("\n--- MENÚ INICIAL ---\n");
        printf("1. Iniciar partida\n");
        printf("2. Salir\n");
        printf("Seleccione una opción: ");
        scanf("%d", &opcion);

        switch(opcion) {
            case 1:
                elegirNombre();
                gameRunning = 1;
                menuJuego();
                break;
            case 2:
                printf("Saliendo del juego...\n");
                exit(0);
                break;
            default:
                printf("Opción no válida. Intente de nuevo.\n");
        }
    } while (1);
}

void menuJuego() {
    int opcion;
    while (gameRunning) {
        printf("\n--- MENÚ DEL JUEGO ---\n");
        printf("1. Tomar ítem\n");
        printf("2. Interactuar con personaje\n");
        printf("3. Avanzar\n");
        printf("4. Reiniciar partida\n");
        printf("5. Salir del juego\n");
        printf("Seleccione una opción: ");
        scanf("%d", &opcion);

        switch(opcion) {
            case 1:
                printf("%s tomó un ítem.\n", playerName);
                break;
            case 2:
                printf("%s se encuentra con un personaje...\n", playerName);
                menuLucha(); // Podría derivar en lucha o interacción
                break;
            case 3:
                printf("%s avanza al siguiente escenario.\n", playerName);
                break;
            case 4:
                printf("Reiniciando partida...\n");
                gameRunning = 0;
                menuInicial();
                break;
            case 5:
                printf("Saliendo del juego...\n");
                exit(0);
                break;
            default:
                printf("Opción no válida. Intente nuevamente.\n");
        }
    }
}

void menuLucha() {
    int opcion;
    printf("\n--- MENÚ DE LUCHA ---\n");
    printf("1. Luchar\n");
    printf("2. Hackear personaje\n");
    printf("3. Usar ítems\n");
    printf("4. Huir\n");
    printf("Seleccione una opción: ");
    scanf("%d", &opcion);

    switch(opcion) {
        case 1:
            printf("¡%s lucha valientemente!\n", playerName);
            break;
        case 2:
            printf("%s intenta hackear al personaje...\n", playerName);
            break;
        case 3:
            printf("%s usa un ítem.\n", playerName);
            break;
        case 4:
            printf("%s decide huir de la batalla.\n", playerName);
            break;
        default:
            printf("Opción no válida.\n");
    }
}

int main() {
    menuInicial();
    return 0;
}
