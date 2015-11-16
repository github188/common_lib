#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "list.h"


#undef  DBG_ON
#undef  FILE_NAME
#define 	DBG_ON  	(0x01)
#define 	FILE_NAME 	"list_test:"
#define 	dbg_printf(fmt,arg...) \
	do{if(DBG_ON)fprintf(stderr,FILE_NAME"%s(line=%d)->"fmt,__FUNCTION__,__LINE__,##arg);}while(0)




typedef struct list_test
{
	struct list_test * prev;
	struct list_test * next;
	int value;

}list_test_t;

	
/*not all right !*/
int main(int argc,char * argv[])
{

	int i = 0;
	
	list_test_t list;
	list_test_t * p = NULL;
	list.value = -1;
	list_init(&list);

	list_test_t node[4];

	for(i=0;i<4;++i)
	{
		node[i].value = i;
		list_insert_before(&list,&node[i]);
	}

    for (i=0, p=list.next; i<4; ++i, p=p->next) 
	{
		dbg_printf("%d \n",p->value);  /*0 1 2 3 */

	}


	list_init(&list);
	for(i=0;i<4;++i)
	{
		node[i].value = i;
		list_insert_after(&list,&node[i]);
	}
    for (i=0, p=list.next; i<4; ++i, p=p->next) 
	{
		dbg_printf("%d \n",p->value);  /*3 2 1 0 */

	}


	list_init(&list);

    for (i=0, p=list.next; i<4; ++i, p=p->next) 
	{
		dbg_printf("%d \n",p->value);  
	}



	
	

	

	
	
	
	


	return(0);
}
