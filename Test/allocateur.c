#include <stdio.h>
#include <pthread.h>

int nlibre = 123;

void* ALL_IS_OK = (void*)123456789L;

pthread_cond_t c;
pthread_mutex_t mutex;

void* allouer(void* n){
	int i;
	int* p_n = (int*)n;
	for(i=0; i<100; i++){
		pthread_mutex_lock(&mutex);
		while(*p_n > nlibre){
			pthread_cond_wait(&c, &mutex);
			pthread_cond_signal(&c);
		}
		nlibre = nlibre - *p_n;
		printf("ALLOCATION : %d\n", nlibre);
		pthread_mutex_unlock(&mutex);
	}
	return ALL_IS_OK;
}

void* liberer(void* m){
	int i;
	int* p_m = (int*)m;
	for(i=0; i<100; i++){
		pthread_mutex_lock(&mutex);
		nlibre = nlibre + *p_m;
		printf("	LIBERATION : %d\n", nlibre);
		pthread_cond_broadcast(&c);
		pthread_mutex_unlock(&mutex);
		pthread_cond_signal(&c);
	}
	return ALL_IS_OK;
}

void main(){
	pthread_t p1, p2;
	void* status;

	int* p_n = malloc(sizeof(int));
	int* p_m = malloc(sizeof(int));

	*p_n = 10;
	*p_m = 2;

	pthread_cond_init(&c, NULL);
	pthread_mutex_init(&mutex, NULL);

	pthread_create(&p1, NULL, allouer, (void*)p_n);
	pthread_create(&p2, NULL, liberer, (void*)p_m);

	pthread_join(p1, &status);
	if (status == ALL_IS_OK) printf("Thread %lx completed ok \n", p1);

	pthread_join(p2, &status);
	if (status == ALL_IS_OK) printf("Thread %lx completed ok \n", p2);

	free(p_n);
	free(p_m);
}