#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "hashmap.h"


typedef struct HashMap HashMap;
int enlarge_called=0;

struct HashMap {
    Pair ** buckets;
    long size; //cantidad de datos/pairs en la tabla
    long capacity; //capacidad de la tabla
    long current; //indice del ultimo dato accedido
};

Pair * createPair( char * key,  void * value) {
    Pair * new = (Pair *)malloc(sizeof(Pair));
    new->key = key;
    new->value = value;
    return new;
}

long hash( char * key, long capacity) {
    unsigned long hash = 0;
     char * ptr;
    for (ptr = key; *ptr != '\0'; ptr++) {
        hash += hash*32 + tolower(*ptr);
    }
    return hash%capacity;
}

int is_equal(void* key1, void* key2){
    if(key1==NULL || key2==NULL) return 0;
    if(strcmp((char*)key1,(char*)key2) == 0) return 1;
    return 0;
}


void insertMap(HashMap * map, char * key, void * value) {
    //buckets es un arreglo, y current es una posicion
    
    long pos1 = hash(key, map->capacity); 
    /*1)preguntar por la capacidad del arreglo para ver si luego lo aumentamos en uno.
    2) entrar en un map con el while
    */
    
    Pair *actualPos = map->buckets[pos1];

    while( actualPos != NULL ){
        pos1 = (pos1 + 1) % map->capacity;
        actualPos = map->buckets[pos1];
    }
    
    if(map->buckets[pos1] == NULL){
        map->buckets[pos1] = createPair(strdup(key), value);
    }
    else{
        map->buckets[pos1]->key = strdup(key);
        map->buckets[pos1]->value = value;
    }
    map->size++;
    

}

void enlarge(HashMap * map) {
    enlarge_called = 1; //no borrar (testing purposes)
    long old_capacity = map->capacity;
    Pair **old_buckets = map->buckets;

    map->capacity *= 2;
    map->buckets = calloc(map->capacity, sizeof(Pair *));
    map->size = 0;

    for (long i = 0; i < old_capacity; i++) {
        if (old_buckets[i] != NULL && old_buckets[i]->key != NULL) {
            insertMap(map, old_buckets[i]->key, old_buckets[i]->value);
            //free(old_buckets[i]);
        }
    }
    free(old_buckets);

}


HashMap * createMap(long capacity) {
    /*1.- Implemente la función *createMap* en el archivo hashmap.c. Esta función crea una variable de tipo HashMap,
     inicializa el arreglo de buckets con casillas nulas, inicializa el resto de variables y retorna el mapa. 
     Inicialice el índice current a -1.*/
    HashMap *map = malloc(sizeof(HashMap));
    if(map == NULL) exit(EXIT_FAILURE);

     
    map->buckets = (Pair**)calloc(capacity, sizeof(Pair*));
    if(map->buckets == NULL){
        free(map);
        return NULL;
    }
    
    /*
    for( long k = 0; k < capacity; k++){
        map->buckets[k] = NULL;
    }*/
    map->size = 0;
    map->capacity = capacity;
    map->current = -1;

    
    return map;
}

void eraseMap(HashMap * map,  char * key) {    
    long pos1 = hash(key, map->capacity);
    long originalPos = pos1;
    while( map->buckets[pos1] != NULL){
        if(map->buckets[pos1]->key != NULL && is_equal(map->buckets[pos1]->key, key)){
            free(map->buckets[pos1]);
            map->buckets[pos1]->key = NULL;
            map->size--;
            return ;
        }
        pos1 = (pos1 + 1) % map->capacity;
        if(pos1 == originalPos) break;
        
    }
}

Pair * searchMap(HashMap * map,  char * key) {   
    long pos1 = hash(key, map->capacity); 
    /*1)preguntar por la capacidad del arreglo para ver si luego lo aumentamos en uno.
    2) entrar en un map con el while
    */
    
    //recorrer el mapa
    while( map->buckets[pos1] != NULL && map->buckets[pos1]->key != NULL ){
        if(is_equal(map->buckets[pos1]->key, key)){
            map->current = pos1;
            return map->buckets[pos1];
        }
        pos1 = (pos1 + 1) % map->capacity;
        
    }
    
    return NULL;
}

Pair * firstMap(HashMap * map) {
    
    for(long k = 0; k < map->capacity; k++){
        if(map->buckets[k] != NULL && map->buckets[k]->key){
            map->current = k;
            return map->buckets[k];
        }
    }

    return NULL;
}

Pair * nextMap(HashMap * map) {
    for(long k = map->current + 1; k < map->capacity; k++){
        if(map->buckets[k] != NULL && map->buckets[k]->key){
            map->current = k;
            return map->buckets[k];
        }
    }
    return NULL;
}