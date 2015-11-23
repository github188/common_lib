#ifndef  _worker_h
#define _worker_h

#include <pthread.h>


#define WORKQ_VALID     0xdec1992

typedef struct work_data
{
	struct work_data * next;
	void * data;
	
}work_data_t;



typedef struct work_handle
{
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	pthread_attr_t attr;
	work_data_t * first;
	work_data_t * last;
	int valid;
	int is_quit;
	int max_pthreads;
	int run_pthreads;
	int idle_pthreads;
	void * (*fun)(void * arg);
	
	
}work_handle_t;



int workq_init (work_handle_t *wq, int threads_max, void *(*fun)(void *arg));
int workq_destroy (work_handle_t *wq);
int workq_add (work_handle_t *wq, void *element);


#endif  /*_worker_h*/

