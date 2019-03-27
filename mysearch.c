#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <string.h>
#include <errno.h>
#include "mysearch.h"
#include "mydir.h"
#include "mylist.h"

MSEARCHOPTS MySearchOptions_Create(){
    MSEARCHOPTS mso = (MSEARCHOPTS)malloc(sizeof(struct MySearchOptions));
    if(mso == NULL){
        fprintf(stderr, "Out of memory\n");
        return NULL;
    }

    mso->name = NULL;
    mso->preg = NULL;

    return mso;
}

void MySearch_Action_Print(MSEARCHOPTS options, MDIRENT ent){
    char *path = MyDirEnt_Path(ent);
    if(path == NULL)
        return;

    printf("%s\n", path);
    free(path);
}

int MySearch_Test(MSEARCHOPTS options, MDIRENT ent){
    int result = 0;
    //文件名过滤
    if(options->name != NULL){
        //按正则表达式匹配
        if(options->preg != NULL){
            regmatch_t pmatch[1];
            if(regexec(options->preg, ent->ent->d_name, 1, pmatch, 0) == 0){
                result = 1;
            }
        }else{
            //按文件名匹配
            if(strstr(ent->ent->d_name, options->name) != NULL){
                result = 1;
            }
        }

        if(result == 0)
            return 0;
    }

    //类型过滤
    if(options->mode != 0){
        if((options->mode & S_IFDIR) == S_IFDIR && ent->ent->d_type == DT_DIR){
            result = 1;
        }
        if((options->mode & S_IFREG) == S_IFREG && ent->ent->d_type == DT_REG){
            result = 1;
        }

        if(result == 0)
            return 0;
    }

    return 1;
}

int MySearchOptions_PregCompile(MSEARCHOPTS options){
    if(options->name == NULL)
        return 0;
        
    if(options->preg != NULL){
        regfree(options->preg);
    }

    options->preg = (regex_t *)malloc(sizeof(regex_t));
    int result = regcomp(options->preg, options->name, REG_EXTENDED);
    if(result != 0){
        regfree(options->preg);
    }

    return result;
}

void MySearch_Search(MSEARCHOPTS options, void(*action)(MSEARCHOPTS, MDIRENT), char *dir){    
    DIR *pDir = opendir(dir);
    if(pDir == NULL){
        int err = errno;
        fprintf(stderr, "%s: %s\n", "/usr", strerror(err));
        return;
    }

    ML list = ML_Create();
    struct dirent *ent = readdir(pDir);
    while(ent != NULL){
        if(strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0){
            ent = readdir(pDir);
            continue;
        }

        MDIRENT md = MyDirEnt_Create(dir, ent);
        if(md == NULL){
            ML_Destroy(list);
            return;
        }

        ML_Append(list, md);
        ent = readdir(pDir);
    }

    closedir(pDir);

    MDIRENT md = ML_Pop(list);
    while(md != NULL){
        int result = MySearch_Test(options, md);
        if(result != 0){
            action(options, md);
        }

        //读取目录
        if(md->ent->d_type == DT_DIR){
            char *path = MyDirEnt_Path(md);
            if(path == NULL){
                MyDirEnt_Destory(md);
                md = ML_Pop(list);
                continue;
            }

            pDir = opendir(path);
            if(pDir == NULL){
                int err = errno;
                fprintf(stderr, "%s: %s\n", path, strerror(err));
                free(path);
                MyDirEnt_Destory(md);
                md = ML_Pop(list);
                continue;
            }

            struct dirent *pDirEnt = readdir(pDir);
            while(pDirEnt != NULL){
                if(strcmp(pDirEnt->d_name, ".") == 0 || strcmp(pDirEnt->d_name, "..") == 0){
                    pDirEnt = readdir(pDir);
                    continue;
                }

                MDIRENT child = MyDirEnt_Create(path, pDirEnt);
                if(child == NULL)
                    continue;

                ML_Append(list, child);
                pDirEnt = readdir(pDir);
            }
            closedir(pDir);
            free(path);
        }

        MyDirEnt_Destory(md);
        md = ML_Pop(list);
    }

    ML_Destroy(list);
}

int MySearch_BuildOptions(MSEARCHOPTS options, int argc, char *argv[]){
    for(int i=0; i<argc; i++){
        char *arg = argv[i];
        if(strcmp(arg,"-name") == 0){
            if(i + 1 >= argc){
                return MS_ENAME;
            }

            options->name = argv[i+1];
        }else if(strcmp(arg, "-type") == 0){
            if(i + 1 >= argc){
                return MS_ETYPE;
            }

            for(int j=0; j<strlen(argv[i+1]); j++){
                switch(argv[i+1][j]){
                    case 'f':
                        options->mode = options->mode | S_IFREG;
                    break;
                    case 'd':
                        options->mode = options->mode | S_IFDIR;
                    break;
                }
            }
        }else if(strcmp(arg, "-dir") == 0){
            if(i + 1 >= argc){
                return MS_EDIR;
            }

            options->dir = argv[i+1];
        }
    }

    options->preg = (regex_t *)malloc(sizeof(regex_t));
    if(options->preg == NULL)
        return MS_EREG;
    
    return MySearchOptions_PregCompile(options);
}

void MySearchOptions_Destory(MSEARCHOPTS mso){
    if(mso->preg != NULL){
        regfree(mso->preg);
    }

    free(mso);
}
