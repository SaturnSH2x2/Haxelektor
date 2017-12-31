#ifndef FILESTUFF_H
#define FILESTUFF_H

typedef enum {
    CANNOT_OPEN_DIRECTORY = -1,
    CANNOT_COPY_INSTANCE = -2
} fileErrors;

char** listAllFiles(const char* path, u16* entryC, int listOnlyMods);
int copyFile(const char* src, const char* dst);
int copyDir(const char* src, const char* dst);
int removeDir(char* path);

#endif
