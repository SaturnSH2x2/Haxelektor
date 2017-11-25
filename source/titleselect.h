#ifndef TITLESELECT_H
#define TITLESELECT_H

// this was helpful:  https://www.3dbrew.org/wiki/SMDH
// this was also helpful: https://github.com/BernardoGiordano/Checkpoint/blob/master/include/smdh.h

typedef struct {
    u16 shortDescription[0x40];
    u16 longDescription[0x80];
    u16 publisher[0x40];
} smdhAppTitle;

typedef struct {
    u8 gameRating[0x10];
    u32 regionLockout;
    u8 matchMakerIDs[0xC];
    u32 flags;
    u16 eula;
    u32 optimalDefaultFrame;
    u32 cecID;
} smdhAppSettings;

typedef struct {
    // header
    u32 magic;
    u16 version;
    u16 reserved;
    
    // 16 app title structs, one for each region
    smdhAppTitle appTitles[16];
    
    // app settings
    smdhAppSettings appSettings;
    u8 reserved2[0x8];
    
    // icon data
    u8 smallIconData[0x480];
    u16 bigIconData[0x900];
} smdhStruct;

char* selectTitle();

#endif