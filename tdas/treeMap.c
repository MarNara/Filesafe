#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "treemap.h"

typedef struct TreeNode TreeNode;


struct TreeNode {
    Pair* pair;
    TreeNode * left;
    TreeNode * right;
    TreeNode * parent;
};

struct TreeMap {
    TreeNode * root;
    TreeNode * current;
    int (*lower_than) (void* key1, void* key2);
};

int is_equal(TreeMap* tree, void* key1, void* key2){
    if(tree->lower_than(key1,key2)==0 &&  
        tree->lower_than(key2,key1)==0) return 1;
    else return 0;
}


TreeNode * createTreeNode(void* key, void * value) {
    TreeNode * new = (TreeNode *)malloc(sizeof(TreeNode));
    if (new == NULL) return NULL;
    new->pair = (Pair *)malloc(sizeof(Pair));
    new->pair->key = key;
    new->pair->value = value;
    new->parent = new->left = new->right = NULL;
    return new;
}

TreeMap * createTreeMap(int (*lower_than) (void* key1, void* key2)) {
    TreeMap * map = (TreeMap *) malloc(sizeof(TreeMap));
    if(map == NULL) return NULL;

    map->root = NULL;
    map->current = NULL;
    map->lower_than = lower_than;
    return map;
}


void insertTreeMap(TreeMap * tree, void* key, void * value) {
    TreeNode * actual = tree->root;
    TreeNode * padre = NULL;
    while (actual != NULL)
    {
        padre = actual;
        if (is_equal(tree,actual->pair->key,key))
        {
            return;
        }
        else if ((tree->lower_than(key,actual->pair->key)))
        {
            actual = actual->left;
        }
        else
        {
            actual = actual->right;
        }
        
    }
    
    TreeNode * nodoNuevo = createTreeNode(key,value);
    
    nodoNuevo->parent = padre;

    if (padre == NULL)
    {
        tree->root = nodoNuevo;
    }
    else if (tree->lower_than(key,padre->pair->key))
    {
        padre->left = nodoNuevo;
    }
    else
    {
        padre->right = nodoNuevo;    
    }
    tree->current = nodoNuevo;
}

TreeNode * minimum(TreeNode * x){
    if (x == NULL) return NULL;
    while (x->left != NULL)
    {
        x = x->left;
    }
    
    return x;
}


void removeNode(TreeMap * tree, TreeNode* node) {
    if(node == NULL){
        return;
    }
    //nodo sin hijos, primero debo preguntar si no estan y que hacer en cada caso.
    if(node->left == NULL && node->right == NULL){
        if(node->parent == NULL){
            tree->root = NULL;//como tampoco hay raiz transformo el nodo en raiz.
        }
        else if(node->parent->left == node){
            node->parent->left = NULL;//para desconectar el nodo
        }
        else{
            node->parent->right = NULL;
        }
        

        free(node);
    }
    else if(node->left == NULL || node->right == NULL){
        //necesito el nodo hijo, ya que tiene uno, puede ser izquierdo o derecho
        TreeNode* nino;
        if(node->parent->left != NULL){
            nino = node->left;
        }
        else{
            nino = node->right;
        }
        //evaluar

        if(node->parent == NULL){
            tree->root = nino;
        }
        else if(node->parent->left == node){
            node->parent->left = nino;
        }
        else{
            node->parent->right = nino;
        }

        //falta actualizar el padre
        
        nino->parent = node->parent;
        
        free(node);
        return;
    }
    else{
        /*como debo usar la funcion minimum es decir el menor de los mayores sera reemplazado como el el proximo nodo
        que estaba en esa posicion cambiando las claves por las del dato que acabo de elegir para que lo reemplace */
        TreeNode* nodoMenor = minimum(node->right);
        node->pair->key = nodoMenor->pair->key;
        node->pair->value = nodoMenor->pair->value;
        //eliminar el nodo como en la tarea en casa
        removeNode(tree, nodoMenor);

    }
    

}


void eraseTreeMap(TreeMap * tree, void* key){
    if (tree == NULL || tree->root == NULL) return;

    if (searchTreeMap(tree, key) == NULL) return;
    TreeNode* node = tree->current;
    removeNode(tree, node);

}




Pair * searchTreeMap(TreeMap * tree, void* key) {
    TreeNode *actual = tree->root;

    while (actual != NULL)
    {
        if (is_equal(tree,key,actual->pair->key))
        {
            tree->current = actual;
            return actual->pair;
        }
        else if (tree->lower_than(key,actual->pair->key))
        {
            actual = actual->left;
        }
        
        else
        {
            actual = actual->right;
        }

    }
    

    return NULL;
}


Pair * upperBound(TreeMap * tree, void* key) {
    return NULL;
}

Pair * firstTreeMap(TreeMap * tree) {
    if (tree == NULL || tree->root == NULL) return NULL;
    TreeNode *aux = tree->root;
    while (aux->left  != NULL)
    {
        aux = aux->left ;
    }
    tree->current = aux;
    return aux->pair ;
}

Pair * nextTreeMap(TreeMap * tree) {
    if (tree == NULL) return NULL;

    TreeNode *actual = tree->current;

    if (actual->right != NULL)
    {
        actual = actual->right;
        while (actual->left != NULL)
        {
            actual = actual->left;
        }
        tree->current = actual;
        return actual->pair;
    }
    TreeNode *padre = actual->parent;

    while (padre != NULL && actual == padre->right)
    {   
        actual = padre;
        padre = padre->parent;
    }
    
    tree->current = padre;
    if (padre == NULL) return NULL;
    return padre->pair;
}