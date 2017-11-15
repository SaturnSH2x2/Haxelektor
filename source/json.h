#ifndef JSON_H
#define JSON_H

int jsonLoad(char* path, int selCount, char** selections, int* selectArray);
int jsonSave(char* path, int selCount, char** selections, int* selectArray);

#endif