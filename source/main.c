#include <stdlib.h>
#include <stdio.h>
#include <3ds.h>

#include "pp2d/pp2d.h"

#include "ui.h"
#include "filestuff.h"

int main() {
    pp2d_init();
    romfsInit();
    //consoleInit(GFX_TOP, NULL);
    
    uiInit("0004000000175E00");
    uiModSelectLoop();
    
    //uiExit();
    romfsExit();
    pp2d_exit();
	return 0;
}
