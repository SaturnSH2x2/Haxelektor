#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <3ds.h>

#include "ui.h"
#include "titleselect.h"
#include "pp2d/pp2d.h"

#define MAXSIZE 500

/*
    A good amount of the code seen here was taken from Checkpoint.
    In particular, these files:
        https://github.com/BernardoGiordano/Checkpoint/blob/master/source/smdh.cpp
        https://github.com/BernardoGiordano/Checkpoint/blob/master/source/title.cpp
*/

Handle fsSession;

int region = 1;  // English by default, TODO: have it so that the program reads the 3DS's Region and sets it accordingly

char** descriptions;
char** publishers;

int getSMDHData(u64 tid, u8 mediaType, int index) {
    // load smdh struct into memory
    Handle fileHandle;
    u32 high = (u32)(tid >> 32);
    
    u32 archPath[] = {(u32)tid, high, mediaType, 0x0};
    static const u32 filePath[] = {0x0, 0x0, 0x2, 0x6E6F6369};
    
    FS_Path binArchPath = {PATH_BINARY, 0x10, archPath};
    FS_Path binFilePath = {PATH_BINARY, 0x14, filePath};
    
    smdhStruct* smdh = malloc(sizeof(smdhStruct));
    
    Result res = FSUSER_OpenFileDirectly(&fileHandle, ARCHIVE_SAVEDATA_AND_CONTENT, binArchPath, binFilePath, FS_OPEN_READ, 0);
    if (!res) {
        free(smdh);
        smdh = NULL;
        return -1;
    }
    
    u32 read;
    FSFILE_Read(fileHandle, &read, 0, smdh, sizeof(smdhStruct));
    FSFILE_Close(fileHandle);
    
    // get short description and publishers
    memset(descriptions[index], 0, MAXSIZE * sizeof(char*));
    snprintf(descriptions[index], 0x80, "%s", (char*)smdh->appTitles[region].shortDescription);
    
    memset(publishers[index], 0, MAXSIZE * sizeof(char*));
    snprintf(publishers[index], 0x80, "%s", (char*)smdh->appTitles[region].publisher);
    
    // get icon data
    u32 width = 48, height = 48;
    u32* img = (u32*)malloc(width * height * sizeof(u32));
    
    for (u32 x = 0; x < width; x++) {
        for (u32 y = 0; y < height; y++) {
            unsigned int destPixel = (x + y * width);
            unsigned int sourcePixel = (((y >> 3) * (width >> 3) + (x >> 3)) << 6) + ((x & 1) | ((y & 1) << 1) | ((x & 2) << 1) | ((y & 2) << 2) | ((x & 4) << 2) | ((y & 4) << 3));
            
            u8 r = ((smdh->bigIconData[sourcePixel] >> 11) & 0b11111) << 3;
            u8 g = ((smdh->bigIconData[sourcePixel] >> 5) & 0b111111) << 2;
            u8 b = ((smdh->bigIconData[sourcePixel]) & 0b11111) << 3;
            
            img[destPixel] = (r << 24) | (g << 16) | (b << 8) | 0xff;
        }
    }
    
    pp2d_load_texture_memory(index, (u8*)img, (u32)width, (u32)height);
    free(img);
    
    free(smdh);
    smdh = NULL;
    
    return 0;
}

u32 selectTitle() {
    uiError("Stubbed.  Proceed to mod selection screen.");
    return 0x0004000000175E00;
}