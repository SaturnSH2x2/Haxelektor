#include <stdlib.h>
#include <stdio.h>
#include <3ds.h>

#include "pp2d/pp2d.h"

#include "ui.h"
#include "filestuff.h"

int main() {
    pp2d_init();
    
    uiInit();
    uiModSelectLoop();
    
    uiExit();
    pp2d_exit();
	return 0;
}
