#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <3ds.h>
#include <errno.h>

#include "filestuff.h"
#include "ui.h"

#define MAXDIRSIZE 300

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
            contentFolder = NULL;
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
    FILE* s;
    FILE* d;
    
    consoleInit(GFX_BOTTOM, NULL);
    s = fopen(src, "rb");
    d = fopen(dst, "w+");
    if (s == NULL || d == NULL) {
        return -1;
    }
    
    buf = fgetc(s);
    do {
        fputc(buf, d);
        buf = fgetc(s);
    } while (buf != EOF);
    
    fclose(s);
    fclose(d);
    
    return 0;
}

int copyDir(const char* src, const char* dst) {
    // first off, does it exist and is it a directory?
    consoleInit(GFX_BOTTOM, NULL);
    DIR* curdir = opendir(src);
    if (curdir == NULL) {
        printf("error: 0x%x\n", errno);
        return -1;
    }
        
    // check to see that the destination directory exists,
    // if it doesn't, create it
    DIR* dstdir = opendir(dst);
    if (dstdir == NULL) {
        mkdir(dst, 0777);
    } else {
        closedir(dstdir);
    }

    struct dirent* entry;
    char* srcSubPath = malloc(MAXDIRSIZE * sizeof(char*));
    char* dstSubPath = malloc(MAXDIRSIZE * sizeof(char*));

    while ( (entry = readdir(curdir)) != NULL ) {
        // copy the names
        snprintf(srcSubPath, MAXDIRSIZE, "%s/%s", src, entry->d_name);
        snprintf(dstSubPath, MAXDIRSIZE, "%s/%s", dst, entry->d_name);

        if ( strncmp(entry->d_name, ".", sizeof(entry->d_name)) == 0 || \
             strncmp(entry->d_name, "..", sizeof(entry->d_name)) == 0 ) {
            continue;
        }

        // yay, recursion, end me
        if (copyDir(srcSubPath, dstSubPath) == -1) {  // it's a file, we need to copy it as such
            if (copyFile(srcSubPath, dstSubPath) == -1) {
                return -1;
            }
        }
    }

    free(srcSubPath);
    free(dstSubPath);

    closedir(curdir);
    
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
