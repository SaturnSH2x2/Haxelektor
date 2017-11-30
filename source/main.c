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
        continue;
    }
}

int main() {
    char* tidStr;
    char* endptr = NULL;
    u64 tidInt;
    LOOP_RETURN result;
    
    pp2d_init();
    romfsInit();
    
    tidStr = selectTitle();
    tidInt = strtoll(tidStr, &endptr, 16);
    
    uiInit(tidStr);
    result = uiModSelectLoop();
    if (result != LAUNCH_GAME)
        return 0;

    romfsExit();
    pp2d_exit();

    // wait for the app to launch
    u8 param[0x300];
    u8 hmac[0x20];
    
    // TODO: perhaps include some way to boot for cartridge-type games?
    APT_PrepareToDoApplicationJump(0, tidInt, MEDIATYPE_SD);
    APT_DoApplicationJump(param, sizeof(param), hmac);
    hang();

	return 0;
}
