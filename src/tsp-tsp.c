#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "tsp-types.h"
#include "tsp-genmap.h"
#include "tsp-print.h"
#include "tsp-tsp.h"

void* ALL_IS_OK = (void*)123456789L;

pthread_mutex_t mutex;

/* dernier minimum trouvé */
int minimum;

extern Cell* tab_threads;
extern tsp_path_t sol; /* Correspond à la solution */
extern int sol_len; /* Longueur du chemin sol */
extern int nb_threads;
extern long long int cuts;

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

/**
 * [tsp  description]
 * @param hops    nombre de ville dans le chemin path
 * @param len     longueur du chemin path
 * @param path    chemin
 * @param cuts    ??
 * @param sol     Solution résultant 
 * @param sol_len [description]
 */
void* tsp (void* arguments)
{
    //printf("    ENTREE TSP avec %lx\n", pthread_self());

    
    //printf("Le thread %lx prend le mutex\n", pthread_self());

    if (tab_threads[getTID()].arguments.len + cutprefix[(nb_towns-tab_threads[getTID()].arguments.hops)] >= minimum) { /* Si sa longueur du chemin + .... */
        pthread_mutex_lock (&mutex); /* On verrouille le mutex */
        (cuts)++ ;
        //printf("EXIT TSP avec %lx\n", pthread_self());
        
        tab_threads[getTID()].occupe = 0;
        //printf("Le thread %lx rend le mutex\n", pthread_self());
        pthread_mutex_unlock (&mutex); /* On déverrouille le mutex */
        return ALL_IS_OK;
    }

    //printf("Le thread %lx rend le mutex\n", pthread_self());
    //pthread_mutex_unlock (&mutex); /* On déverrouille le mutex */
    if (tab_threads[getTID()].arguments.hops == nb_towns) { /* Si on a un chemin complet (ie qui contient toutes les villes) */
	    tab_threads[getTID()].arguments.me = tab_threads[getTID()].arguments.path [tab_threads[getTID()].arguments.hops - 1];
	    tab_threads[getTID()].arguments.dist = distance[tab_threads[getTID()].arguments.me][0]; // retourner en 0

        //pthread_mutex_lock (&mutex); /* On verrouille le mutex */
        //printf("Le thread %lx prend le mutex\n", pthread_self());
        
        pthread_mutex_lock (&mutex); /* On verrouille le mutex */
        if ( tab_threads[getTID()].arguments.len + tab_threads[getTID()].arguments.dist < minimum ) { /* Si ce chemin représente un plus court chemin temporaire alors on stocke sa 
                valeure dans solution et on l'écrit */

		    minimum = tab_threads[getTID()].arguments.len + tab_threads[getTID()].arguments.dist;
		    sol_len = tab_threads[getTID()].arguments.len + tab_threads[getTID()].arguments.dist;

		    memcpy(sol, tab_threads[getTID()].arguments.path, nb_towns*sizeof(int));

		    print_solution (tab_threads[getTID()].arguments.path, tab_threads[getTID()].arguments.len+tab_threads[getTID()].arguments.dist);
	    }
        pthread_mutex_unlock (&mutex); /* On déverrouille le mutex */

        //pthread_mutex_unlock (&mutex);
        //printf("Le thread %lx rend le mutex\n", pthread_self());
        
    }

    else { /* Si on n'a pas un chemin complet (ie qui ne contient pas toutes les villes) */
        tab_threads[getTID()].arguments.me = tab_threads[getTID()].arguments.path [tab_threads[getTID()].arguments.hops - 1]; /* On stocke la dernière ville du chemin dans la variable "me" */      

        for (tab_threads[getTID()].arguments.i = 0; tab_threads[getTID()].arguments.i < nb_towns; tab_threads[getTID()].arguments.i++) { /* On regarde toutes les villes */
      
            if (!present (tab_threads[getTID()].arguments.i, tab_threads[getTID()].arguments.hops, tab_threads[getTID()].arguments.path)) { /* On rajoute les villes qui ne sont pas présentes dans le chemin tab_threads[getTID()].arguments.path */
                tab_threads[getTID()].arguments.path[tab_threads[getTID()].arguments.hops] = tab_threads[getTID()].arguments.i;
                tab_threads[getTID()].arguments.dist = distance[tab_threads[getTID()].arguments.me][tab_threads[getTID()].arguments.i];

                /********************************************/

                tab_threads[getTID()].arguments.hops = tab_threads[getTID()].arguments.hops + 1;
                tab_threads[getTID()].arguments.len = tab_threads[getTID()].arguments.len + tab_threads[getTID()].arguments.dist;

                return tsp((void*)NULL);

                /********************************************/
            }
        }
    }

    //printf("    EXIT TSP avec %lx\n", pthread_self());
   
    /************************************/

    tab_threads[getTID()].occupe = 0;
    return ALL_IS_OK;
    
    /************************************/
}

int getTID (void){
    int i;
    for(i=0;i<nb_threads;i++){
        if(pthread_equal(pthread_self(),tab_threads[i].thread)){
        return i;
        }
    }
    return -1;
}

/****************************************************************************/