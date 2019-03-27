#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include "mydir.h"

MDIRENT MyDirEnt_Create(char *parent, struct dirent *ent){
    MDIRENT md = (MDIRENT)malloc(sizeof(struct MyDirEnt));
    if(md == NULL){
        fprintf(stderr, "Out of memory\n");
        return NULL;
    }

    int len = strlen(parent) + 1;
    md->parent = (char *)malloc(len);
    if(md->parent == NULL){
        free(md);
        fprintf(stderr, "Out of memory\n");
        return NULL;
    }

    strcpy(md->parent, parent);
    md->ent = (struct dirent *)malloc(sizeof(struct dirent));
    if(md->ent == NULL){
        free(md->parent);
        free(md);
        fprintf(stderr, "Out of memory\n");
        return NULL;
    }

    strcpy(md->ent->d_name, ent->d_name);
    md->ent->d_type = ent->d_type;

    return md;
}

char *MyDirEnt_Path(MDIRENT md){
    int len = strlen(md->parent) + strlen(md->ent->d_name) + 2;
    char *path = (char *)malloc(len);
    if(path == NULL){;
        fprintf(stderr, "Out of memory\n");
        return NULL;
    }

    sprintf(path, "%s/%s", md->parent, md->ent->d_name);
    return path;
}

void MyDirEnt_Destory(MDIRENT md){
    free(md->ent);
    free(md->parent);
    free(md);
}
