int nlibre = 123;
pthread_cond_t c;
pthread_mutex_t mutex;
pthread_cond_init(&c, NULL);

void allouer(int n){
	pthread_mutex_lock(&mutex);
	while(n > nlibre){
		pthread_cond_wait(&c, &mutex);
		pthread_cond_signal(&c);
	}
	nlibre = nlibre - n;
	pthread_mutex_unlock(&mutex);
}

void liberer(int m){
	pthread_mutex_lock(&mutex);
	nlibre = nlibre + m;
	pthread_cond_broadcast(&c);
	pthread_mutex_unlock(&mutex);
	pthread_cond_signal(&c);
}