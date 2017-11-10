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
#define NUMBTNS 5

// ------------------------------------------------------
// UI Mod Select Variables
// ------------------------------------------------------

// globals, probably important
UIButton** buttonList;
    // 0 -> Launch Game
    // 1 -> Save Config
    // 2 -> Load Config
    // 3 -> Go Back

// required for listing files in mod select
char** modListing;
int modCount;
u16* imageList;
u8* modSelected;

s16 entryIndex = 0;  // position of index in array
u16 indexPos = 0;    // position of screen in array

// END
// ------------------------------------------------------


// for debugging
void stall() {
    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        if (kDown)
            break;
        
        gfxFlushBuffers();
        gfxSwapBuffers();
    }
}
    
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
            snprintf(but->text, MAXSIZE, "Apply Patches");
            break;
        case 2:
            snprintf(but->text, MAXSIZE, "Save Config");
            break;
        case 3:
            snprintf(but->text, MAXSIZE, "Load Config");
            break;
        case 4:
            snprintf(but->text, MAXSIZE, "Go Back");
            break;
        default:
            snprintf(but->text, MAXSIZE, "whoops, that wasn't supposed to happen");
            break;
    }
}

void drawButton(UIButton* but) {
    int textWidth = pp2d_get_text_width(but->text, 0.5f, 0.5f);
    
    pp2d_draw_rectangle(but->x, but->y, but->width, but->height, GREYFG);
    pp2d_draw_text(but->x + (but->width / 2) - (textWidth / 2), but->y + (but->height / 3), 0.5f, 0.5f, WHITE, but->text);
}

void createModEntry(GameMod* mod, char* name, char* description, int imgID) {
    snprintf(mod->name, MAXSIZE, name);
    snprintf(mod->description, MAXSIZE, description);
    
    mod->imgID = imgID;
    mod->isSelected = 0;
}

void uiInit() {
    // create them buttons
    buttonList = malloc(NUMBTNS * sizeof(*buttonList));
    for (int i = 0; i < NUMBTNS; i++) {
        buttonList[i] = malloc(sizeof(UIButton*));
        createButton(buttonList[i], 220, 10 + (i * 35), 95, 27, i);
    }
    
    // Nothing like HARDCODING SHIT IN WHEN IT WON'T WORK FOR SOME STUPID-ASS FUCKING REASON OTHERWISE
    buttonList[0]->height = 30;
    buttonList[1]->height = 30;
    buttonList[2]->height = 30;
    buttonList[3]->height = 30;
    buttonList[4]->height = 30;
    
    // create and fill mod list
    modListing = listAllFiles("/3ds/data/Haxelektor/test/", &modCount);
    
    imageList = malloc(modCount * sizeof(u16*));
    modSelected = malloc(modCount * sizeof(u8*));
    
    for (int i = 0; i < modCount; i++) {
        modSelected[i] = 0;
        imageList[i] = 0;
    }
}

LOOP_RETURN uiModSelectLoop() {
    pp2d_set_screen_color(GFX_TOP, LGREYBG);
    pp2d_set_screen_color(GFX_BOTTOM, LGREYBG);
    
    // debug shit
    consoleInit(GFX_TOP, NULL);
    
    for (int i = 0; i < modCount; i++)
        printf("%s\n", modListing[i]);
    stall();

    while (aptMainLoop()) {
        int add = 0;
        
        // read input
        hidScanInput();
        u32 kDown = hidKeysDown();
        
        // react according to input
        if (kDown & KEY_START)
            break;
        
        switch (kDown) {
            case KEY_UP:
                entryIndex--;
                break;
            case KEY_DOWN:
                entryIndex++;
                add = 1;
                break;
            case KEY_RIGHT:
                entryIndex = entryIndex + 13;
                if ((entryIndex >= modCount) && indexPos != ((int)ceil((float)modCount / 13.0) - 1) * 13) {
                    entryIndex = modCount - 1;
                }
                indexPos = indexPos + 13;
                break;
            case KEY_LEFT:
                entryIndex = entryIndex - 13;
                indexPos = indexPos - 13;
                break;
            default:
                break;
        };
        
        if (entryIndex >= modCount) {
            entryIndex = 0;
            indexPos = 0;
        } else if (entryIndex < 0) {
            entryIndex = modCount - 1;
            indexPos = ((int)ceil((float)modCount / 13.0) - 1) * 13;
        }
        
        if (kDown) {
            if (entryIndex < indexPos)
                indexPos = indexPos - 13;
            else if ((entryIndex % 13) == 0 && entryIndex != 0 && add == 1)
                indexPos = indexPos + 13;
        }

        printf("%d\n", indexPos);
        
        // display to screen
        pp2d_begin_draw(GFX_BOTTOM);
            // draw mod entries
            int max = (indexPos + 13 < modCount) ? indexPos + 13 : modCount;
            
            for (int i = indexPos; i < max; i++) {
                if (i == entryIndex)
                    pp2d_draw_rectangle(0, 20 + ((i % 13) * 15), 320, 15, GREYFG);
                pp2d_draw_text(0, 20 + ((i % 13) * 15), 0.5f, 0.5f, WHITE, modListing[i]);
                pp2d_draw_text(50, 20 + ((i % 13) * 15), 0.5f, 0.5f, WHITE, "aaaabbbb");
            }
        
            // draw side and bottom bar
            pp2d_draw_rectangle(0, 0, 320, 20, GREYFG);
            pp2d_draw_rectangle(210, 0, 120, 240, GREY2FG);
            pp2d_draw_rectangle(0, 220, 320, 220, DGREY);
        
            // draw all the side buttons
            for (int i = 0; i < NUMBTNS; i++) {
                drawButton(buttonList[i]);
            }
        pp2d_end_draw();
    }
    
    return GO_BACK;
}

void uiExit() {
    free(modListing);  
    free(buttonList);
    free(imageList);
    free(modSelected);
}