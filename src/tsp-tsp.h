#ifndef TSP_TSP_H
#define TSP_TSP_H

#include <pthread.h>

/* dernier minimum trouvé */
extern int minimum;

struct arg_struct {
	int hops;
	int len;
	tsp_path_t path; 
	long long int *cuts; 
	tsp_path_t sol; 
	int *sol_len;
};

int present (int city, int hops, tsp_path_t path);
void* tsp (void* arguments);


typedef struct Cell{
	pthread_t thread;
	struct Cell* suiv;
} Cell;
typedef Cell* Liste;

Liste Creer_Liste(void);

Liste Ajouter(Liste l, int Taille_Max);

Liste Supprimer(Liste l, int indice);

pthread_t Dernier(Liste l);


#endif
