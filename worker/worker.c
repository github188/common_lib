#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "../common.h"
#include "worker.h"



#undef  DBG_ON
#undef  FILE_NAME
#define 	DBG_ON  	(0x01)
#define 	FILE_NAME 	"worker:"



static void *workq_server (void *arg)
{
    struct timespec timeout;
    work_handle_t *wq = (work_handle_t *)arg;
    work_data_t *we = NULL;
    int status, timedout;

    status = pthread_mutex_lock (&wq->mutex);
    if (status != 0)
        return NULL;

    while (1)
	{
        timedout = 0;
        dbg_printf (" %ld Worker waiting for work\n",pthread_self());
        clock_gettime (CLOCK_REALTIME, &timeout);
        timeout.tv_sec += 2;

        while (wq->first == NULL && !wq->is_quit)
		{
           // status = pthread_cond_timedwait (&wq->cond, &wq->mutex, &timeout);
			status = pthread_cond_wait (&wq->cond, &wq->mutex);
                    
            if (status == ETIMEDOUT) 
			{
                dbg_printf("Worker wait timed out\n");
                timedout = 1;
                break;
            } 
			else if (status != 0)
			{
     
				dbg_printf("something wrong ! \n");
                wq->run_pthreads--;
                pthread_mutex_unlock (&wq->mutex);
                return NULL;
            }
			else
			{
				
			}
        }

		
        dbg_printf("%ld bengin to work ! \n", pthread_self());
        we = wq->first;

        if (we != NULL) 
		{
            wq->first = we->next;
            if (wq->last == we)
                wq->last = NULL;
			
            status = pthread_mutex_unlock (&wq->mutex);
            if (status != 0)
                return NULL;

			if(NULL != wq->fun)
			{
				 wq->fun(we->data);
			}
           
            free (we);
			we = NULL;
            status = pthread_mutex_lock (&wq->mutex);
            if (status != 0)
                return NULL;
        }

     
        if (wq->first == NULL && wq->is_quit)
		{
            dbg_printf("%ld bengin to exit ! \n", pthread_self());
            wq->run_pthreads--;

            if (wq->run_pthreads == 0)
                pthread_cond_broadcast (&wq->cond);
			
            pthread_mutex_unlock (&wq->mutex);
            return NULL;
        }

        if (wq->first == NULL && timedout)
		{
            wq->run_pthreads--;
			wq->idle_pthreads ++;
            
        }
    }

    pthread_mutex_unlock (&wq->mutex);

    return NULL;
}


int workq_init (work_handle_t *wq, int threads_max, void *(*fun)(void *arg))
{
    int ret = -1;
    ret = pthread_attr_init (&wq->attr);
	if(0 != ret)
	{
		dbg_printf("pthread_attr_init is fail ! \n");
		return(-1);
	}

    ret = pthread_attr_setdetachstate(&wq->attr, PTHREAD_CREATE_DETACHED); 
    if (ret != 0)
	{
        pthread_attr_destroy (&wq->attr);
		dbg_printf("pthread_attr_setdetachstate is fail ! \n");
		return (-2);
    }
    ret = pthread_mutex_init (&wq->mutex, NULL);
    if (ret != 0)
	{
		dbg_printf("pthread_mutex_init is fail ! \n");
        pthread_attr_destroy (&wq->attr);
        return (-3);
    }
    ret = pthread_cond_init (&wq->cond, NULL);
    if (ret != 0) 
	{
		dbg_printf("pthread_cond_init is fail ! \n");
        pthread_mutex_destroy (&wq->mutex);
        pthread_attr_destroy (&wq->attr);
        return (-4);
    }
    wq->is_quit = 0;                      
    wq->first = wq->last = NULL;      
    wq->max_pthreads= threads_max;         
    wq->run_pthreads = 0;                  
    wq->idle_pthreads = 0;                       
    wq->fun = fun;			
    wq->valid = WORKQ_VALID;
	
    return 0;
}





int workq_destroy (work_handle_t *wq)
{
    int status, status1, status2;

    if (wq->valid != WORKQ_VALID)
    {
		dbg_printf("please check the param ! \n");
		return (-1);
	}
       
    status = pthread_mutex_lock (&wq->mutex);
    if (status != 0)
        return status;
    wq->valid = 0;             


    if (wq->run_pthreads > 0)
	{ 
        wq->is_quit = 1;
        if (wq->idle_pthreads > 0)
		{
            status = pthread_cond_broadcast (&wq->cond); 
            if (status != 0) 
			{
                pthread_mutex_unlock (&wq->mutex);
                return status;
            }
        }

        while (wq->run_pthreads > 0) 
		{
            status = pthread_cond_wait (&wq->cond, &wq->mutex); 
            if (status != 0)
			{
                pthread_mutex_unlock (&wq->mutex);
                return status;
            }
        }       
    }
    status = pthread_mutex_unlock (&wq->mutex);
    if (status != 0)
        return status;
	
    status = pthread_mutex_destroy (&wq->mutex);
    status1 = pthread_cond_destroy (&wq->cond);
    status2 = pthread_attr_destroy (&wq->attr);
    return (status ? status : (status1 ? status1 : status2));
}



int workq_add (work_handle_t *wq, void *element)
{
    work_data_t *item;
    pthread_t id;
    int status;

    if (wq->valid != WORKQ_VALID)
        return (-1);

    item = (work_data_t *)malloc (sizeof (work_data_t));
    if (item == NULL)
        return (-2);
	
    item->data = element;
    item->next = NULL;
    status = pthread_mutex_lock (&wq->mutex);
    if (status != 0)
	{
        free (item);
        return status;
    }


    if (wq->first == NULL)
        wq->first = item;
    else
        wq->last->next = item;
    wq->last = item;


	if (wq->run_pthreads < wq->max_pthreads )
	{

        dbg_printf("Creating new worker\n");
        status = pthread_create (&id, &wq->attr, workq_server, (void*)wq);
         
        if (status != 0) 
		{
            pthread_mutex_unlock (&wq->mutex);
            return status;
        }
        wq->run_pthreads ++;
    }
	else
	{
        status = pthread_cond_signal (&wq->cond);
        if (status != 0) 
		{
            pthread_mutex_unlock (&wq->mutex);
            return status;
        }	

	}
    pthread_mutex_unlock (&wq->mutex);
    return 0;
}





