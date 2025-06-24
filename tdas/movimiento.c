#include "movimiento.h"
#include "hashmap.h"
#include <math.h>

// Función para inicializar el personaje con valores por defecto
void InicializarPersonaje(Personaje *jugador) {
    *jugador = (Personaje){
        .nombre = "",
        .vida = 100,
        .ataque = 10,
        .defensa = 5,
        .posicion = {0, 0},
        .velocidad = {0, 0},
        .inventario = createMap(100),
        .enSuelo = false,
        .tiempoCoyote = TIEMPO_COYOTE,
        .contadorCoyote = 0.0f,
        .tiempoBufferSalto = TIEMPO_BUFFER_SALTO,
        .contadorBufferSalto = 0.0f,
        .puedeDash = true,
        .estaDashing = false,
        .tiempoDash = TIEMPO_DASH,
        .contadorDash = 0.0f,
        .puedeDobleSalto = true,
        .enParedIzquierda = false,
        .enParedDerecha = false,
        .contadorSaltoPared = 0.0f
    };
}

// Procesa las entradas del jugador (salto y dash)
void ProcesarEntradas(Personaje *jugador) {
    if (IsKeyPressed(KEY_SPACE)) {
        jugador->contadorBufferSalto = jugador->tiempoBufferSalto;
    }

    if (IsKeyPressed(KEY_LEFT_SHIFT) && jugador->puedeDash) {
        jugador->estaDashing = true;
        jugador->puedeDash = false;
        jugador->contadorDash = jugador->tiempoDash;
        jugador->velocidad.y = 0;
        jugador->velocidad.x = (IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT)) * VELOCIDAD_DASH;
    }
}

// Aplica física básica al jugador (movimiento, gravedad, fricción)
void AplicarFisicas(Personaje *jugador, float delta) {
    if (!jugador->estaDashing) {
        // Movimiento horizontal
        if (IsKeyDown(KEY_RIGHT)) {
            jugador->velocidad.x += ACELERACION * delta;
        } else if (IsKeyDown(KEY_LEFT)) {
            jugador->velocidad.x -= ACELERACION * delta;
        } else {
            // Fricción cuando no hay movimiento
            if (jugador->velocidad.x > 0) {
                jugador->velocidad.x = fmaxf(jugador->velocidad.x - FRICCION * delta, 0);
            } else if (jugador->velocidad.x < 0) {
                jugador->velocidad.x = fminf(jugador->velocidad.x + FRICCION * delta, 0);
            }
        }

        // Limitar velocidad horizontal
        jugador->velocidad.x = fminf(fmaxf(jugador->velocidad.x, -VELOCIDAD_MOVIMIENTO), VELOCIDAD_MOVIMIENTO);
    }

    // Gravedad (solo si no está en dash)
    if (!jugador->estaDashing) {
        jugador->velocidad.y += GRAVITY * delta;
        // Limitar velocidad de caída
        if (jugador->velocidad.y > VELOCIDAD_MAX_CAIDA) {
            jugador->velocidad.y = VELOCIDAD_MAX_CAIDA;
        }
    }
}

// Maneja la duración y finalización del dash
void ManejarDash(Personaje *jugador, float delta) {
    if (jugador->estaDashing) {
        jugador->contadorDash -= delta;
        if (jugador->contadorDash <= 0) {
            jugador->estaDashing = false;
        }
    }
}

