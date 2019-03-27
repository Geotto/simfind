#ifndef MYLIST_H_INCLUDED
#define MYLIST_H_INCLUDED

#include "mydir.h"

#define MYLIST_BUF_COUNT (1000)
#define MYLIST_BUF_SIZE (1000)

typedef struct MyListNode{
    MDIRENT md;
    struct MyListNode *prev;
    struct MyListNode *next;
} *MLNODE;

typedef struct MyList{
    MLNODE list_n;
    MLNODE list_t;
    MLNODE list_d;
    int buf_size;
    int buf_len;
    MLNODE *buf;
} *ML;

/**
 * 创建链表
 */
ML ML_Create();

/**
 * 附加到链表
 */
int ML_Append(ML list, MDIRENT md);

/**
 * 从链表移除
 */
MDIRENT ML_Pop(ML list);

/**
 * 销毁链表
 */
void ML_Destroy(ML list);

#endif // MYLIST_H_INCLUDED
