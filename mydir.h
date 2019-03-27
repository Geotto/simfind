#ifndef MYDIR_H_INCLUDED
#define MYDIR_H_INCLUDED

#include <dirent.h>

/**
 * 文件入口信息
 */
typedef struct MyDirEnt{
    /**
     * 父目录
     */
    char *parent;

    /**
     * 文件入口
     */
    struct dirent *ent;
} *MDIRENT;

/**
 * 创建文件入口信息
 * @param parent 父目录名
 * @param ent 文件入口
 * @return 创建的文件入口信息，如果创建失败则返回NULL
 */
MDIRENT MyDirEnt_Create(char *parent, struct dirent *ent);

/**
 * 获取路径
 * @param md 文件入口
 * @return 路径字符串。如果失败则返回NULL，需要手动释放返回的字符串
 */
char *MyDirEnt_Path(MDIRENT md);

/**
 * 销毁文件入口信息
 */
void MyDirEnt_Destory(MDIRENT ent);

#endif // MYDIR_H_INCLUDED
