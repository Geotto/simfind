#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "mylist.h"
#include "mydir.h"
#include "mysearch.h"

void printlist(ML list, int direction){
    if(direction == 0){
        MLNODE node = list->list_n->next;
        while(node != NULL){
            printf("%s\n", node->md->parent);
            node = node->next;
        }
    }else{
        MLNODE node = list->list_t->next;
        while(node != NULL){
            printf("%s\n", node->md->parent);
            node = node->prev;
        }
    }

    printf("************\n");
}

int test_mydirent(){
    DIR *pDir = opendir("/usr/bin");
    if(pDir == NULL){
        int err = errno;
        fprintf(stderr, "%s: %s\n", "/usr/bin", strerror(err));
        return 1;
    }

    struct dirent *ent = readdir(pDir);
    while(ent != NULL){
        MDIRENT md = MyDirEnt_Create("/usr/bin", ent);
        printf("%s-%s\n", md->parent, md->ent->d_name);
        MyDirEnt_Destory(md);

        ent = readdir(pDir);
    }

    return 0;
}

int test_mydirent_path(){
    DIR *pDir = opendir("/usr/bin");
    if(pDir == NULL){
        int err = errno;
        fprintf(stderr, "%s: %s\n", "/usr/bin", strerror(err));
        return 1;
    }

    struct dirent *ent = readdir(pDir);
    while(ent != NULL){
        MDIRENT md = MyDirEnt_Create("/usr/bin", ent);
        char *path = MyDirEnt_Path(md);
        if(path == NULL){
            ent = readdir(pDir);
            continue;
        }

        printf("%s\n", path);
        free(path);
        MyDirEnt_Destory(md);

        ent = readdir(pDir);
    }

    return 0;
}

int test_mylist_append(){
    DIR *pDir = opendir("/usr/bin");
    if(pDir == NULL){
        int err = errno;
        fprintf(stderr, "%s: %s\n", "/usr/bin", strerror(err));
        return 1;
    }

    ML list = ML_Create();
    struct dirent *ent = readdir(pDir);
    while(ent != NULL){
        MDIRENT md = MyDirEnt_Create("/usr/bin", ent);
        ML_Append(list, md);

        ent = readdir(pDir);
    }

    MDIRENT md = ML_Pop(list);
    while(md != NULL){
        char *path = MyDirEnt_Path(md);
        if(path == NULL){
            MyDirEnt_Destory(md);
            md = ML_Pop(list);
            continue;
        }

        printf("%s\n", path);
        free(path);

        md = ML_Pop(list);
    }

    return 0;
}

int test_mysearchoptions(){
    MSEARCHOPTS mso = MySearchOptions_Create();
    if(mso == NULL)
        return 1;

    mso->name = "\\.h$";
    mso->preg = (regex_t *)malloc(sizeof(regex_t));
    int result = regcomp(mso->preg, mso->name, REG_EXTENDED);
    if(result != 0){
        fprintf(stderr, "Can not compile regular express %s\n", mso->name);
        MySearchOptions_Destory(mso);
        return 1;
    }

    MySearchOptions_Destory(mso);
    return 0;
}

int test_mysearch_action_print(){
    DIR *pDir = opendir("/usr/bin");
    if(pDir == NULL){
        int err = errno;
        fprintf(stderr, "%s: %s\n", "/usr/bin", strerror(err));
        return 1;
    }

    ML list = ML_Create();
    struct dirent *ent = readdir(pDir);
    while(ent != NULL){
        MDIRENT md = MyDirEnt_Create("/usr/bin", ent);
        ML_Append(list, md);

        ent = readdir(pDir);
    }

    MSEARCHOPTS options = MySearchOptions_Create();
    if(options == NULL)
        return 1;

    MDIRENT md = ML_Pop(list);
    while(md != NULL){
        char *path = MyDirEnt_Path(md);
        if(path == NULL){
            MyDirEnt_Destory(md);
            md = ML_Pop(list);
            continue;
        }

        MySearch_Action_Print(options, md);
        free(path);

        md = ML_Pop(list);
    }

    MySearchOptions_Destory(options);
    return 0;
}

int test_mysearch_test_name(){
    MSEARCHOPTS options = MySearchOptions_Create();
    if(options == NULL)
        return 1;

    options->name = "sh";

    DIR *pDir = opendir("/bin");
    if(pDir == NULL){
        int err = errno;
        fprintf(stderr, "%s: %s\n", "/bin", strerror(err));

        MySearchOptions_Destory(options);
        return 1;
    }

    struct dirent *ent = readdir(pDir);
    while(ent != NULL){
        MDIRENT md = MyDirEnt_Create("bin", ent);
        if(md == NULL){
            MySearchOptions_Destory(options);
            return 1;
        }

        int result = MySearch_Test(options, md);
        if(strcmp("bash", ent->d_name) == 0 && result == 0){            
            MyDirEnt_Destory(md);
            MySearchOptions_Destory(options);
            return 1;
        }

        MyDirEnt_Destory(md);
        ent = readdir(pDir);
    }

    closedir(pDir);
    MySearchOptions_Destory(options);
    return 0;
}

