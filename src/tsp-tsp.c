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

void* tsp (void* arguments)
{	
		//int hops = tab_threads[getTID()].arguments.hops;
		//int len = tab_threads[getTID()].arguments.len;
		tsp_path_t path;
		//memcpy(path, tab_threads[getTID()].arguments.path, MAX_TOWNS * sizeof (int));
		int len = ((struct arg_struct*)arguments)->len;
		int hops = ((struct arg_struct*)arguments)->hops;
		memcpy(path,((struct arg_struct*)arguments)->path, MAX_TOWNS * sizeof (int));

		/*for(int k=0;k<10;k++){
			printf("%d",path[k]);
		}
		printf("#####");
		printf("\n");*/	

		pthread_mutex_lock(&mutex);

		if (len + cutprefix[(nb_towns-hops)] >= minimum) {
		    (cuts)++ ;
				pthread_mutex_unlock(&mutex);
		    return ALL_IS_OK;
		  }
		pthread_mutex_unlock(&mutex);    




    if (hops == nb_towns) {
	    int me = path [hops - 1];
	    int dist = distance[me][0]; // retourner en 0

		pthread_mutex_lock(&mutex);	

    if ( len + dist < minimum ) {
		    minimum = len + dist;
		    sol_len = len + dist;
		    memcpy(sol, path, nb_towns*sizeof(int));
		    print_solution (path, len+dist);
	  }

		pthread_mutex_unlock(&mutex);
    }




 		else {
        int me = path [hops - 1];        
        for (int i = 0; i < nb_towns; i++) {
						/*printf("I : %d, HOPS : %d, PATH : ",i,hops);
						for(int k=0;k<10;k++){
							printf("%d ",path[k]);
						}
						printf("\n");*/	
            if (!present (i, hops, path)) {
                path[hops] = i;
								//tab_threads[i].arguments.path[hops] = i;
                int dist = distance[me][i];
								struct arg_struct arg ;
								//(tab_threads[i].arguments.hops)++;
								//tab_threads[i].arguments.len += dist;
					
								arg.hops = hops + 1 ;
								arg.len = len + dist;
								memcpy(arg.path, path, MAX_TOWNS * sizeof (int));

                tsp ((void*)&arg);

								//(tab_threads[i].arguments.hops)--;
								//tab_threads[i].arguments.len -= dist;
            }
        }
    }
return ALL_IS_OK;
}

