#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "tsp-types.h"
#include "tsp-genmap.h"
#include "tsp-print.h"
#include "tsp-tsp.h"

void* ALL_IS_OK = (void*)123456789L;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; /* Création du mutex */

/* dernier minimum trouvé */
int minimum;

extern Cell* tab_threads;

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

    /*********** On ressort les variables de la arg_struct ****************************************/

    struct arg_struct *args = (struct arg_struct*) arguments;

    int hops = args->hops;
    int len = args->len;
    tsp_path_t path;
    memcpy(path, args->path, MAX_TOWNS * sizeof (int));
    long long int *cuts = args->cuts;
    tsp_path_t sol;
    memcpy(sol, args->sol, MAX_TOWNS * sizeof (int));
    int *sol_len = args->sol_len;

    int N_thread = args->N_thread;

    /**********************************************************************************************/

    pthread_mutex_lock (&mutex); /* On verrouille le mutex */
    if (len + cutprefix[(nb_towns-hops)] >= minimum) { /* Si sa longueur du chemin + .... */
        (*cuts)++ ;
        //printf("EXIT TSP avec %lx\n", pthread_self());
        
        tab_threads[N_thread].occupe = 0;
        pthread_mutex_unlock (&mutex); /* On déverrouille le mutex */
        return (void*)NULL;
    }

    if (hops == nb_towns) { /* Si on a un chemin complet (ie qui contient toutes les villes) */
	    int me = path [hops - 1];
	    int dist = distance[me][0]; // retourner en 0

        //pthread_mutex_lock (&mutex); /* On verrouille le mutex */
        if ( len + dist < minimum ) { /* Si ce chemin représente un plus court chemin temporaire alors on stocke sa 
                valeure dans solution et on l'écrit */
		    minimum = len + dist;
		    *sol_len = len + dist;
		    memcpy(sol, path, nb_towns*sizeof(int));
            //pthread_mutex_unlock (&mutex); /* On déverrouille le mutex */

		    print_solution (path, len+dist);
	    }

    }

    else { /* Si on n'a pas un chemin complet (ie qui ne contient pas toutes les villes) */
        int me = path [hops - 1]; /* On stocke la dernière ville du chemin dans la variable "me" */      

        //pthread_mutex_lock (&mutex); /* On verrouille le mutex */
        for (int i = 0; i < nb_towns; i++) { /* On regarde toutes les villes */
      
            if (!present (i, hops, path)) { /* On rajoute les villes qui ne sont pas présentes dans le chemin path */
                path[hops] = i;
                int dist = distance[me][i];

                /********************************************/

                args->hops = hops + 1;
                args->len = len + dist;

                memcpy(args->path, path, (hops+1) * sizeof (int));
                memcpy(args->sol, sol, MAX_TOWNS * sizeof (int));
                pthread_mutex_unlock (&mutex); /* On déverrouille le mutex */

                return tsp ((void*)args);

                /********************************************/
            }
        }
    }

    //printf("    EXIT TSP avec %lx\n", pthread_self());
   
    /************************************/
    tab_threads[N_thread].occupe = 0;
    pthread_mutex_unlock (&mutex); /* On déverrouille le mutex */
    return (void*)NULL;;
    
    /************************************/
}

/****************************************************************************/