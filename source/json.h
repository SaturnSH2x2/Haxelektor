#ifndef JSON_H
#define JSON_H

int jsonLoad(char* path, s16 selCount, char** selections, u8* selectArray);
int jsonSave(char* path, s16 selCount, char** selections, u8* selectArray);

#endif