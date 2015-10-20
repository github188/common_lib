#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>  
#include<string.h>  


#undef  DBG_ON
#undef  FILE_NAME
#define 	DBG_ON  	(0x01)
#define 	FILE_NAME 	"pthread_specific:"
#define 	dbg_printf(fmt,arg...) \
	do{if(DBG_ON)fprintf(stderr,FILE_NAME"%s(line=%d)->"fmt,__FUNCTION__,__LINE__,##arg);}while(0)
		


typedef struct unit
{	
	unsigned int num1;
	unsigned int num2;
}unitt_t;

typedef struct global_test
{
	pthread_key_t handle_key;
	unitt_t	* pthread_handle;
	
}global_test_t;


static global_test_t *  global = NULL; 

int global_init(unsigned int num)
{
	int ret = -1;
	if(NULL != global)
	{
		dbg_printf("this is has been init ! \n");
		return(-1);
	}

	global = calloc(1,sizeof(*global));
	if(NULL == global)
	{
		dbg_printf("global is fail ! \n");
		return(-2);
	}

	global->pthread_handle = calloc(1,num*sizeof(unitt_t));
	if(NULL == global->pthread_handle)
	{
		dbg_printf("calloc fail ! \n");
		goto fail;
	}

	ret = pthread_key_create(&global->handle_key, NULL);
	if(ret)
	{
		dbg_printf("pthread_key_create is fail ! \n");
		goto fail;
	}


	return(0);

fail:

	if(NULL != global->pthread_handle)
	{
		free(global->pthread_handle);
		global->pthread_handle = NULL;
	}

	if(NULL != global)
	{
		free(global);
		global = NULL;

	}

	return(-1);

	

}


void global_uninit(void)
{
	if(NULL == global)return;
	pthread_key_delete(global->handle_key);
	if(NULL != global->pthread_handle)
	{
		free(global->pthread_handle);
		global->pthread_handle = NULL;
	}

	if(NULL != global)
	{
		free(global);
		global = NULL;

	}
}


int global_set_value(global_test_t * pglobal,void * value)
{
	if(NULL == pglobal )
	{
		dbg_printf("please check the param ! \n");
		return(-1);
	}
	pthread_setspecific(pglobal->handle_key, (void *)value);
	
	return(0);
}


void * global_get_value(global_test_t * pglobal)
{
	if(NULL == pglobal)
	{
		dbg_printf("please check the param ! \n");
		return(NULL);
	}
	return(pthread_getspecific(pglobal->handle_key));
}




void * global_fun(void * arg)
{

	unitt_t * unit_test_temp = NULL;
	unsigned int  index = *(unsigned int *)arg;
	dbg_printf("index === %d \n",index);
	global_test_t * global_handle = global;
	unitt_t * unit_test = (unitt_t*)(global_handle->pthread_handle+index);
	if(NULL == global_handle ||  NULL == unit_test)
	{
		dbg_printf("check the param ! \n");
		return(NULL);
	}

	unit_test->num1 = 1;
	unit_test->num2 = 2;
	global_set_value(global_handle,(void*)unit_test);

	int is_run = 1;
	while(is_run)
	{
		unit_test_temp = (unitt_t *)global_get_value(global_handle);		
		if(index == 1 )
		{
			unit_test->num1 += 10;
			unit_test->num2 += 20;	
		}
		else
		{
			unit_test->num1 += 100;
			unit_test->num2 += 200;
		}

		dbg_printf("unit_test->num1==%d unit_test->num2==%d  index==%d \n",unit_test->num1,unit_test->num2,index);
		sleep(2);


	}


}




   

int main()  
{  

	dbg_printf("this is a test ! \n");
	global_init(3);
	pthread_t test1;  
	pthread_t test2;
	unsigned int index1,index2;
	index1 = 1;
	pthread_create(&test1, NULL,global_fun,(void*)&index1); 
	pthread_detach(test1);
	index2 = 2;
	pthread_create(&test2, NULL,global_fun,(void*)&index2);
	pthread_detach(test2);

	int is_run = 1;
	while(is_run)
	{
	//	dbg_printf("this is main loop ! \n");
		sleep(5);

	}
    return 0;  
}  
