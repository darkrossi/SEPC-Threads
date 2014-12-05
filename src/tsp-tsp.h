#ifndef TSP_TSP_H
#define TSP_TSP_H

#include <pthread.h>

/* dernier minimum trouvé */
extern int minimum;

/********************************************/

struct arg_struct {
	int hops; /* Nb de ville dans le chemin path */
	int len; /* Longueur du chemin path */
	tsp_path_t path; /* Chemin */

	long long int *cuts; 

	tsp_path_t sol; /* Chemin solution résultant de tsp */
	int *sol_len; /* Longueur de la solution */

	int N_thread;
};

/*********************************************/

int present (int city, int hops, tsp_path_t path);
void* tsp (void* arguments);

/*********************************************/

typedef struct Cell{
	pthread_t thread;
	int occupe;
} Cell;

/********************************************/

#endif