// Maneja las colisiones con el entorno
void ManejarColisiones(Personaje *jugador, Vector2 nuevaPos, int anchoMapa, int altoMapa, bool (*VerificarColision)(int, int, int, int)) {
    // --- Colisión Horizontal ---
    int col_izq_px = (int)nuevaPos.x;
    int col_der_px = (int)(nuevaPos.x + TILE_SIZE - 1);
    int col_arriba_y_px = (int)jugador->posicion.y;
    int col_abajo_y_px = (int)(jugador->posicion.y + TILE_SIZE - 1);

    int col_izq_tile = col_izq_px / TILE_SIZE;
    int col_der_tile = col_der_px / TILE_SIZE;
    int col_arriba_y_tile = col_arriba_y_px / TILE_SIZE;
    int col_abajo_y_tile = col_abajo_y_px / TILE_SIZE;

    if (jugador->velocidad.x > 0) { // Moviéndose a la derecha
        if (VerificarColision(col_der_tile, col_arriba_y_tile, anchoMapa, altoMapa) ||
            VerificarColision(col_der_tile, col_abajo_y_tile, anchoMapa, altoMapa)) {
            nuevaPos.x = col_der_tile * TILE_SIZE - TILE_SIZE;
            jugador->velocidad.x = 0;
        }
    } else if (jugador->velocidad.x < 0) { // Moviéndose a la izquierda
        if (VerificarColision(col_izq_tile, col_arriba_y_tile, anchoMapa, altoMapa) ||
            VerificarColision(col_izq_tile, col_abajo_y_tile, anchoMapa, altoMapa)) {
            nuevaPos.x = (col_izq_tile + 1) * TILE_SIZE;
            jugador->velocidad.x = 0;
        }
    }

    // --- Colisión Vertical ---
    int col_arriba_px = (int)nuevaPos.y;
    int col_abajo_px = (int)(nuevaPos.y + TILE_SIZE - 1);
    int col_izq_x_px = (int)nuevaPos.x;
    int col_der_x_px = (int)(nuevaPos.x + TILE_SIZE - 1);

    int col_arriba_tile = col_arriba_px / TILE_SIZE;
    int col_abajo_tile = col_abajo_px / TILE_SIZE;
    int col_izq_x_tile = col_izq_x_px / TILE_SIZE;
    int col_der_x_tile = col_der_x_px / TILE_SIZE;

    jugador->enSuelo = false; // Resetear en cada frame

    if (jugador->velocidad.y >= 0) { // Cayendo o en reposo vertical
        if (VerificarColision(col_izq_x_tile, col_abajo_tile, anchoMapa, altoMapa) ||
            VerificarColision(col_der_x_tile, col_abajo_tile, anchoMapa, altoMapa)) {
            nuevaPos.y = col_abajo_tile * TILE_SIZE - TILE_SIZE;
            jugador->velocidad.y = 0;
            jugador->enSuelo = true;
            jugador->puedeDobleSalto = true;
            jugador->puedeDash = true;
        }
    } else { // Subiendo (salto)
        if (VerificarColision(col_izq_x_tile, col_arriba_tile, anchoMapa, altoMapa) ||
            VerificarColision(col_der_x_tile, col_arriba_tile, anchoMapa, altoMapa)) {
            nuevaPos.y = (col_arriba_tile + 1) * TILE_SIZE;
            jugador->velocidad.y = 0;
        }
    }

    jugador->posicion = nuevaPos;
}

// Detecta si el jugador está tocando una pared
void ManejarParedes(Personaje *jugador, int anchoMapa, int altoMapa, bool (*VerificarColision)(int, int, int, int)) {
    jugador->enParedIzquierda = 
        VerificarColision(
            (int)floorf((jugador->posicion.x - 1) / TILE_SIZE),
            (int)floorf(jugador->posicion.y / TILE_SIZE),
            anchoMapa, altoMapa
        ) || 
        VerificarColision(
            (int)floorf((jugador->posicion.x - 1) / TILE_SIZE),
            (int)floorf((jugador->posicion.y + TILE_SIZE - 1) / TILE_SIZE),
            anchoMapa, altoMapa
        );

    jugador->enParedDerecha = 
        VerificarColision(
            (int)floorf((jugador->posicion.x + TILE_SIZE) / TILE_SIZE),
            (int)floorf(jugador->posicion.y / TILE_SIZE),
            anchoMapa, altoMapa
        ) || 
        VerificarColision(
            (int)floorf((jugador->posicion.x + TILE_SIZE) / TILE_SIZE),
            (int)floorf((jugador->posicion.y + TILE_SIZE - 1) / TILE_SIZE),
            anchoMapa, altoMapa
        );
}

