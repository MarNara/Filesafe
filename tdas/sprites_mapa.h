#ifndef SPRITES_MAPA_H
#define SPRITES_MAPA_H

void ProcesarMapaParaSprites(int** matrizMapa, int filas, int columnas);
void LimpiarSpritesActivos();
void EliminarSpritePorPosicion(List* lista, Vector2 pos);

#endif