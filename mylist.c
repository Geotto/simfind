#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mylist.h"

ML ML_Create(){
    ML list = (ML)malloc(sizeof(struct MyList));
    if(list == NULL)
        return NULL;

    list->list_n = (MLNODE)malloc(sizeof(struct MyListNode));
    if(list->list_n == NULL){
        free(list);
        return NULL;
    }

    list->list_n->md = NULL;
    list->list_n->prev = NULL;
    list->list_n->next = NULL;

    list->list_t = (MLNODE)malloc(sizeof(struct MyListNode));
    if(list->list_t == NULL){
        free(list->list_n);
        free(list);
        return NULL;
    }

    list->list_t->md = NULL;
    list->list_t->prev = NULL;
    list->list_t->next = NULL;

    list->list_d = (MLNODE)malloc(sizeof(struct MyListNode));
    if(list->list_d == NULL){
        free(list->list_t);
        free(list->list_n);
        free(list);
        return NULL;
    }

    list->list_d->md = NULL;
    list->list_d->prev = NULL;
    list->list_d->next = NULL;

    list->buf = (MLNODE *)malloc(sizeof(struct MyListNode) * MYLIST_BUF_COUNT);
    if(list->buf == NULL){
        free(list->list_d);
        free(list->list_t);
        free(list->list_n);
        free(list);
        return NULL;
    }

    for(int i=0; i<MYLIST_BUF_COUNT; i++){
        if(i == 0){
            list->buf[i] = (MLNODE)malloc(sizeof(struct MyListNode) * MYLIST_BUF_SIZE);
            if(list->buf[i] == NULL){
                free(list->buf);
                free(list->list_d);
                free(list->list_t);
                free(list->list_n);
                free(list);
                return NULL;
            }
        }else{
            list->buf[i] = NULL;
        }
    }

    list->buf_size = sizeof(struct MyListNode) * MYLIST_BUF_SIZE;
    list->buf_len = 0;

    return list;
}

MLNODE ML_Alloc(ML list){
    if(list->list_d->next != NULL){
        MLNODE node = list->list_d->next;
        list->list_d->next = node->next;
        if(node->next != NULL){
            node->next->prev = NULL;
        }

        node->next = NULL;
        node->prev = NULL;
        return node;
    }

    int size = sizeof(struct MyListNode);
    int size_f = size * MYLIST_BUF_SIZE;
    int buf_index = list->buf_len / size_f;
    if(buf_index < MYLIST_BUF_COUNT && list->buf[buf_index] != NULL){
        int offset = list->buf_len % size_f;
        MLNODE node = (MLNODE)(*(list->buf + buf_index) + offset / size);
        node->md = NULL;
        node->prev = NULL;
        node->next = NULL;

        list->buf_len = list->buf_len + size;
        return node;
    }

    if(buf_index < MYLIST_BUF_COUNT){
        list->buf[buf_index] = (MLNODE)malloc(size_f);
        if(list->buf[buf_index] == NULL){
            fprintf(stderr, "Outof Memory\n");
            return NULL;
        }

        int offset = list->buf_len % (size_f);
        MLNODE node = (MLNODE)(*(list->buf + buf_index) + offset / size);
        node->md = NULL;
        node->prev = NULL;
        node->next = NULL;

        list->buf_len = list->buf_len + size;
        return node;
    }

    fprintf(stderr, "Can not allocate more memory\n");
    return NULL;
}

int ML_Append(ML list, MDIRENT md){
    MLNODE node = ML_Alloc(list);
    if(node == NULL)
        return -1;

    node->md = md;

    if(list->list_n->next == NULL){
        list->list_n->next = node;
        list->list_t->next = node;
    }else{
        MLNODE t_node = list->list_t->next;
        t_node->next = node;
        node->prev = t_node;
        list->list_t->next = node;
    }

    return 0;
}

MDIRENT ML_Pop(ML list){
    if(list->list_n->next == NULL)
        return NULL;

    MLNODE node = list->list_n->next;
    list->list_n->next = node->next;
    if(node->next != NULL){
        node->next->prev = list->list_n;
    }

    if(list->list_t->next == node){
        list->list_t->next = NULL;
    }

    MDIRENT md = node->md;
    node->prev = NULL;
    node->next = list->list_d->next;
    list->list_d->next = node;

    return md;
}

void ML_Destroy(ML list){
    for(int i=0; i<MYLIST_BUF_COUNT; i++){
        if(list->buf[i] != NULL){
            free(list->buf[i]);
        }
    }

    free(list->buf);
    free(list->list_d);
    free(list->list_t);
    free(list->list_n);
    free(list);
}
