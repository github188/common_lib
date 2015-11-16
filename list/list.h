#ifndef _list_h
#define _list_h


typedef  struct list_node
{
	void * prev;
	void * next;
}list_node_t;



void list_init(void * node)
{
    ((list_node_t*)node)->next = ((list_node_t*)node)->prev = node;
}


int list_is_empty(void * node)
{
    return (((list_node_t*)node)->next == node);
}


void list_link_node(void * prev,void * next)
{

	((list_node_t*)prev)->next = next;
	((list_node_t*)next)->prev = prev;
}


void list_insert_after(void *pos, void *node)
{
    ((list_node_t*)node)->prev = pos;
    ((list_node_t*)node)->next = ((list_node_t*)pos)->next;
    ((list_node_t*) ((list_node_t*)pos)->next) ->prev = node;
    ((list_node_t*)pos)->next = node;
}



void list_insert_before(void *pos, void *node)
{
	list_insert_after(((list_node_t*)pos)->prev,node);
}


void list_insert_tail(void *lst,void *pos)
{


    list_node_t *lst_last = (list_node_t *) ((list_node_t*)lst)->prev;
    list_node_t *pos_next = (list_node_t *) ((list_node_t*)pos)->next;

    list_link_node(pos,lst);
    list_link_node(lst_last,pos_next);


	
}


void list_insert_head(void *pos, void *lst)
{
	list_insert_tail(((list_node_t*)pos)->prev, lst);
}




void  list_erase(void *node)
{
    list_link_node(((list_node_t*)node)->prev, ((list_node_t*)node)->next);
    list_init(node); /*safer*/
}


list_node_t * list_find_node(void *list, void *node)
{
    list_node_t *p = (list_node_t *) ((list_node_t*)list)->next;
    while (p != list && p != node)
	p = (list_node_t *) p->next;

    return p==node ? p : NULL;
}



list_node_t* list_search(list_node_t *list, void *value,int (*comp)(void *value, const void * node))       		
{
    list_node_t *p = (list_node_t *) ((list_node_t*)list)->next;
    while (p != list && (*comp)(value, p) != 0)
	p = (list_node_t *) p->next;

    return p==list ? NULL : p;
}


int  list_size(const void *list)
{
    const list_node_t *node = (const list_node_t*) ((const list_node_t*)list)->next;
    int count = 0;

    while (node != list) {
	++count;
	node = (list_node_t*)node->next;
    }

    return count;
}




#endif  /*_list_h*/

