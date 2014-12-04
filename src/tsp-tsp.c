#include <assert.h>
#include <string.h>

#include <stdlib.h>

#include "tsp-types.h"
#include "tsp-genmap.h"
#include "tsp-print.h"
#include "tsp-tsp.h"

void* ALL_IS_OK = (void*)123456789L;

/* dernier minimum trouvé */
int minimum;

/* résolution du problème du voyageur de commerce */
int present (int city, int hops, tsp_path_t path)
{
    for (int i = 0; i < hops; i++) {
        if (path [i] == city) {
            return 1;
        }
    }
    
    return 0 ;
}



// void tsp (int hops, int len, tsp_path_t path, long long int *cuts, tsp_path_t sol, int *sol_len)
void* tsp (void* arguments)
{
    struct arg_struct *args = (struct arg_struct*) arguments;

    int hops = args->hops;
    int len = args->len;
    tsp_path_t path;
    memcpy(path, args->path, MAX_TOWNS * sizeof (int));
    long long int *cuts = args->cuts;
    tsp_path_t sol;
    memcpy(sol, args->sol, MAX_TOWNS * sizeof (int));
    int *sol_len = args->sol_len;

  if (len + cutprefix[(nb_towns-hops)] >= minimum) {
      (*cuts)++ ;
      return ALL_IS_OK;
    }
    
    if (hops == nb_towns) {
	    int me = path [hops - 1];
	    int dist = distance[me][0]; // retourner en 0
            if ( len + dist < minimum ) {
		    minimum = len + dist;
		    *sol_len = len + dist;
		    memcpy(sol, path, nb_towns*sizeof(int));
		    print_solution (path, len+dist);
	    }
    } else {
        int me = path [hops - 1];        
        for (int i = 0; i < nb_towns; i++) {
            if (!present (i, hops, path)) {
                path[hops] = i;
                int dist = distance[me][i];

                args->hops = hops + 1;
                args->len = len + dist;
                memcpy(args->path, path, MAX_TOWNS * sizeof (int));
                memcpy(args->sol, sol, MAX_TOWNS * sizeof (int));
                tsp ((void*)args);
            }
        }
    }
    pthread_exit(NULL);
    return ALL_IS_OK;
}



Liste Creer_Liste(void){
    return NULL;
}

Liste Ajouter(Liste l, int Taille_Max){
    Cell* cellule = malloc(sizeof(Cell));
    Liste l_temp = l;
    int i = 1;

    if (l == NULL){
        cellule->suiv = NULL;
        return cellule;
    }

    while (l_temp->suiv != NULL && i < Taille_Max){
        l_temp = l_temp->suiv;
        i++;
    }
    if (i < Taille_Max){
        cellule->suiv = NULL;
        l_temp->suiv = cellule;
        return l;
    }
    else return NULL;
    
}

Liste Supprimer(Liste l, int indice){
    Liste temp = l;
    if (indice == 0){
        free(l);
        return temp->suiv;
    }

    l->suiv = Supprimer(l->suiv, indice -1);
    return l;
}

pthread_t Dernier(Liste l){ /* Quand on appelle cette fonction, la liste n'est pas vide */
    while (l->suiv != NULL){
        l = l->suiv;
    }
    return l->thread;
}

