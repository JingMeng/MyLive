//
// @author admin on 2018/1/8.
//
#include <stdio.h>
#include <malloc.h>
typedef struct queue_node {
    struct queue_node* prev;
    struct queue_node* next;
    void *p ;
} node;

static node *phead = NULL ;
static int count = 0;
static node* create_node(void *pval){
    node *pnode = NULL ;
    pnode = malloc(sizeof(node));
    if(pnode) {
        pnode->prev = pnode->next = pnode;
        pnode->p = pval;
    }
    return pnode;
}

int create_queue() {
    phead = create_node(NULL);
    if(!phead){
        return -1;
    }
    count = 0;
    return 0;
}

int queue_is_empty(){
    return count == 0;
}

int queue_size(){
    return count ;
}

static node* get_node(int index){
    if(index < 0 || index >=count ){
        return NULL ;
    }
    if(index <= (count / 2)){
        int i = 0;
        node *ponde = phead->next;
        while((i++) < index)
            ponde = ponde->next;
        return ponde;
    }
    int j = 0;
    int rindex = count - index -1;
    node *rnode = phead->prev;
    while ((j++)<rindex)
        rnode = rnode->prev;
    return rnode;
}

//获取双向链表的第一个元素，成功返回元素指针，否则返回NULL
void* get_first_node(){
    return get_node(0);
}
//获取双向链表的最后一个元素，成功返回元素指针，否则返回NULL
void* get_last_node(){
    return get_node(count-1);
}

void* get_element_from_queue(int index){
    node *pindex = get_node(index);
    if(!pindex){
        return NULL;
    }
    return pindex->p;
}
void* get_first_element(){
    return get_element_from_queue(0);
}
//获取双向链表的最后一个元素，成功返回元素指针，否则返回NULL
void* get_last_element(){
    return get_element_from_queue(count-1);
}
//将元素插入到双向列表的index上，成功返回0，失败返回-1
int queue_insert(int index, void *element){
    if(index == 0){
        return queue_insert_first(element);
    }
    node *pindex = get_node(index);
    if(!pindex)
        return -1;
    node *pnode = create_node(element);
    if(!pnode){
        return -1;
    }
    pnode->prev = pindex->prev;
    pnode->next = pindex;
    pindex->prev->next = pnode;
    pindex->prev = pnode;
    count++;
    return 0;
}
//将元素插入到表头位置，成功返回0，失败返回-1
int queue_insert_first(void *element){
    node *pnode = create_node(element);
    if(!pnode)
        return -1;
    pnode->prev = phead;
    pnode->next = phead->next;
    phead->next->prev = pnode;
    phead->next = pnode;
    count++;
    return 0;
}
//将元素插入到末尾位置，成功返回0，失败返回-1
int queue_insert_last(void *element){
    node *pnode = create_node(element);
    if(!pnode){
        return -1;
    }
    pnode->next = phead;
    pnode->prev = phead->prev;
    phead->prev->next = pnode;
    phead->next = pnode;
    count++;
    return 0;
}
//删除index上的元素,成功返回0，失败返回-1
int delete_element(int index){
    node *pindex = get_node(index);
    if(!pindex){
        return -1;
    }
    pindex->next->next = pindex->prev;
    pindex->prev->next = pindex->next;
    free(pindex);
    count--;
    return 0;
}
//删除第一个节点的元素，成功返回0，失败返回-1
int delete_first_element(){
    return delete_element(0);
}
//删除最后一个节点上的元素，成功返回0，失败返回-1
int delete_last_element(){
    return delete_element(count - 1);
}

int destroy_queue(){
    if(!phead){
        return -1;
    }
    node *pnode = phead->next;
    node *ptmp = NULL ;
    while(pnode != phead){
        ptmp = pnode;
        pnode = pnode->next;
        free(ptmp);
    }
    free(phead);
    phead = NULL;
    count = 0;
    return 0;
}