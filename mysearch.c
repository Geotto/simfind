#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
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
    mso->mode_include = MS_MODE_NONE;
    mso->mode_exclude = MS_MODE_NONE;

    return mso;
}

void MySearch_Action_Print(MSEARCHOPTS options, MDIRENT ent){
    char *path = MyDirEnt_Path(ent);
    if(path == NULL)
        return;

    printf("%s\n", path);
    free(path);
}

int MySearch_Merge(int include, int exclude){
    int result = 0;
    for(int i=0; i<10; i++){
        int mask = 1 << i;
        int in = include & mask;
        int ex = exclude & mask;
        if(in != 0 && ex == 0){
            result = result | mask;
        }
    }

    return result;
}

int MySearch_Test(MSEARCHOPTS options, MDIRENT ent){
    int result = 1;
    //文件名过滤
    if(options->name != NULL){
        result = 0;

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

    struct stat * statbuf = NULL;

    //类型过滤
    if(options->mode != 0){
        result = 0;
        
        if((options->mode & S_IFDIR) == S_IFDIR && ent->ent->d_type == DT_DIR){
            result = 1;
        }
        if((options->mode & S_IFREG) == S_IFREG && ent->ent->d_type == DT_REG){
            result = 1;
        }

        if(result == 0)
            return 0;
    }

    if(options->uid != 0){
        result = 0;
        char *path = MyDirEnt_Path(ent);
        if(path == NULL){
            return 0;
        }

        if(statbuf == NULL){
            statbuf = MySearch_GetStatBuf(path);
            if(statbuf == NULL){
                free(path);
                return 0;
            }
        }

        if(statbuf->st_uid == options->uid){
            result = 1;
        }else{
            result = 0;
        }

        free(path);
        if(result == 0)
            return 0;
    }

    if(options->gid != 0){
        result = 0;
        char *path = MyDirEnt_Path(ent);
        if(path == NULL){
            return 0;
        }

        if(statbuf == NULL){
            statbuf = MySearch_GetStatBuf(path);
            if(statbuf == NULL){
                free(path);
                return 0;
            }
        }

        if(statbuf->st_gid == options->gid){
            result = 1;
        }else{
            result = 0;
        }

        free(path);
        if(result == 0)
            return 0;
    }

    if(options->mode_include != MS_MODE_NONE){
        result = 0;
        if(statbuf == NULL){
            char *path = MyDirEnt_Path(ent);
            if(path == NULL)
                return 0;

            statbuf = MySearch_GetStatBuf(path);
            if(statbuf == NULL){
                free(path);
                return 0;
            }
        }

        result = MySearch_CheckMode(MySearch_Merge(options->mode_include, options->mode_exclude), statbuf->st_mode);
        if(result == 0)
            return 0;
    }

    if(statbuf != NULL){
        free(statbuf);
    }

    return result;
}

int MySearch_CheckMode(int mode_expect, mode_t mode_actual){
    int mode = 0;

    if((mode_actual & S_IXOTH) == S_IXOTH){
        mode = mode | MS_MODE_OX;
    }
    
    if((mode_actual & S_IWOTH) == S_IWOTH){
        mode = mode | MS_MODE_OW;
    }
    
    if((mode_actual & S_IROTH) == S_IROTH){
        mode = mode | MS_MODE_OR;
    }
    
    if((mode_actual & S_IXGRP) == S_IXGRP){
        mode = mode | MS_MODE_GX;
    }
    
    if((mode_actual & S_IWGRP) == S_IWGRP){
        mode = mode | MS_MODE_GW;
    }
    
    if((mode_actual & S_IRGRP) == S_IRGRP){
        mode = mode | MS_MODE_GR;
    }
    
    if((mode_actual & S_IXUSR) == S_IXUSR){
        mode = mode | MS_MODE_UX;
    }
    
    if((mode_actual & S_IWUSR) == S_IWUSR){
        mode = mode | MS_MODE_UW;
    }
    
    if((mode_actual & S_IRUSR) == S_IRUSR){
        mode = mode | MS_MODE_UR;
    }

    int mask = 0;
    if((mode_expect & MS_MODE_FLAGA) == MS_MODE_FLAGA){
        mask = mask | 511;
    }else if((mode_expect & MS_MODE_FLAGU) == MS_MODE_FLAGU){
        mask = mask | 448;
    }else if((mode_expect & MS_MODE_FLAGG) == MS_MODE_FLAGG){
        mask = mask | 56;
    }else if((mode_expect & MS_MODE_FLAGO) == MS_MODE_FLAGO){
        mask = mask | 7;
    }

    return (mode & mask) == (mode_expect & mask);
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
    struct option opts[] = {
        { "dir", 1, NULL, 'd' },
        { "type", 1, NULL, 't' },
        { "name", 1, NULL, 'n'},
        {  "uid", 1, NULL, 'u' },
        {  "gid", 1, NULL, 'g' },
        { "mode", 1, NULL, 'm'},
        { "unmode", 1, NULL, 'M'}
    };

    int opt;
    while((opt = getopt_long(argc, argv, "d:t:n:u:g:m:M:", opts, NULL)) != -1){
        switch(opt){
            case 'd':
            options->dir = optarg;
            break;

            case 't':
            for(int i=0; i<strlen(optarg); i++){
                switch(optarg[i]){
                    case 'f':
                        options->mode = options->mode | S_IFREG;
                    break;
                    case 'd':
                        options->mode = options->mode | S_IFDIR;
                    break;
                }
            }
            break;

            case 'n':
            options->name = optarg;
            break;

            case 'u':
            errno = 0;
            long int uid = strtol(optarg, NULL, 10);
            if(errno != 0){
                fprintf(stderr, "%s not a valid value\n", optarg);
                return MS_EUID;
            }
            options->uid = uid;
            break;

            case 'g':
            errno = 0;
            long int gid = strtol(optarg, NULL, 10);
            if(errno != 0){
                fprintf(stderr, "%s not a valid value\n", optarg);
                return MS_EGID;
            }
            options->gid = gid;
            break;

            case 'm':
            options->mode_include = MS_MODE_NONE;
            int pos = 0;
            for(int i=0; i<strlen(optarg); i++){
                char ch = optarg[i];
                if(ch >= '0' && ch <= '7'){
                    int value = ch - '0';
                    pos++;
                    switch(pos){
                        case 1:
                        options->mode_include = options->mode_include | (value << 6) | MS_MODE_FLAGU;
                        break;

                        case 2:
                        options->mode_include = options->mode_include | (value << 3) | MS_MODE_FLAGG;
                        break;

                        case 3:
                        options->mode_include = options->mode_include | value | MS_MODE_FLAGO;
                        break;
                    }
                }

                switch(ch){
                    case 'a':
                        pos = -1;
                        options->mode_include = options->mode_include | MS_MODE_FLAGA;
                        break;

                    case 'u':
                        pos = 1;
                        options->mode_include = options->mode_include | MS_MODE_FLAGU;
                        break;

                    case 'g':
                        pos = 2;
                        options->mode_include = options->mode_include | MS_MODE_FLAGG;
                        break;

                    case 'o':
                        pos = 3;
                        options->mode_include = options->mode_include | MS_MODE_FLAGO;
                        break;

                    case 'r':
                        switch(pos){
                            case -1:
                                options->mode_include = options->mode_include | MS_MODE_UR | MS_MODE_GR | MS_MODE_OR;
                                break;
                            case 1:
                                options->mode_include = options->mode_include | MS_MODE_UR;
                                break;
                            case 2:
                                options->mode_include = options->mode_include | MS_MODE_GR;
                                break;
                            case 3:
                                options->mode_include = options->mode_include | MS_MODE_OR;
                                break;
                        }
                    break;
                    case 'w':
                        switch(pos){
                            case -1:
                                options->mode_include = options->mode_include | MS_MODE_UW | MS_MODE_GW | MS_MODE_OW;
                                break;
                            case 1:
                                options->mode_include = options->mode_include | MS_MODE_UW;
                                break;
                            case 2:
                                options->mode_include = options->mode_include | MS_MODE_GW;
                                break;
                            case 3:
                                options->mode_include = options->mode_include | MS_MODE_OW;
                                break;
                        }
                    break;
                    case 'x':
                        switch(pos){
                            case -1:
                                options->mode_include = options->mode_include | MS_MODE_UX | MS_MODE_GX | MS_MODE_OX;
                                break;
                            case 1:
                                options->mode_include = options->mode_include | MS_MODE_UX;
                                break;
                            case 2:
                                options->mode_include = options->mode_include | MS_MODE_GX;
                                break;
                            case 3:
                                options->mode_include = options->mode_include | MS_MODE_OX;
                                break;
                        }
                    break;
                }
            }
            break;

            case 'M':
            options->mode_exclude = MS_MODE_NONE;
            pos = 0;
            for(int i=0; i<strlen(optarg); i++){
                char ch = optarg[i];
                if(ch >= '0' && ch <= '7'){
                    int value = ch - '0';
                    pos++;
                    switch(pos){
                        case 1:
                        options->mode_exclude = options->mode_exclude | (value << 6) | MS_MODE_FLAGU;
                        break;

                        case 2:
                        options->mode_exclude = options->mode_exclude | (value << 3) | MS_MODE_FLAGG;
                        break;

                        case 3:
                        options->mode_exclude = options->mode_exclude | value | MS_MODE_FLAGO;
                        break;
                    }
                }

                switch(ch){
                    case 'a':
                        pos = -1;
                        options->mode_exclude = options->mode_exclude | MS_MODE_FLAGA;
                        break;

                    case 'u':
                        pos = 1;
                        options->mode_exclude = options->mode_exclude | MS_MODE_FLAGU;
                        break;

                    case 'g':
                        pos = 2;
                        options->mode_exclude = options->mode_exclude | MS_MODE_FLAGG;
                        break;

                    case 'o':
                        pos = 3;
                        options->mode_exclude = options->mode_exclude | MS_MODE_FLAGO;
                        break;

                    case 'r':
                        switch(pos){
                            case -1:
                                options->mode_exclude = options->mode_exclude | MS_MODE_UR | MS_MODE_GR | MS_MODE_OR;
                                break;
                            case 1:
                                options->mode_exclude = options->mode_exclude | MS_MODE_UR;
                                break;
                            case 2:
                                options->mode_exclude = options->mode_exclude | MS_MODE_GR;
                                break;
                            case 3:
                                options->mode_exclude = options->mode_exclude | MS_MODE_OR;
                                break;
                        }
                    break;
                    case 'w':
                        switch(pos){
                            case -1:
                                options->mode_exclude = options->mode_exclude | MS_MODE_UW | MS_MODE_GW | MS_MODE_OW;
                                break;
                            case 1:
                                options->mode_exclude = options->mode_exclude | MS_MODE_UW;
                                break;
                            case 2:
                                options->mode_exclude = options->mode_exclude | MS_MODE_GW;
                                break;
                            case 3:
                                options->mode_exclude = options->mode_exclude | MS_MODE_OW;
                                break;
                        }
                    break;
                    case 'x':
                        switch(pos){
                            case -1:
                                options->mode_exclude = options->mode_exclude | MS_MODE_UX | MS_MODE_GX | MS_MODE_OX;
                                break;
                            case 1:
                                options->mode_exclude = options->mode_exclude | MS_MODE_UX;
                                break;
                            case 2:
                                options->mode_exclude = options->mode_exclude | MS_MODE_GX;
                                break;
                            case 3:
                                options->mode_exclude = options->mode_exclude | MS_MODE_OX;
                                break;
                        }
                    break;
                }
            }
            break;

            case ':':
            fprintf(stderr, "option %c need an argument\n",  optopt);
            return MS_ERROR;
            break;

            case '?':
            fprintf(stderr, "unkown option\n");
            break;
        }
    }

    options->preg = (regex_t *)malloc(sizeof(regex_t));
    if(options->preg == NULL)
        return MS_EREG;
    
    return MySearchOptions_PregCompile(options);
}

struct stat *MySearch_GetStatBuf(char *path){
    struct stat *statbuf = (struct stat *)malloc(sizeof(struct stat));
    if(statbuf == NULL){
        fprintf(stderr, "Out of memory\n");
        return NULL;
    }

    if(stat(path, statbuf) != 0){
        int err = errno;
        fprintf(stderr, "%s: %s\n", path, strerror(err));
        free(statbuf);
        return NULL;
    }

    return statbuf;
}

void MySearchOptions_Destory(MSEARCHOPTS mso){
    if(mso->preg != NULL){
        regfree(mso->preg);
    }

    free(mso);
}
