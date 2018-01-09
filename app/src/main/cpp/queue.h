//
// @author admin on 2018/1/8.
//

#ifndef MYLIVE_QUEUE_H
#define MYLIVE_QUEUE_H

//创建双向链表
extern int create_queue();
//销毁双向链表
extern int destroy_queue();
//判断双向链表是否为空，为空返回1，否则返回0
extern int queue_is_empty();
//返回双向链表的大小
extern int queue_size();
//根据双向链表的index返回元素指针，为空则返回NULL
extern void* get_element_from_queue(int index);
//获取双向链表的第一个元素，成功返回元素指针，否则返回NULL
extern void* get_first_element();
//获取双向链表的最后一个元素，成功返回元素指针，否则返回NULL
extern void* get_last_element();
//将元素插入到双向列表的index上，成功返回0，失败返回-1
extern int queue_insert(int index, void *element);
//将元素插入到表头位置，成功返回0，失败返回-1
extern int queue_insert_first(void *element);
//将元素插入到末尾位置，成功返回0，失败返回-1
extern int queue_insert_last(void *element);
//删除index上的元素,成功返回0，失败返回-1
extern int delete_element(int index);
//删除第一个节点的元素，成功返回0，失败返回-1
extern int delete_first_element();
//删除最后一个节点上的元素，成功返回0，失败返回-1
extern int delete_last_element();
#endif //MYLIVE_QUEUE_H
