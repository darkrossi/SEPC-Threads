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
};

/*********************************************/

int present (int city, int hops, tsp_path_t path);
void* tsp (void* arguments);

/*********************************************/

typedef struct Cell{
	pthread_t thread;
	int occupe;
	struct arg_struct arguments;
} Cell;

/********************************************/

int getTID (void);

#endif
