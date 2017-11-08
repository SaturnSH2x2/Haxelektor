#include <stdlib.h>
#include <3ds.h>

#include "pp2d/pp2d.h"
#include "filestuff.h"
#include "ui.h"

#define WHITE   RGBA8(255, 255, 255, 255)
#define GREYBG  RGBA8(255, 120, 120, 120)
#define LGREYBG RGBA8(255, 200, 200, 200)
#define GREYFG  RGBA8(120, 120, 120, 255)
#define GREY2FG RGBA8(80,   80,  80, 255)
#define DGREY   RGBA8( 50,  50,  50, 255)

#define MAXSIZE 255
#define NUMBTNS 4

// globals, probably important
UIButton** buttonList;
    // 0 -> Launch Game
    // 1 -> Save Config
    // 2 -> Load Config
    // 3 -> Go Back
    
void createButton(UIButton* but, int x, int y, int width, int height, int textIndex) {
    but->x = x;
    but->y = y;
    but->width = width;
    but->height = height;
    
    but->text = malloc(MAXSIZE * sizeof(char*));
    switch (textIndex) {
        case 0:
            snprintf(but->text, MAXSIZE, "Launch Game");
            break;
        case 1:
            snprintf(but->text, MAXSIZE, "Save Config");
            break;
        case 2:
            snprintf(but->text, MAXSIZE, "Load Config");
            break;
        case 3:
            snprintf(but->text, MAXSIZE, "Go Back");
            break;
        default:
            snprintf(but->text, MAXSIZE, "whoops, that wasn't supposed to happen");
            break;
    }
}

void drawButton(UIButton* but) {
    int textWidth = pp2d_get_text_width(but->text, 0.3f, 0.3f);
    
    pp2d_draw_rectangle(but->x, but->y, but->width, but->height, GREYFG);
    pp2d_draw_text(but->x + but->width / 2 - textWidth / 2, but->y, 0.3f, 0.3f, WHITE, but->text);
}

void uiInit() {
    // create them buttons
    buttonList = malloc(NUMBTNS * sizeof(*buttonList));
    for (int i = 0; i < NUMBTNS; i++) {
        buttonList[i] = malloc(sizeof(UIButton*));
        createButton(buttonList[i], 220, 20 + (i * 25), 80, 20, i);
    }
}

LOOP_RETURN uiModSelectLoop() {
    pp2d_set_screen_color(GFX_TOP, LGREYBG);
    pp2d_set_screen_color(GFX_BOTTOM, LGREYBG);
    consoleInit(GFX_TOP, NULL);

    while (aptMainLoop()) {
        // read input
        hidScanInput();
        u32 kDown = hidKeysDown();
        
        // react according to input
        if (kDown & KEY_START)
            break;

        // display to screen
        pp2d_begin_draw(GFX_BOTTOM);
            for (int i = 0; i < NUMBTNS; i++) {
                drawButton(buttonList[i]);
            }
        pp2d_end_draw();
    }
    
    return GO_BACK;
}

void uiExit() {
    for (int i = 0; i < NUMBTNS; i++)
        free(buttonList[i]);
    free(buttonList);
}