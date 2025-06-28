#include "movimiento.h"
#include "hashmap.h"
#include <math.h>

// Inicializa todos los atributos del personaje con valores predeterminados
void InicializarPersonaje(Personaje *jugador) {
    *jugador = (Personaje){
        .nombre = "",
        .vida = 100,
        .ataque = 10,
        .defensa = 5,
        .posicion = {0, 0},
        .velocidad = {0, 0},
        .offsetVisual = {0, -32},
        .inventario = createMap(100), // Mapa para guardar objetos
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

// Detecta entradas del jugador (espacio para salto, shift para dash)
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

// Aplica fuerzas al personaje: aceleración, fricción, gravedad
void AplicarFisicas(Personaje *jugador, float delta) {
    if (!jugador->estaDashing) {
        // Movimiento horizontal por teclas
        if (IsKeyDown(KEY_RIGHT)) {
            jugador->velocidad.x += ACELERACION * delta;
        } else if (IsKeyDown(KEY_LEFT)) {
            jugador->velocidad.x -= ACELERACION * delta;
        } else {
            // Aplicar fricción si no se mueve
            if (jugador->velocidad.x > 0)
                jugador->velocidad.x = fmaxf(jugador->velocidad.x - FRICCION * delta, 0);
            else if (jugador->velocidad.x < 0)
                jugador->velocidad.x = fminf(jugador->velocidad.x + FRICCION * delta, 0);
        }

        // Limitar velocidad en eje X
        jugador->velocidad.x = fminf(fmaxf(jugador->velocidad.x, -VELOCIDAD_MOVIMIENTO), VELOCIDAD_MOVIMIENTO);
    }

    // Aplicar gravedad si no está dashing
    if (!jugador->estaDashing) {
        jugador->velocidad.y += GRAVITY * delta;
        if (jugador->velocidad.y > VELOCIDAD_MAX_CAIDA)
            jugador->velocidad.y = VELOCIDAD_MAX_CAIDA;
    }
}

// Controla el tiempo del dash y su finalización
void ManejarDash(Personaje *jugador, float delta) {
    if (jugador->estaDashing) {
        jugador->contadorDash -= delta;
        if (jugador->contadorDash <= 0) {
            jugador->estaDashing = false;
        }
    }
}

// Maneja las colisiones del personaje con el mapa, tanto en X como en Y
void ManejarColisiones(Personaje *jugador, Vector2 nuevaPos, int anchoMapa, int altoMapa, bool (*VerificarColision)(int, int, int, int)) {
    // Calcular tiles con los que se podría chocar en X
    int col_izq_tile = (int)nuevaPos.x / TILE_SIZE;
    int col_der_tile = (int)(nuevaPos.x + TILE_SIZE - 1) / TILE_SIZE;
    int col_arriba_y_tile = (int)jugador->posicion.y / TILE_SIZE;
    int col_abajo_y_tile = (int)(jugador->posicion.y + TILE_SIZE - 1) / TILE_SIZE;

    // Colisión horizontal
    if (jugador->velocidad.x > 0) {
        if (VerificarColision(col_der_tile, col_arriba_y_tile, anchoMapa, altoMapa) ||
            VerificarColision(col_der_tile, col_abajo_y_tile, anchoMapa, altoMapa)) {
            nuevaPos.x = col_der_tile * TILE_SIZE - TILE_SIZE;
            jugador->velocidad.x = 0;
        }
    } else if (jugador->velocidad.x < 0) {
        if (VerificarColision(col_izq_tile, col_arriba_y_tile, anchoMapa, altoMapa) ||
            VerificarColision(col_izq_tile, col_abajo_y_tile, anchoMapa, altoMapa)) {
            nuevaPos.x = (col_izq_tile + 1) * TILE_SIZE;
            jugador->velocidad.x = 0;
        }
    }

    // Calcular tiles verticales
    int col_arriba_tile = (int)nuevaPos.y / TILE_SIZE;
    int col_abajo_tile = (int)(nuevaPos.y + TILE_SIZE + 2) / TILE_SIZE;
    int col_izq_x_tile = (int)nuevaPos.x / TILE_SIZE;
    int col_der_x_tile = (int)(nuevaPos.x + TILE_SIZE - 1) / TILE_SIZE;

    jugador->enSuelo = false;

    if (jugador->velocidad.y >= 0) { // Cayendo
        if (VerificarColision(col_izq_x_tile, col_abajo_tile, anchoMapa, altoMapa) ||
            VerificarColision(col_der_x_tile, col_abajo_tile, anchoMapa, altoMapa)) {
            nuevaPos.y = col_abajo_tile * TILE_SIZE - TILE_SIZE;
            jugador->velocidad.y = 0;
            jugador->enSuelo = true;
            jugador->puedeDobleSalto = true;
            jugador->puedeDash = true;
        }
    } else { // Subiendo
        if (VerificarColision(col_izq_x_tile, col_arriba_tile, anchoMapa, altoMapa) ||
            VerificarColision(col_der_x_tile, col_arriba_tile, anchoMapa, altoMapa)) {
            nuevaPos.y = (col_arriba_tile + 1) * TILE_SIZE;
            jugador->velocidad.y = 0;
        }
    }

    jugador->posicion = nuevaPos;
}

// Detecta si el jugador está en contacto con una pared (izquierda o derecha)
void ManejarParedes(Personaje *jugador, int anchoMapa, int altoMapa, bool (*VerificarColision)(int, int, int, int)) {
    jugador->enParedIzquierda = 
        VerificarColision((int)floorf((jugador->posicion.x - 1) / TILE_SIZE), (int)floorf(jugador->posicion.y / TILE_SIZE), anchoMapa, altoMapa) ||
        VerificarColision((int)floorf((jugador->posicion.x - 1) / TILE_SIZE), (int)floorf((jugador->posicion.y + TILE_SIZE - 1) / TILE_SIZE), anchoMapa, altoMapa);

    jugador->enParedDerecha = 
        VerificarColision((int)floorf((jugador->posicion.x + TILE_SIZE) / TILE_SIZE), (int)floorf(jugador->posicion.y / TILE_SIZE), anchoMapa, altoMapa) ||
        VerificarColision((int)floorf((jugador->posicion.x + TILE_SIZE) / TILE_SIZE), (int)floorf((jugador->posicion.y + TILE_SIZE - 1) / TILE_SIZE), anchoMapa, altoMapa);
}

// Controla todos los tipos de salto: normal, doble y en pared
void ManejarSaltos(Personaje *jugador, float delta) {
    // Coyote time
    jugador->contadorCoyote = jugador->enSuelo ? jugador->tiempoCoyote : jugador->contadorCoyote - delta;

    // Buffer de salto
    jugador->contadorBufferSalto -= delta;

    if (jugador->contadorBufferSalto > 0.0f &&
        (jugador->contadorCoyote > 0.0f || jugador->puedeDobleSalto || jugador->enParedIzquierda || jugador->enParedDerecha)) {
        
        if (jugador->enParedIzquierda || jugador->enParedDerecha) {
            jugador->velocidad.y = JUMP_FORCE;
            jugador->velocidad.x = jugador->enParedIzquierda ? VELOCIDAD_MOVIMIENTO : -VELOCIDAD_MOVIMIENTO;
            jugador->contadorSaltoPared = TIEMPO_SALTO_PARED;
        } else {
            jugador->velocidad.y = JUMP_FORCE;
            if (!jugador->enSuelo && jugador->puedeDobleSalto)
                jugador->puedeDobleSalto = false;
        }

        jugador->enSuelo = false;
        jugador->contadorCoyote = 0.0f;
        jugador->contadorBufferSalto = 0.0f;
    }

    // Cancelar salto si se suelta la tecla antes de tiempo
    if (IsKeyReleased(KEY_SPACE) && jugador->velocidad.y < 0)
        jugador->velocidad.y *= 0.5f;

    // Salto en pared con control de tiempo
    if (jugador->contadorSaltoPared > 0) {
        jugador->contadorSaltoPared -= delta;
        if ((jugador->velocidad.x > 0 && IsKeyDown(KEY_LEFT)) ||
            (jugador->velocidad.x < 0 && IsKeyDown(KEY_RIGHT))) {
            jugador->velocidad.x = 0;
        }
    }
}

// Ejecuta todo el sistema de movimiento del jugador
void ActualizarMovimiento(Personaje *jugador, int anchoMapa, int altoMapa, float delta, bool (*VerificarColision)(int, int, int, int)) {
    ProcesarEntradas(jugador);
    ManejarSaltos(jugador, delta);
    AplicarFisicas(jugador, delta);
    ManejarDash(jugador, delta);

    Vector2 nuevaPos = jugador->posicion;
    nuevaPos.x += jugador->velocidad.x * delta;
    nuevaPos.y += jugador->velocidad.y * delta;

    ManejarColisiones(jugador, nuevaPos, anchoMapa, altoMapa, VerificarColision);
    ManejarParedes(jugador, anchoMapa, altoMapa, VerificarColision);
    ActualizarSpriteJugador(jugador);
}

// Asigna el sprite correspondiente al estado actual del jugador
void ActualizarSpriteJugador(Personaje* jugador) {
    jugador->offsetVisual = (Vector2){0, -32};

    bool moviendoseDerecha = jugador->velocidad.x > 0.1f;
    bool moviendoseIzquierda = jugador->velocidad.x < -0.1f;

    if (jugador->estaDashing) {
        jugador->spriteActual = jugador->spriteRun;
        jugador->spriteActual->flipX = !moviendoseDerecha;
    } else if ((jugador->enParedIzquierda || jugador->enParedDerecha) && !jugador->enSuelo) {
        jugador->spriteActual = jugador->spriteClimb;
        jugador->spriteActual->flipX = jugador->enParedIzquierda;
    } else if (!jugador->enSuelo) {
        jugador->spriteActual = (jugador->velocidad.y < -0.1f) ? jugador->spriteJumpUp : jugador->spriteJumpDown;
        jugador->spriteActual->flipX = !moviendoseDerecha;
    } else if (fabs(jugador->velocidad.x) > 5.0f) {
        static bool primerPaso = true;
        if (primerPaso) {
            jugador->spriteActual = jugador->spriteIdleStart;
            if (jugador->spriteActual->currentFrame >= jugador->spriteActual->frameCount - 1)
                primerPaso = false;
        } else {
            jugador->spriteActual = jugador->spriteIdleWalk;
        }
        jugador->spriteActual->flipX = !moviendoseDerecha;
    } else {
        jugador->spriteActual = jugador->spriteIdleStart;
        jugador->spriteActual->flipX = !moviendoseDerecha;
    }

    jugador->spriteActual->position = (Vector2){
        jugador->posicion.x + jugador->offsetVisual.x,
        jugador->posicion.y + jugador->offsetVisual.y
    };
}
