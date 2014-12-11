#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <time.h>
#include <assert.h>
#include <complex.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

#include "tsp-types.h"
#include "tsp-job.h"
#include "tsp-genmap.h"
#include "tsp-print.h"
#include "tsp-tsp.h"

/***************************************************************
 * Ce programme est multi-threads, fonctionne, mais est plus   *
 * lent que la version initiale.                               *
 * Principe du programme :                                     *
 * On affecte un 'job' de la file d'attente à un thread        *
 * disponible. Lorsqu'on voit qu'un thread est occupé, on      *
 * teste le thread suivant et ainsi de suite.                  *
 * Une fois la file vidée on attend la fin de tous les threads *
 * lancés avant d'afficher le résultat.                        *
 ***************************************************************/


/* macro de mesure de temps, retourne une valeur en nanosecondes */
#define TIME_DIFF(t1, t2) \
  ((t2.tv_sec - t1.tv_sec) * 1000000000ll + (long long int) (t2.tv_nsec - t1.tv_nsec))

/* valeur de renvoi de la fonction tsp */
extern void* ALL_IS_OK;

/* fonction de recherche du meilleur chemin */
extern void* tsp (void* arguments);

/* mutex */
extern pthread_mutex_t mutex;

/* tableau contenant des informations sur les threads utilisés */
Cell* tab_threads;

/* tableau des distances */
tsp_distance_matrix_t distance ={};

/** Paramètres **/

/* nombre de villes */
int nb_towns=10;
/* graine */
long int myseed= 0;
/* nombre de threads */
int nb_threads=1;

/* affichage SVG */
bool affiche_sol= false;

/* solution actuelle*/
tsp_path_t sol;

/* longueur de la solution actuelle*/
int sol_len;

/* nombre total de 'abort' lors du parcours des chemins */
long long int cuts = 0;


static void generate_tsp_jobs (struct tsp_queue *q, int hops, int len, tsp_path_t path, long long int *cuts, tsp_path_t sol, int *sol_len, int depth)
{
    if (len >= minimum) {
        (*cuts)++ ;
        return;
    }
    
    if (hops == depth) {
        /* On enregistre du travail à faire plus tard... */
        add_job (q, path, hops, len);
    } else {
        int me = path [hops - 1];        
        for (int i = 0; i < nb_towns; i++) {
            if (!present (i, hops, path)) {
                path[hops] = i;
                int dist = distance[me][i];
                generate_tsp_jobs (q, hops + 1, len + dist, path, cuts, sol, sol_len, depth);
            }
        }
    }
}

static void usage(const char *name) {
  fprintf (stderr, "Usage: %s [-s] <ncities> <seed> <nthreads>\n", name);
  exit (-1);
}

int main (int argc, char **argv)
{
    unsigned long long perf;
    tsp_path_t path;
    struct tsp_queue q;
    struct timespec t1, t2;
	

    /* lire les arguments */
    int opt;
    while ((opt = getopt(argc, argv, "s")) != -1) {
      switch (opt) {
      case 's':
	affiche_sol = true;
	break;
      default:
	usage(argv[0]);
	break;
      }
    }

    if (optind != argc-3)
      usage(argv[0]);

    nb_towns = atoi(argv[optind]);
    myseed = atol(argv[optind+1]);
    nb_threads = atoi(argv[optind+2]);
    assert(nb_towns > 0);
    assert(nb_threads > 0);
   
    minimum = INT_MAX;
      
    /* generer la carte et la matrice de distance */
    fprintf (stderr, "ncities = %3d\n", nb_towns);
    genmap ();

    init_queue (&q);

    clock_gettime (CLOCK_REALTIME, &t1);
	
	/* on vide le chemin 'path' : on met des -1 dans toutes les cases */
    memset (path, -1, MAX_TOWNS * sizeof (int));
	/* on définit la première ville comme étant 0 */
    path[0] = 0;

    /* On met des travaux dans la file d'attente q */
    generate_tsp_jobs (&q, 1, 0, path, &cuts, sol, & sol_len, 3);
    no_more_jobs (&q);
   
	/* on vide le chemin 'solution' : on met des -1 dans toutes les cases */
	/* puis on définit la première ville comme étant 0 */
    tsp_path_t solution;
    memset (solution, -1, MAX_TOWNS * sizeof (int));
    solution[0] = 0;

	/* allocation du tableau d'informations sur les threads */
    tab_threads = malloc(nb_threads*sizeof(Cell));
	memset(tab_threads, 0, nb_threads*sizeof(int));

	/* initialisation du mutex*/
	pthread_mutex_init(&mutex, NULL);

	int i;

	/* on place tous les threads dans l'état 'non-occupé' */
	for(i=0; i<nb_threads; i++){
		tab_threads[i].occupe = 0;
    }

	/* Tant que la file de jobs n'est pas vide on sort un job et on l'execute avec tsp */
    while (!empty_queue (&q)) {
        int hops = 0, len = 0;
		/* on récupère un job dans la file d'atente des jobs */
        get_job (&q, solution, &hops, &len);
        
        i = 0;
		/* on boucle tant qu'on n'a pas trouvé de thread pour effectuer le job */
        while (1){
			/* si le thread i n'est pas occupé alors on va l'utiliser pour effectuer le job */
            if(!tab_threads[i].occupe){
                
				/* on place le thread i en statut 'occupé' */
                tab_threads[i].occupe = 1;

				/* on crée une structure arguments qui va recevoir les arguments de tsp */
                struct arg_struct arguments;

				arguments.hops = hops;
				arguments.len = len;
				memcpy(arguments.path, solution, nb_towns * sizeof (int));

				/* tsp est récursive et on a besoin de savoir quand le thread a terminé le job afin de le mettre en 'non-occupé' */
				/* On introduit donc un entier qui est 1 si le tsp appelé est celui du père et 0 si c'est dans les fils */
                arguments.pere = 1; 

				/* lancement du thread sur tsp avec les bons arguments*/
                pthread_create (&tab_threads[i].thread, NULL, tsp, (void*)&arguments);
                break;
            }
			/* sinon on va regarder si le suivant%nb_threads est occupé */
            i = (i+1)%nb_threads;
        }
    }

    i = 0;
	/* on attend la fin de tous les threads avant de continuer */
    while(1){
        pthread_join(tab_threads[i].thread, NULL);
        i++;
        if (i == nb_threads) break;
    }
    
    clock_gettime (CLOCK_REALTIME, &t2);

    if (affiche_sol)
      print_solution_svg (sol, sol_len);

    perf = TIME_DIFF (t1,t2);
    printf("<!-- # = %d seed = %ld len = %d threads = %d time = %lld.%03lld ms ( %lld coupures ) -->\n",
	   nb_towns, myseed, sol_len, nb_threads,
	   perf/1000000ll, perf%1000000ll, cuts);

    return 0 ;
}