// Maneja todos los tipos de saltos (normal, doble, en pared)
void ManejarSaltos(Personaje *jugador, float delta) {
    // Actualizar coyote time
    if (jugador->enSuelo) {
        jugador->contadorCoyote = jugador->tiempoCoyote;
    } else {
        jugador->contadorCoyote -= delta;
    }

    // Actualizar buffer de salto
    jugador->contadorBufferSalto -= delta;

    // Manejar saltos
    if (jugador->contadorBufferSalto > 0.0f && 
        (jugador->contadorCoyote > 0.0f || jugador->puedeDobleSalto || jugador->enParedIzquierda || jugador->enParedDerecha)) {
        
        if (jugador->enParedIzquierda || jugador->enParedDerecha) {
            jugador->velocidad.y = JUMP_FORCE;
            jugador->velocidad.x = (jugador->enParedIzquierda ? VELOCIDAD_MOVIMIENTO : -VELOCIDAD_MOVIMIENTO);
            jugador->contadorSaltoPared = TIEMPO_SALTO_PARED;
        } else {
            jugador->velocidad.y = JUMP_FORCE;
            if (!jugador->enSuelo && jugador->puedeDobleSalto) {
                jugador->puedeDobleSalto = false;
            }
        }
        
        jugador->enSuelo = false;
        jugador->contadorCoyote = 0.0f;
        jugador->contadorBufferSalto = 0.0f;
    }

    // Salto de altura variable (mantener/release)
    if (IsKeyReleased(KEY_SPACE) && jugador->velocidad.y < 0) {
        jugador->velocidad.y *= 0.5f;
    }

    // Manejar tiempo de salto en pared
    if (jugador->contadorSaltoPared > 0) {
        jugador->contadorSaltoPared -= delta;
        if (jugador->contadorSaltoPared > 0) {
            if ((jugador->velocidad.x > 0 && IsKeyDown(KEY_LEFT)) || 
                (jugador->velocidad.x < 0 && IsKeyDown(KEY_RIGHT))) {
                jugador->velocidad.x = 0;
            }
        }
    }
}

// Función principal que coordina todas las acciones de movimiento
void ActualizarMovimiento(Personaje *jugador, int anchoMapa, int altoMapa, float delta, bool (*VerificarColision)(int, int, int, int)) {
    ProcesarEntradas(jugador);

    // 1. Saltos antes del movimiento para que el salto tenga efecto antes de mover
    ManejarSaltos(jugador, delta);

    // 2. Físicas generales
    AplicarFisicas(jugador, delta);

    // 3. Dash (modifica velocidad)
    ManejarDash(jugador, delta);

    // 4. Calcular nueva posición
    Vector2 nuevaPos = jugador->posicion;
    nuevaPos.x += jugador->velocidad.x * delta;
    nuevaPos.y += jugador->velocidad.y * delta;

    // 5. Colisiones y entorno
    ManejarColisiones(jugador, nuevaPos, anchoMapa, altoMapa, VerificarColision);
    ManejarParedes(jugador, anchoMapa, altoMapa, VerificarColision);

    // 6. Actualizar sprite según estado
    if (!jugador->enSuelo) {
        jugador->spriteActual = jugador->spriteJump;
    } 
    else if (jugador->enParedIzquierda || jugador->enParedDerecha) {
        jugador->spriteActual = jugador->spriteClimb;
    }
    else if (fabs(jugador->velocidad.x) > 5.0f) { // Umbral más bajo
        jugador->spriteActual = jugador->spriteRun;
    } 
    else {
        jugador->spriteActual = jugador->spriteIdle;
    }
    ActualizarSpriteJugador(jugador);
}

void ActualizarSpriteJugador(Personaje* jugador) {
    // Prioridad 1: Dash
    if (jugador->estaDashing) {
        jugador->spriteActual = jugador->spriteRun; // O un spriteDash si lo tienes
    }
    // Prioridad 2: Escalando paredes
    else if ((jugador->enParedIzquierda || jugador->enParedDerecha) && !jugador->enSuelo) {
        jugador->spriteActual = jugador->spriteClimb;
    }
    // Prioridad 3: Saltando/cayendo
    else if (!jugador->enSuelo) {
        jugador->spriteActual = jugador->spriteJump;
    }
    // Prioridad 4: Corriendo
    else if (fabs(jugador->velocidad.x) > 5.0f) {
        jugador->spriteActual = jugador->spriteRun;
    }
    // Por defecto: Idle
    else {
        jugador->spriteActual = jugador->spriteIdle;
    }
    
    // Sincronizar posición
    jugador->spriteActual->position = jugador->posicion;
}