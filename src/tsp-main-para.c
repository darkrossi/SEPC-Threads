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

#include "tsp-types.h"
#include "tsp-job.h"
#include "tsp-genmap.h"
#include "tsp-print.h"
#include "tsp-tsp.h"



/*****************************/

#include <pthread.h>

extern void* ALL_IS_OK;
extern void* tsp (void* arguments);
extern pthread_mutex_t mutex;

Cell* tab_threads;

tsp_path_t sol; /* Correspond à la solution */
int sol_len; /* Longueur du chemin sol */
long long int cuts;

/*****************************/



/* macro de mesure de temps, retourne une valeur en nanosecondes */
#define TIME_DIFF(t1, t2) \
  ((t2.tv_sec - t1.tv_sec) * 1000000000ll + (long long int) (t2.tv_nsec - t1.tv_nsec))


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

/**
 * [generate_tsp_jobs  description]
 * @param q       Queue de jobs
 * @param hops    Nombre de villes dans path
 * @param len     Longueur de path
 * @param path    Chemin
 * @param cuts    ??
 * @param sol     Solution
 * @param sol_len Longueur de solution
 * @param depth   Profondeur
 */
/* Initialement : &q, 1, 0, path, &cuts, sol, & sol_len, 3 */ 
static void generate_tsp_jobs (struct tsp_queue *q, int hops, int len, tsp_path_t path, long long int *cuts, tsp_path_t sol, int *sol_len, int depth)
{
    if (len >= minimum) { /* Si la longueur du chemin est plus grande que le minimum global */
        (*cuts)++ ; /* On incrémente cuts */
        return;
    }
    
    if (hops == depth) { /* Si le nombre de ville dans path est égal à la profondeur depth */

        /* On enregistre du travail à faire plus tard... */
        add_job (q, path, hops, len);
    }

    else {/* Si le nombre de ville dans path est différent à la profondeur depth */
        int me = path [hops - 1]; /*On stocke la dernière ville du chemin dans me */

        for (int i = 0; i < nb_towns; i++) { /* On parcourt toutes les villes */
            if (!present (i, hops, path)) { /* Si la ville i n'est pas présente dans path */
                path[hops] = i; /* On rajoute la ville i à path*/
                int dist = distance[me][i]; /* On note la distance entre l'ex dernière ville de path avec la new dernière ville
                    de path */
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

    tsp_path_t path; /* Correspond au chemin parcouru */

    cuts = 0;
    struct tsp_queue q;
    struct timespec t1, t2;

    /***********************/
    /***********************/


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
    /**/
   
    minimum = INT_MAX; /* On initialise la variable global minimum */
      
    /* generer la carte et la matrice de distance */
    fprintf (stderr, "ncities = %3d\n", nb_towns);
    genmap ();
    /**/

    init_queue (&q); /* Initialise la liste des jobs à faire */

    clock_gettime (CLOCK_REALTIME, &t1); /* Déclenche le chrono */

    memset (path, -1, MAX_TOWNS * sizeof (int)); /* Met que des -1 dans le tableau d'entiers path */
    path[0] = 0;

    generate_tsp_jobs (&q, 1, 0, path, &cuts, sol, & sol_len, 3); /* Génére toutes les tâches à effectuer */
    no_more_jobs (&q); /* Dit qu'il n'y aura plus de jobs à ajouter à la liste */
   
    tsp_path_t solution;
    memset (solution, -1, MAX_TOWNS * sizeof (int)); /* Met que des -1 dans le tableau d'entiers solution */



     /**********   ADD   **************/
    int i, j = 0, k = 0;


    tab_threads = malloc(nb_threads*sizeof(Cell));
    memset(tab_threads, 0, nb_threads*sizeof(int));

     /******************************/

    pthread_mutex_init(&mutex, NULL);

    while (!empty_queue (&q)) {
        j++;
        //printf("ENTREE WHILE1 MAIN avec %lx\n", pthread_self());

        int hops = 0; /* Nb de villes parcourus par un chemin donné (hop = saut..) */
        int len = 0; /* Longueur d'un chemin donné */

        get_job (&q, solution, &hops, &len); /* Sort un job de la queue et le met dans solution */
        

        /****************************************************/

        //printf("ENTREE SEARCH THREAD DISPO MAIN avec %lx\n", pthread_self());
        pthread_mutex_lock (&mutex); /* On verrouille le mutex */
        for(i=0; i<nb_threads; i++){

            if(!tab_threads[i].occupe){

                tab_threads[i].arguments.hops = hops;
                tab_threads[i].arguments.len = len;
                memcpy(tab_threads[i].arguments.path, solution, hops * sizeof (int));
               
                tab_threads[i].occupe = 1;

                pthread_create (&tab_threads[i].thread, NULL, tsp, (void*)NULL);
                k++;
                break;
            }
        }
        pthread_mutex_unlock (&mutex);

        if (i == nb_threads){
            
            add_job (&q, solution, hops, len) ;
        }

       // printf("EXIT SEARCH THREAD DISPO MAIN avec %lx\n", pthread_self());

        //if (j%20 == 0){
            /** Parcours de la liste des threads **/
            int temp_parcours = 0;
            printf("\n    # Parcours de la liste #\n");
            for(i=0; i<nb_threads; i++) {
                if(tab_threads[i].occupe) temp_parcours += 1;
            }
            printf("    Actuellement, il y a %d threads dans la liste\n\n", temp_parcours);
            printf("%d\n",j);
            /*******************************/
        //}

        //printf("EXIT WHILE MAIN avec %lx\n", pthread_self());

    }
    printf("%d et %d\n",j, k);
    i=0;
    while(1){
        pthread_mutex_lock (&mutex); /* On verrouille le mutex */
        if (!tab_threads[i].occupe) i++;
        pthread_mutex_unlock (&mutex); /* On verrouille le mutex */
        if (i>=nb_threads) break;
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
