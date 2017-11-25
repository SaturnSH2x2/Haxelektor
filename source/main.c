#include <stdlib.h>
#include <stdio.h>
#include <3ds.h>

#include "pp2d/pp2d.h"

#include "ui.h"
#include "filestuff.h"
#include "titleselect.h"

// used this as a reference:  https://github.com/devkitPro/3ds-examples/blob/master/app_launch/source/main.c
void launchGame(u64 tid) {
    return;
}

void hang() {
    while (aptMainLoop()) {
        hidScanInput();
        
        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }
}

int main() {
    char* tidStr;
    u64 tidInt;
    LOOP_RETURN result;
    Result res;
    
    pp2d_init();
    romfsInit();
    
    tidStr = selectTitle();
    tidInt = atoi(tidStr);
    
    uiInit("00040000000EDF00");
    result = uiModSelectLoop();
    if (result != LAUNCH_GAME)
        return 0;
    
    //uiExit();
    tidInt = 0x0004001000021400LL;   // USA Camera
    romfsExit();
    pp2d_exit();
    
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);

    // wait for the app to launch
    u8 param[0x300];
    u8 hmac[0x20];
    
    //memset(param, 0, sizeof(param));
    //memset(hmac, 0, sizeof(hmac));
    
    printf("launching %llx\n", tidInt);
    while (aptMainLoop()) {
        hidScanInput();
        if (hidKeysDown())
            break;
    }
    
    res = APT_PrepareToDoApplicationJump(0, tidInt, 0);
    printf("APT_PrepareToDoApplicationJump result: %ld\n", res);
    res = APT_DoApplicationJump(param, sizeof(param), hmac);
    printf("APT_DoApplicationJump result: %ld\n", res);
    hang();

	return 0;
}
