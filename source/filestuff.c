#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <3ds.h>

#include "filestuff.h"

#define MAXDIRSIZE 255


char** listAllFiles(const char* path, int* entryC) {
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
    
    while ( (dEnt = readdir(d)) != NULL ) {
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
    
    closedir(d);
    arrSize--;
    
    *entryC = arrIndex;
    
    return entries;
}

int copyFile(const char* src, const char* dst) {
    int buf;
    
    FILE* s = fopen(src, "rb");
    FILE* d = fopen(dst, "wb+");
    if (s == NULL || d == NULL) {
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
    if (curdir == NULL)
        return -1;

    // check to see that the destination directory exists,
    // if it doesn't, create it
    DIR* dstdir = opendir(dst);
    if (dstdir == NULL) {
        // https://stackoverflow.com/a/675051
        int result = mkdir(dst, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if (result == -1) {
            fprintf(stderr, "error: could not create %s\n", dst);
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
                fprintf(stderr, "error: could not copy %s\n", entry->d_name);
                return -1;
            }
        }
    }

    free(srcSubPath);
    free(dstSubPath);

    return 0;
}
