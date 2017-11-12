#ifndef FILESTUFF_H
#define FILESTUFF_H

char** listAllFiles(const char* path, u16* entryC, int listOnlyMods);
int copyFile(const char* src, const char* dst);
int copyDir(const char* src, const char* dst);
int removeDir(char* path);

#endif