int test_mysearch_test_preg(){
    MSEARCHOPTS options = MySearchOptions_Create();
    if(options == NULL)
        return 1;

    options->name = "std.*\\.h$";
    if(MySearchOptions_PregCompile(options) != 0){
        MySearchOptions_Destory(options);
        return 1;
    }

    DIR *pDir = opendir("/usr/include");
    if(pDir == NULL){
        int err = errno;
        fprintf(stderr, "%s: %s\n", "/usr/include", strerror(err));

        MySearchOptions_Destory(options);
        return 1;
    }

    struct dirent *ent = readdir(pDir);
    while(ent != NULL){
        MDIRENT md = MyDirEnt_Create("/usr/include", ent);
        if(md == NULL){
            MySearchOptions_Destory(options);
            return 1;
        }

        int result = MySearch_Test(options, md);
        if(strcmp("stdio.h", ent->d_name) == 0 && result == 0){            
            MyDirEnt_Destory(md);
            MySearchOptions_Destory(options);
            return 1;
        }

        MyDirEnt_Destory(md);
        ent = readdir(pDir);
    }

    closedir(pDir);
    MySearchOptions_Destory(options);
    return 0;
}

int test_mysearch_test_dir(){
    MSEARCHOPTS options = MySearchOptions_Create();
    if(options == NULL)
        return 1;

    options->mode = S_IFDIR;
    DIR *pDir = opendir("/usr");
    if(pDir == NULL){
        int err = errno;
        fprintf(stderr, "%s: %s\n", "/usr", strerror(err));

        MySearchOptions_Destory(options);
        return 1;
    }

    struct dirent *ent = readdir(pDir);
    while(ent != NULL){
        MDIRENT md = MyDirEnt_Create("/usr", ent);
        if(md == NULL){
            MySearchOptions_Destory(options);
            return 1;
        }

        int result = MySearch_Test(options, md);
        if(strcmp("bin", ent->d_name) == 0 && result == 0){            
            MyDirEnt_Destory(md);
            MySearchOptions_Destory(options);
            return 1;
        }

        MyDirEnt_Destory(md);
        ent = readdir(pDir);
    }

    closedir(pDir);
    MySearchOptions_Destory(options);
    return 0;
}

int test_mysearch_test_file(){
    MSEARCHOPTS options = MySearchOptions_Create();
    if(options == NULL)
        return 1;

    options->mode = S_IFREG;
    DIR *pDir = opendir("/bin");
    if(pDir == NULL){
        int err = errno;
        fprintf(stderr, "%s: %s\n", "/bin", strerror(err));

        MySearchOptions_Destory(options);
        return 1;
    }

    struct dirent *ent = readdir(pDir);
    while(ent != NULL){
        MDIRENT md = MyDirEnt_Create("/bin", ent);
        if(md == NULL){
            MySearchOptions_Destory(options);
            return 1;
        }

        int result = MySearch_Test(options, md);
        if(strcmp("bash", ent->d_name) == 0 && result == 0){            
            MyDirEnt_Destory(md);
            MySearchOptions_Destory(options);
            return 1;
        }

        MyDirEnt_Destory(md);
        ent = readdir(pDir);
    }

    closedir(pDir);
    MySearchOptions_Destory(options);
    return 0;
}

int test_mysearch_search(){
    int result = 0;
    MSEARCHOPTS options = MySearchOptions_Create();
    options->name = "^.+\\.h$";
    if(MySearchOptions_PregCompile(options) == 0){
        MySearch_Search(options, MySearch_Action_Print, "/usr");
    }else{
        result = 1;
    }

    MySearchOptions_Destory(options);
    return result;
}

int test_mysearch_buildoptions(){
    MSEARCHOPTS options = MySearchOptions_Create();
    if(options == NULL)
        return 1;

    char *argv[] = { "-name", ".*\\.h", "-type", "fd", "-dir", "/usr" };
    int result = MySearch_BuildOptions(options, 6, argv);
    MySearchOptions_Destory(options);

    return result;
}

int main(int argc, char *argv[])
{
    //return test_mydirent();
    //return test_mydirent_path();
    //return test_mylist_append();
    //return test_mysearchoptions();
    //return test_mysearch_action_print();
    //return test_mysearch_test_name();
    //return test_mysearch_test_preg();
    //return test_mysearch_test_dir();
    //return test_mysearch_test_file();
    //return test_mysearch_search();
    //return test_mysearch_buildoptions();

    MSEARCHOPTS options = MySearchOptions_Create();
    if(options == NULL)
        return 1;

    int result = MySearch_BuildOptions(options, argc, argv);
    if(result == 0){
        MySearch_Search(options, MySearch_Action_Print, options->dir == NULL ? "." : options->dir);
    }

    MySearchOptions_Destory(options);

    /* printf("Hello world!\n");
    ML list = ML_Create();
    for(int i=0; i<10000; i++){
        char name[21];
        sprintf(name, "element at %d", i);
        // printf(name);
        //printlist(list, 0);
        int result = ML_Append(list, name);
        if(result == -1)
            break;

        //printlist(list, 0);
        // printlist(list, 1);
    }

    char *name = ML_Pop(list);
    while(name != NULL){
        printf("%s\n", name);
        free(name);
        name = ML_Pop(list);
    }

    ML_Destroy(list);

    return 0; */
}
