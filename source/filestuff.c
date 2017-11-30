#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <3ds.h>

#include "filestuff.h"
#include "ui.h"

#define MAXDIRSIZE 10000

// TODO: add code.bin/code.ips support
bool isAMod(const char* path) {
    DIR* moddir = opendir(path);
    struct dirent* entry;
    
    DIR* contentFolder;
    char* subpath = malloc(MAXDIRSIZE * sizeof(char*));
    
    while ((entry = readdir(moddir)) != NULL) {
        memset(subpath, 0, MAXDIRSIZE * sizeof(char*));
        snprintf(subpath, MAXDIRSIZE, "%s/%s", path, entry->d_name);
        if ((contentFolder = opendir(subpath)) != NULL) { 
            closedir(contentFolder);
            if (strncmp(entry->d_name, "mod", sizeof(entry->d_name)) == 0)
                return true;
        }
    }
    
    free(subpath);
    closedir(moddir);
    return false;
}

char** listAllFiles(const char* path, u16* entryC, int listOnlyMods) {
    char** entries;
    int arrSize = 2;
    int arrIndex = 0;
    entries = malloc(arrSize * sizeof(*entries));
    if (entries == NULL) {
        printf("failed to allocate memory\n");
        return NULL;
    }
    
    struct dirent* dEnt;
    DIR* d = opendir(path);
    if ( d == NULL )
        return NULL;
    
    char* subpath = malloc(MAXDIRSIZE * sizeof(subpath));
    
    while ( (dEnt = readdir(d)) != NULL ) {
        if (listOnlyMods) {
            memset(subpath, 0, MAXDIRSIZE * sizeof(subpath));
            snprintf(subpath, MAXDIRSIZE, "%s/%s", path, dEnt->d_name);
            if (!isAMod(subpath))
                continue;
        }
        
        entries[arrIndex] = malloc(MAXDIRSIZE * sizeof(char*));
        snprintf(entries[arrIndex], MAXDIRSIZE, dEnt->d_name);
        
        arrIndex++;
        if (arrIndex >= arrSize) {
            arrSize++;
            entries = realloc(entries, arrSize * sizeof(*entries));
            if (entries == NULL) {
                printf("failed to allocate memory\n");
                return NULL;
            }
        }
    }
    
    if (arrIndex == 0)
        return NULL;
    
    free(subpath);
    closedir(d);
    arrSize--;
    
    *entryC = arrIndex;
    
    return entries;
}

int copyFile(const char* src, const char* dst) {
    int buf;
    char* createDirBuf;
    FILE* s;
    FILE* d;
    
    copyFileRetry:
    s = fopen(src, "rb");
    d = fopen(dst, "wb");
    if (s == NULL || d == NULL) {
        createDirBuf = malloc(MAXDIRSIZE * sizeof(char*));
        for (int i = (sizeof(dst) / sizeof(char*)) - 1; i > 0; i--) {
            uiError("preparing to rename directory");
            if (dst[i] == '/') {
                snprintf(createDirBuf, i, "%s", dst);
                mkdir(createDirBuf, 0777);
                uiError("went back, created directory, retrying");
                goto copyFileRetry;
            }
        }
        printf("source: %s, dest: %s\n", src, dst);
        return -1;
    }
    
    while ((buf = fgetc(s)) != EOF) {
        fputc(buf, d);
    }
    
    fclose(s);
    fclose(d);
    
    return 0;
}

int copyDir(const char* src, const char* dst) {
    // first off, does it exist and is it a directory?
    DIR* curdir = opendir(src);
    if (curdir == NULL) {
        //uiError("source directory does not exist");
        //uiError(src);
        return -1;
    }
        
    // check to see that the destination directory exists,
    // if it doesn't, create it
    DIR* dstdir = opendir(dst);
    if (dstdir == NULL) {
        // https://stackoverflow.com/a/675051
        int result = mkdir(dst, 0777);
        if (result == -1) {
            //uiError("failed to create directory");
            //uiError(dst);
            return -1;
        }
    } else {
        closedir(dstdir);
    }

    struct dirent* entry;
    char* srcSubPath = malloc(MAXDIRSIZE);
    char* dstSubPath = malloc(MAXDIRSIZE);

    while ( (entry = readdir(curdir)) != NULL ) {
        // copy the names
        // while, yes, there is a possiblility that this can get truncated 
        snprintf(srcSubPath, MAXDIRSIZE, "%s/%s", src, entry->d_name);
        snprintf(dstSubPath, MAXDIRSIZE, "%s/%s", dst, entry->d_name);

        if ( strncmp(entry->d_name, ".", sizeof(entry->d_name)) == 0 || \
             strncmp(entry->d_name, "..", sizeof(entry->d_name)) == 0 ) {
            continue;
        }

        // yay, recursion, end me
        if (copyDir(srcSubPath, dstSubPath) == -1) {  // it's a file, we need to copy it as such
            if (copyFile(srcSubPath, dstSubPath) == -1) {
                //uiError("failed to copy file");
                //uiError(srcSubPath);
                //uiError(entry->d_name);
                return -1;
            }
        }
    }

    free(srcSubPath);
    free(dstSubPath);

    return 0;
}

int removeDir(char* path) {
    DIR* dir = opendir(path);
    if (dir == NULL)
        return 0;
    
    struct dirent* entry;
    char* strPath = malloc(MAXDIRSIZE * sizeof(char*));
    while ((entry = readdir(dir)) != NULL) {
        if (strncmp(".", entry->d_name, sizeof(entry->d_name) == 0) || \
            strncmp("..", entry->d_name, sizeof(entry->d_name) == 0))
            continue;
            
        memset(strPath, 0, MAXDIRSIZE * sizeof(char*));
        snprintf(strPath, MAXDIRSIZE, "%s/%s", path, entry->d_name);
        
        if (remove(strPath) != 0) {
            removeDir(strPath);
            if (rmdir(strPath) != 0) {
                return -1;
            }
        }
    }
    
    free(strPath);
    closedir(dir);
    
    return 0;
}
