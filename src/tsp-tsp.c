#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "tsp-types.h"
#include "tsp-genmap.h"
#include "tsp-print.h"
#include "tsp-tsp.h"

/* dernier minimum trouvé */
int minimum;
void* ALL_IS_OK = (void*)123456789L;
pthread_mutex_t mutex;

extern tsp_path_t sol;
extern int sol_len;
extern long long int cuts;
extern Cell* tab_threads;
extern int *cutprefix;

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

int getTID (void)
{
	int i;
	for(i=0;i<20;i++){ // remplacer 20 par nb_threads !
		if(pthread_equal(pthread_self(),tab_threads[i].thread)){
			return i;
		}
	}
	return -1;
}

void* tsp (void* arguments){   
	tsp_path_t path;
	int len = ((struct arg_struct*)arguments)->len;
	int hops = ((struct arg_struct*)arguments)->hops;
	memcpy(path,((struct arg_struct*)arguments)->path, nb_towns * sizeof (int));

	int pere = ((struct arg_struct*)arguments)->pere;
	
	//printf("	Je bloque dans %lx\n", pthread_self());

	pthread_mutex_lock(&mutex);
	if (len + cutprefix[(nb_towns-hops)] >= minimum) {
		pthread_mutex_unlock(&mutex);  
		(cuts)++ ;

		if(pere == 1) tab_threads[getTID()].occupe = 0;

		//printf("		Je débloque dans %lx\n", pthread_self());

		return ALL_IS_OK;
	}

	//printf("		Je débloque dans %lx\n", pthread_self());
	pthread_mutex_unlock(&mutex);  

	if (hops == nb_towns) {
		int me = path [hops - 1];
		int dist = distance[me][0]; // retourner en 0

		//printf("	Je bloque dans %lx\n", pthread_self());
		
		pthread_mutex_lock(&mutex);
		if ( len + dist < minimum ) {
			minimum = len + dist;
			//printf("			Je modifie sol_len dans %lx\n", pthread_self());
			sol_len = len + dist;
			memcpy(sol, path, nb_towns*sizeof(int));
			print_solution (path, len+dist);
		}
		pthread_mutex_unlock(&mutex);

		//printf("		Je débloque dans %lx\n", pthread_self());
		
	}

	else {
		int me = path [hops - 1];        
		for (int i = 0; i < nb_towns; i++) {

			if (!present (i, hops, path)) {
				
				int dist = distance[me][i];
				path[hops] = i;

				struct arg_struct arg;
				arg.hops = hops + 1 ;
				arg.len = len + dist;
				memcpy(arg.path, path, nb_towns * sizeof (int));
				arg.pere = pere + 1;

				//printf("	Je passe devant %lx pour la %dème fois (fils %d) et %p\n", pthread_self(), i, pere, &arg);
				tsp ((void*)&arg);
				//printf("		Je passe derrière %lx pour la %dème fois (fils %d) et %p\n", pthread_self(), i, pere, &arg);
			}
		}
	}
	
	if(pere == 1) tab_threads[getTID()].occupe = 0;

	return ALL_IS_OK;
}

