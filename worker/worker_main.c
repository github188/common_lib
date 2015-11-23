#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../common.h"
#include "worker.h"




#undef  DBG_ON
#undef  FILE_NAME
#define 	DBG_ON  	(0x01)
#define 	FILE_NAME 	"main:"




#define ITERATIONS      6

typedef struct power_tag 
{
    int         value;
    int         power;
} power_t;

typedef struct engine_tag
{
    struct engine_tag   *link;
    pthread_t           thread_id;
    int                 calls;
} engine_t;


pthread_key_t engine_key;     
pthread_mutex_t engine_list_mutex = PTHREAD_MUTEX_INITIALIZER;
engine_t *engine_list_head = NULL;
work_handle_t workq;



void destructor (void *value_ptr)
{
    engine_t *engine = (engine_t*)value_ptr;

    pthread_mutex_lock (&engine_list_mutex);
    engine->link = engine_list_head;
    engine_list_head = engine;
    pthread_mutex_unlock (&engine_list_mutex);
}


void * engine_routine (void *arg)
{
    engine_t *engine;
    power_t *power = (power_t*)arg;
    int result, count;
    int status;

    engine = pthread_getspecific (engine_key);
    if (engine == NULL) 
	{
        engine = (engine_t*)malloc (sizeof (engine_t));
        status = pthread_setspecific (engine_key, (void*)engine);
       
        if (status != 0)
            dbg_printf("Set tsd \n");
		
        engine->thread_id = pthread_self ();
        engine->calls = 1;
    }
	else
        engine->calls++;
	
 	result = 1; 
    for (count = 1; count <= power->power; count++)
        result *= power->value;

    printf ("Engine: computing %d^%d==%d\n", power->value, power->power,result);
    free (arg);
}


void *thread_routine (void *arg)
{
    power_t *element;
    int count;
    unsigned int seed = (unsigned int)time (NULL);
    int status;

    for (count = 0; count < ITERATIONS; count++) 
	{
        element = (power_t*)malloc (sizeof (power_t));
        if (element == NULL)
            dbg_printf("Allocate element \n");
		
        element->value = rand_r (&seed) % 20;
        element->power = rand_r (&seed) % 7;
        dbg_printf("Request: %d^%d\n",element->value, element->power);
          
        status = workq_add (&workq, (void*)element);
        if (status != 0)
            dbg_printf ("Add to work queue");
        sleep (rand_r(&seed) % 5);
    }
    return NULL;
}

int main (int argc, char *argv[])
{
    pthread_t thread_id;
    engine_t *engine;
    int count = 0, calls = 0;
    int status;

    status = pthread_key_create (&engine_key, destructor);
    if (status != 0)
        dbg_printf("Create key \n");
	
    status = workq_init (&workq, 4, engine_routine);
    if (status != 0)
        dbg_printf ( "Init work queue \n");
	
    status = pthread_create (&thread_id, NULL, thread_routine, NULL);
    if (status != 0)
        dbg_printf ("Create thread\n");

	
    (void)thread_routine (NULL);

    status = pthread_join (thread_id, NULL);
    if (status != 0)
        dbg_printf ( "Join thread \n");

	
    status = workq_destroy (&workq); /*死等模式将无法退出*/
    if (status != 0)
        dbg_printf ("Destroy work queue \n");

  
    return 0;
}

