#ifndef MYSEARCH_H_INCLUDED
#define MYSEARCH_H_INCLUDED

#include <regex.h>
#include <sys/stat.h>
#include <unistd.h>
#include "mydir.h"

/**
 * 搜索选项
 */
typedef struct MySearchOptions{
    /**
     * 属性选项
     */
    unsigned char attr;

    /**
     * 模式选项
     */
    mode_t mode;

    /**
     * 组id
     */
    gid_t gid;

    /**
     * 用户id
     */
    uid_t uid;

    /**
     * 名称
     */
    char *name;

    /**
     * 搜索目录
    */
    char *dir;

    /**
     * 正则表达式
     */
    regex_t *preg;
} *MSEARCHOPTS;

/**
 * 所有属性
 */
#define MSEARCHOPTS_ATTR_ALL (0xff)

/**
 * 忽略大小写
 */
#define MSEARCHOPTS_ATTR_IGNORE (0x01);

/**
 * 名称选项不正确 
*/
#define MS_ENAME (-1);

/**
 * 类型选项不正确
*/
#define MS_ETYPE (-2);

/**
 * 目录选项不正确
*/
#define MS_EDIR (-3);

/**
 * 正则表达式错误
*/
#define MS_EREG (-4);

/**
 * 创建MySearchOptions
 * @return 如果成功返回生成的搜索选项，否则返回NULL
*/
MSEARCHOPTS MySearchOptions_Create();

/**
 * 打印任务
 * @param options 搜索选项
 * @param ent 文件入口
*/
void MySearch_Action_Print(MSEARCHOPTS options, MDIRENT ent);

/**
 * 进行过滤
 * @param options 搜索选项
 * @param ent 文件入口
 * @return 测试是否通过，0-未通过，1-通过
*/
int MySearch_Test(MSEARCHOPTS options, MDIRENT ent);

/**
 * 将name编译为正则表达式
 * @param options 搜索选项
 * @return 0-成功，其他失败，失败值参考regcomp错误定义
*/
int MySearchOptions_PregCompile(MSEARCHOPTS options);

/***
 * 对dir指示的目录执行搜索操作，并根据options中的条件进行过滤，并对符合条件的文件执行action操作
 * @param options 搜索选项
 * @param action 对符合条件的文件执行的操作
 * @param dir 需要搜索的目录
*/
void MySearch_Search(MSEARCHOPTS options, void (*action)(MSEARCHOPTS, MDIRENT), char *dir);

/**
 * 从命令行参数生成搜索选项
 * @param options 搜索选项
 * @param argc 命令行参数数量
 * @param argv 命令行参数
 * @return 0-成功，其他-失败
*/
int MySearch_BuildOptions(MSEARCHOPTS options, int argc, char *argv[]);

/**
 * 销毁MySearchOptions 
*/
void MySearchOptions_Destory(MSEARCHOPTS mso);

#endif // MYSEARCH_H_INCLUDED
