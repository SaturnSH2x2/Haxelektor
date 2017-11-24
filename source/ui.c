#include <stdlib.h>
#include <3ds.h>

#include <sys/stat.h>

#include "pp2d/pp2d.h"
#include "filestuff.h"
#include "ui.h"
#include "json.h"

#define WHITE   RGBA8(255, 255, 255, 255)
#define GREYBG  RGBA8(255, 120, 120, 120)
#define LGREYBG RGBA8(255, 200, 200, 200)
#define LGREYFG RGBA8(200, 200, 200, 255)
#define GREYFG  RGBA8(120, 120, 120, 255)
#define GREY2FG RGBA8(80,   80,  80, 255)
#define DGREY   RGBA8( 50,  50,  50, 255)

#define MAXSIZE 255
#define NUMBTNS 10

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
char** configListing;
char* currentTid;
u16 modCount;
u16 configCount;
u8* modSelected;

u8 isSaltySD = 0;
    // 0 -> LayeredFS
    // 1 -> SaltySD for Smash 4
    // 2 -> SaltySD for Pokemon S/M

float noWidth;

s16 entryIndex = 0;  // position of index in array
u16 indexPos = 0;    // position of screen in array
u8 cursorPos = 0;    // is the cursor in the file browser or the button list?
s16 configIndex = 0; // for 
s16 buttonIndex = 0;

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

void loadFromFile(char* desc) {
    char* path = malloc(MAXSIZE * sizeof(char*));
    
    snprintf(path, MAXSIZE, "/3ds/data/Haxelektor/%s/mods/%s/description.txt", currentTid, modListing[entryIndex]);
    FILE* fp = fopen(path, "r");
    if (fp == NULL) {
        snprintf(desc, MAXSIZE, "No description provided.");
        return;
    }
    
    fgets(desc, MAXSIZE, fp);
    fclose(fp);
    
    free(path);
}

// this is important, because mod priority and shit
void swapEntries(int index1, int index2) {
    char* listTemp;
    u8 selectTemp;
    
    listTemp = modListing[index1];
    selectTemp = modSelected[index1];
    
    modListing[index1] = modListing[index2];
    modSelected[index1] = modSelected[index2];
    
    modListing[index2] = (char*)listTemp;
    modSelected[index2] = selectTemp;
    
    pp2d_swap_textures(index1, index2);
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
            snprintf(but->text, MAXSIZE, "Remove Patches");
            break;
        case 3:
            snprintf(but->text, MAXSIZE, "Save Config");
            break;
        case 4:
            snprintf(but->text, MAXSIZE, "Load Config");
            break;
        case 5:
            snprintf(but->text, MAXSIZE, "Prio Up");
            break;
        case 6:
            snprintf(but->text, MAXSIZE, "<<");
            break;
        case 7:
            snprintf(but->text, MAXSIZE, ">>");
            break;
        case 8:
            snprintf(but->text, MAXSIZE, "Prio Down");
            break;
        case 9:
            snprintf(but->text, MAXSIZE, "Go Back");
            break;
        default:
            snprintf(but->text, MAXSIZE, "whoops, that wasn't supposed to happen");
            break;
    }
}

void drawButton(UIButton* but, bool selected) {
    u32 color;
    
    int textWidth = pp2d_get_text_width(but->text, 0.4f, 0.4f);
    if (selected)
        color = LGREYFG;
    else
        color = GREYFG;
    
    pp2d_draw_rectangle(but->x, but->y, but->width, but->height, color);
    pp2d_draw_text(but->x + (but->width / 2) - (textWidth / 2), but->y + (but->height / 3), 0.4f, 0.4f, WHITE, but->text);
}

bool isButtonTouched(UIButton* b, touchPosition* t) {
    return (b->x <= t->px && t->px <= b->x + b->width) && \
           (b->y <= t->py && t->py <= b->y + b->height);
}

s16 getCurrentButtonTouched(touchPosition* t) {
    for (int i = 0; i < NUMBTNS; i++) {
        if (isButtonTouched(buttonList[i], t)) 
            return i;
    }
        
    return -1;
}

void createModEntry(GameMod* mod, char* name, char* description, int imgID) {
    snprintf(mod->name, MAXSIZE, name);
    snprintf(mod->description, MAXSIZE, description);
    
    mod->imgID = imgID;
    mod->isSelected = 0;
}

void uiInit(char* tid) {
    // create them buttons
    buttonList = malloc(NUMBTNS * sizeof(*buttonList));
    for (int i = 0; i < NUMBTNS; i++) {
        buttonList[i] = malloc(sizeof(UIButton*));
        createButton(buttonList[i], 220, 10 + (i * 25), 95, 27, i);
    }
    
    noWidth = pp2d_get_text_width("No \uE001", 0.5f, 0.5f);
    
    // Nothing like HARDCODING SHIT IN WHEN IT WON'T WORK FOR SOME STUPID-ASS FUCKING REASON OTHERWISE
    buttonList[0]->height = 20;
    buttonList[1]->height = 20;
    buttonList[2]->height = 20;
    buttonList[3]->height = 20;
    buttonList[4]->height = 20;
    buttonList[5]->height = 20;
    
    buttonList[6]->width = 45;
    buttonList[6]->height = 20;
    buttonList[7]->width = 45;
    buttonList[7]->height = 20;
    
    buttonList[7]->x = 270;
    buttonList[7]->y = buttonList[6]->y;
    
    buttonList[8]->y = 185;
    buttonList[8]->height = 20;
    buttonList[9]->y = 210;
    buttonList[9]->height = 20;
    
    // are we working with a SaltySD title?
    if (strncmp("00040000000EE000", tid, MAXSIZE) == 0 ||  // Smash Bros.
        strncmp("00040000000EDF00", tid, MAXSIZE) == 0)
        isSaltySD = 1;
    else if (strncmp("0004000000164800", tid, MAXSIZE) == 0 ||
             strncmp("0004000000175E00", tid, MAXSIZE) == 0)
        isSaltySD = 2;
    
    // create and fill mod list
    char* path = malloc(MAXSIZE * sizeof(char*));
    memset(path, 0, MAXSIZE * sizeof(char*));
    snprintf(path, MAXSIZE, "/3ds/data/Haxelektor/%s/mods", tid);
    
    modListing = listAllFiles(path, &modCount, 1);
    modSelected = malloc(modCount * sizeof(u8*));
    
    for (int i = 0; i < modCount; i++) {
        modSelected[i] = 0;
        
        memset(path, 0, MAXSIZE * sizeof(char*));
        snprintf(path, MAXSIZE, "3ds/data/Haxelektor/%s/mods/%s/image.png", tid, modListing[i]);
        unsigned result = pp2d_load_texture_png(i, path);
        if (result) {
            pp2d_load_texture_png(i, "romfs:/notexist.png");
        }
    }
    
    // create and fill config list
    memset(path, 0, MAXSIZE * sizeof(char*));
    snprintf(path, MAXSIZE, "/3ds/data/Haxelektor/%s/config/", tid);
    
    mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    configListing = listAllFiles(path, &configCount, 0);
    
    // set TID
    currentTid = malloc(MAXSIZE * sizeof(char*));
    memset(currentTid, 0, MAXSIZE * sizeof(char*));
    snprintf(currentTid, MAXSIZE, "%s", tid);
    
    free(path);
}

// prompts user for input
int uiPrompt(const char* prompt) {
    int result = -1;
    pp2d_set_screen_color(GFX_TOP, LGREYBG);
    pp2d_set_screen_color(GFX_BOTTOM, LGREYBG);
    
    while (aptMainLoop()) {
        
        // update
        hidScanInput();
        u32 kDown = hidKeysDown();
        
        if (kDown & KEY_B) {
            result = -1;
            break;
        } else if (kDown & KEY_A) {
            result = 0;
            break;
        }
        
        // display
        pp2d_begin_draw(GFX_TOP);
            pp2d_draw_rectangle(0, 0, 400, 15, GREYFG);
            pp2d_draw_rectangle(0, 220, 400, 20, GREYFG);
        
            //pp2d_draw_rectangle(0, 100, 400, 15, GREYFG);
            pp2d_draw_text_center(GFX_TOP, 100, 0.5f, 0.5f, WHITE, prompt);
            pp2d_draw_text_center(GFX_TOP, 0, 0.5f, 0.5f, WHITE, "Answer the prompt.");
            //pp2d_draw_rectangle(0, 200, 400, 15, GREYFG);
            pp2d_draw_text(40,  223, 0.5f, 0.5f, WHITE, "Yes \uE000");
            pp2d_draw_text(360 - noWidth, 223, 0.5f, 0.5f, WHITE, "No \uE001");
        pp2d_end_draw();
        
        pp2d_begin_draw(GFX_BOTTOM);
            pp2d_draw_rectangle(0, 0, 320, 20, GREYFG);
            pp2d_draw_rectangle(0, 220, 320, 240, GREYFG);
        pp2d_end_draw();
    }
    
    return result;
}

void uiLoading() {
    pp2d_begin_draw(GFX_TOP);
        pp2d_draw_rectangle(0, 0, 400, 15, GREYFG);
        pp2d_draw_rectangle(0, 220, 400, 20, GREYFG);
        
        pp2d_draw_text_center(GFX_TOP, 100, 0.5f, 0.5f, WHITE, "Please wait.");
    pp2d_end_draw();
    
    pp2d_begin_draw(GFX_BOTTOM);
        pp2d_draw_rectangle(0, 0, 320, 20, GREYFG);
        pp2d_draw_rectangle(0, 220, 320, 240, GREYFG);
    pp2d_end_draw();
}
    
void uiError(const char* error) {
    pp2d_set_screen_color(GFX_TOP, LGREYBG);
    pp2d_set_screen_color(GFX_BOTTOM, LGREYBG);
    
    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        
        if (kDown)
            break;
        
        pp2d_begin_draw(GFX_TOP);
            pp2d_draw_rectangle(0, 0, 400, 15, GREYFG);
            pp2d_draw_rectangle(0, 220, 400, 20, GREYFG);
            pp2d_draw_text_center(GFX_TOP, 100, 0.5f, 0.5f, WHITE, error);
            pp2d_draw_text_center(GFX_TOP, 223, 0.5f, 0.5f, WHITE, "Press any key to continue.");
        pp2d_end_draw();
        
        pp2d_begin_draw(GFX_BOTTOM);
            pp2d_draw_rectangle(0, 0, 320, 20, GREYFG);
            pp2d_draw_rectangle(0, 220, 320, 240, GREYFG);
        pp2d_end_draw();
    }
}

LOOP_RETURN uiModSelectLoop() {
    pp2d_set_screen_color(GFX_TOP, LGREYBG);
    pp2d_set_screen_color(GFX_BOTTOM, LGREYBG);
    
    int selectConfig = 0;
    s16 lCount = modCount;
    
    char* strIndex = malloc(MAXSIZE * sizeof(char*));
    char** listing = modListing;
    
    char desc[255];
    loadFromFile(desc);
    
    int max = 0;
    
    SwkbdState kbd;
    SwkbdButton btn;
    
    char* saveTempPath;

    while (aptMainLoop()) {
        int add = 0;
        
        // read input
        hidScanInput();
        u32 kDown = hidKeysDown();
        
        s16 buttonTouched;
        
        touchPosition touch;
        hidTouchRead(&touch);
        
        // react according to input
        if (kDown & KEY_START)
            break;
        
        
        // button input
        if (cursorPos == 0) {
            switch (kDown) {
                case KEY_DUP:
                    entryIndex--;
                    if (selectConfig == 0)
                        loadFromFile(desc);
                    break;
                case KEY_DDOWN:
                    entryIndex++;
                    if (selectConfig == 0)
                        loadFromFile(desc);
                    add = 1;
                    break;
                case KEY_DLEFT:
                    if (selectConfig == 0)
                        cursorPos = !cursorPos;
                    else {
                        entryIndex = entryIndex - 13;
                        indexPos = indexPos - 13;   
                    }
                    break;
                case KEY_DRIGHT:
                    if (selectConfig == 0)
                        cursorPos = !cursorPos;
                    else {
                        entryIndex = entryIndex + 13;
                        if ((entryIndex >= lCount) && indexPos != ((int)ceil((float)lCount / 13.0) - 1) * 13) {
                            entryIndex = lCount - 1;
                        }
                        indexPos = indexPos + 13;
                    }
                    break;
                case KEY_A:
                    loadConfig:
                    if (selectConfig == 0)
                        modSelected[entryIndex] = !modSelected[entryIndex];
                    else {
                        // load configuration
                        memset(strIndex, 0, MAXSIZE * sizeof(char*));
                        snprintf(strIndex, MAXSIZE, "/3ds/data/Haxelektor/%s/config/%s", currentTid, listing[entryIndex]);
                        int result = jsonLoad(strIndex, modCount, modListing, modSelected);
 

                        
                        if (result == -1)
                            uiError("Failed to load configuration.");
                        else
                            uiError("Configuration loaded.");
                    }
                     
                case KEY_B:
                    // change mode back
                    if (selectConfig != 0) {
                        selectConfig = 0;
                        entryIndex = 0;
                        indexPos = 0;
                        cursorPos = 0;
                        configIndex = 0;
                        buttonIndex = 0;
                        kDown = 0;
                        listing = modListing;
                        lCount = modCount;
                    }
                    break;
                default:
                    break;
            };
            
            if (entryIndex >= lCount) {
                entryIndex = 0;
                indexPos = 0;
                loadFromFile(desc);
            } else if (entryIndex < 0) {
                entryIndex = lCount - 1;
                indexPos = ((int)ceil((float)lCount / 13.0) - 1) * 13;
                loadFromFile(desc);
            }
        } else if (cursorPos == 1) {
            switch (kDown) {
                case KEY_DUP:
                    if (buttonIndex == 7)
                        buttonIndex--;
                    buttonIndex--;
                    break;
                case KEY_DDOWN:
                    if (buttonIndex == 6)
                        buttonIndex++;
                    buttonIndex++;
                    break;
                case KEY_DLEFT:
                    if (buttonIndex == 7)
                        buttonIndex--;
                    else if (buttonIndex == 6)
                        buttonIndex++;
                    else
                        cursorPos = !cursorPos;
                    break;
                case KEY_DRIGHT:
                    if (buttonIndex == 6)
                        buttonIndex++;
                    else if (buttonIndex == 7)
                        buttonIndex--;
                    else
                        cursorPos = !cursorPos;
                    break;
                case KEY_A:
                    buttonTouched = buttonIndex;
                    goto buttonInteraction;
                    break;
                default:
                    break;
            };
        }
        
        // touch input
        buttonTouched = getCurrentButtonTouched(&touch);
        if (buttonTouched != -1)
            printf("button touched: %d\n", buttonTouched);
        
        max = (indexPos + 13 < lCount) ? indexPos + 13 : lCount;
        
        if (kDown & KEY_TOUCH) {
            buttonInteraction:
            switch (buttonTouched) {
                case 0:
                    if (selectConfig != 0)
                        break;
                    free(strIndex);
                    return LAUNCH_GAME;
                    break;
                case 1:
                    if (selectConfig != 0)
                        break;
                    uiLoading();
                    char* temp = malloc(MAXSIZE * sizeof(char*));
                     
                    // create paths as necessary
                    if (isSaltySD == 0) {
                        mkdir("/luma", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                        mkdir("/luma/titles", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                        
                        memset(temp, 0, MAXSIZE * sizeof(char*));
                        snprintf(temp, MAXSIZE, "/luma/titles/%s", currentTid);
                        mkdir(temp, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                        
                        memset(temp, 0, MAXSIZE * sizeof(char*));
                        snprintf(temp, MAXSIZE, "/luma/titles/%s/romfs", currentTid);
                        mkdir(temp, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                    } else if (isSaltySD == 1 || isSaltySD == 2) {
                        mkdir("/saltysd", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                    } 
                    
                    memset(temp, 0, MAXSIZE * sizeof(char*));
                    if (isSaltySD == 1) { // Smash 4
                        snprintf(temp, MAXSIZE, "/saltysd/smash");
                    } else if (isSaltySD == 2) {
                        snprintf(temp, MAXSIZE, "/saltysd/SunMoon");
                    } else {
                        snprintf(temp, MAXSIZE, "/luma/titles/%s/romfs", currentTid);
                    }

                    // delete old unnecessary mod folders, if they exist
                    removeDir(temp);
                    
                    for (int i = modCount - 1; i >= 0; i--) {
                        if (modSelected[i] == 0)
                            continue;
                        
                        memset(strIndex, 0, MAXSIZE * sizeof(char*));
                        snprintf(strIndex, MAXSIZE, "/3ds/data/Haxelektor/%s/mods/%s/mod",currentTid, modListing[i]);
                        copyDir(strIndex, temp);
                    }
                        
                    free(temp);
                    uiError("Patches for this game have been applied.");
                    break;
                case 2:
                    if (selectConfig != 0)
                        break;
                    memset(strIndex, 0, MAXSIZE * sizeof(char*));
                    if (isSaltySD == 0)
                        snprintf(strIndex, MAXSIZE, "/luma/titles/%s/romfs", currentTid);
                    else if (isSaltySD == 1)
                        snprintf(strIndex, MAXSIZE, "saltysd/smash");
                    else if (isSaltySD == 2)
                        snprintf(strIndex, MAXSIZE, "saltysd/SunMoon");
                    
                    removeDir(strIndex);
                    uiError("All patches for this game have been removed.");
                    break;
                case 3:
                    // TODO: kinda stubbed, implement naming stuff and shit
                    saveTempPath = malloc(MAXSIZE * sizeof(char*));
                    
                    memset(saveTempPath, 0, MAXSIZE * sizeof(char*));
                    memset(strIndex, 0, MAXSIZE * sizeof(char*));
                    
                    swkbdInit(&kbd, SWKBD_TYPE_NORMAL, 2, 255);
                    swkbdSetButton(&kbd, SWKBD_BUTTON_LEFT, "Cancel", false);
                    swkbdSetButton(&kbd, SWKBD_BUTTON_RIGHT, "Save", true);
                    swkbdSetHintText(&kbd, "Enter a name for your config file.");
                    
                    btn = SWKBD_BUTTON_NONE;
                    while (btn == SWKBD_BUTTON_NONE)
                        btn = swkbdInputText(&kbd, saveTempPath, MAXSIZE);
                    
                    snprintf(strIndex, MAXSIZE, "/3ds/data/Haxelektor/%s/config/%s", currentTid, saveTempPath);
                    
                    int result = jsonSave(strIndex, modCount, modListing, modSelected);
                    if (result == -1) {
                        uiError("Saving configuration failed.");
                        break;
                    } else
                        uiError("Configuration successfully saved.");
                    
                    memset(strIndex, 0, MAXSIZE * sizeof(char*));
                    snprintf(strIndex, MAXSIZE, "/3ds/data/Haxelektor/%s/config/", currentTid);
                    configListing = listAllFiles(strIndex, &configCount, 0);
                    break;
                case 4:
                    if (selectConfig != 0)
                        break;
                    
                    if (configCount == 0)
                        uiError("There are no existing configs for this title.");
                    
                    selectConfig = !selectConfig;
                    listing = configListing;
                    lCount = configCount;
                    cursorPos = 0;
                    break;
                case 5:
                    if (selectConfig != 0)
                        break;
                    if (entryIndex - 1 < 0)
                        break;
                    
                    swapEntries(entryIndex, entryIndex - 1);
                    entryIndex--;
                    break;
                case 6:
                    if (selectConfig != 0)
                        break;
                    entryIndex = entryIndex - 13;
                    indexPos = indexPos - 13;
                    if (entryIndex > 0)
                        loadFromFile(desc);
                    break;
                case 7:
                    if (selectConfig != 0)
                        break;
                    entryIndex = entryIndex + 13;
                    if ((entryIndex >= modCount) && indexPos != ((int)ceil((float)modCount / 13.0) - 1) * 13) {
                        entryIndex = modCount - 1;
                    }
                    indexPos = indexPos + 13;
                    if (entryIndex < modCount)
                        loadFromFile(desc);
                    break;
                case 8:
                    if (selectConfig != 0)
                        break;
                    if (entryIndex + 1 >= modCount)
                        break;
                    
                    swapEntries(entryIndex, entryIndex + 1);
                    entryIndex++;
                    break;
                case 9:
                    if (selectConfig != 0)
                        break;
                    free(strIndex);
                    return GO_BACK;
                default:
                    break;
            };
            
            s16 prevEntryIndex = entryIndex;

            for (int i = 0; i < max; i++) {
                int horizReach = (selectConfig == 0) ? 200 : 320;
                if ((0 <= touch.px && touch.px <= horizReach) && \
                   (20 + ((i % 13) * 15) <= touch.py && touch.py <= 20 + ((i % 13) * 15) + 15)) {
                        entryIndex = indexPos + i;
                        if (entryIndex == prevEntryIndex && selectConfig == 0)
                           modSelected[indexPos + i] = !modSelected[indexPos + i];
                        else if (selectConfig == 0)
                            loadFromFile(desc);
                        else if (selectConfig != 0 && entryIndex == prevEntryIndex)
                            goto loadConfig;
                        
                        
                        if (cursorPos != 0)
                            cursorPos = !cursorPos;
                        break;
                   }
            }
        }    
        
        // keep indices in place
        if (entryIndex < indexPos) {
            indexPos = indexPos - 13;
            loadFromFile(desc);
        } else if ((entryIndex % 13) == 0 && entryIndex != 0 && add == 1) {
            indexPos = indexPos + 13;
            loadFromFile(desc);
        }
        
        if (indexPos >= lCount) {
            entryIndex = 0;
            indexPos = 0;
            loadFromFile(desc);
        } else if (indexPos < 0) {
            indexPos = ((int)ceil((float)lCount / 13.0) - 1) * 13;
            entryIndex = indexPos;
            loadFromFile(desc);
        }

        if (buttonIndex >= NUMBTNS)
            buttonIndex = 0;
        else if (buttonIndex < 0) 
            buttonIndex = NUMBTNS - 1;            
        
        // display to screen
        pp2d_begin_draw(GFX_TOP);
            pp2d_draw_rectangle(0, 0, 400, 240, GREYFG);
            if (selectConfig == 0) {
                pp2d_draw_texture(entryIndex, 0, 0);
                pp2d_draw_rectangle(0, 220, 400, 20, GREYFG);
                pp2d_draw_text(0, 223, 0.5f, 0.5f, WHITE, desc);
            }
        pp2d_end_draw(); 
        
        pp2d_begin_draw(GFX_BOTTOM);
            // draw mod entries
            
            
            for (int i = indexPos; i < max; i++) {
                memset(strIndex, 0, MAXSIZE * sizeof(char*));
                snprintf(strIndex, MAXSIZE, "%d", i + indexPos + 1);
                
                if (i == entryIndex && cursorPos == 0)
                    pp2d_draw_rectangle(0, 20 + ((i % 13) * 15), 320, 15, GREYFG);
                else if (i == entryIndex)
                    pp2d_draw_rectangle(0, 20 + ((i % 13) * 15), 320, 15, GREY2FG);
                pp2d_draw_text(0, 20 + ((i % 13) * 15), 0.5f, 0.5f, WHITE, strIndex);
                pp2d_draw_text(30, 20 + ((i % 13) * 15), 0.5f, 0.5f, WHITE, listing[i]);
                if (modSelected[i] != 0 && selectConfig == 0) 
                    pp2d_draw_text(150, 20 + ((i % 13) * 15), 0.5f, 0.5f, WHITE, "Selected");
            }
        
            // draw side and bottom bar
            pp2d_draw_rectangle(0, 0, 320, 20, GREYFG);
            pp2d_draw_rectangle(0, 220, 320, 220, GREYFG);
            if (selectConfig == 0)
                pp2d_draw_rectangle(210, 0, 120, 240, GREY2FG);
            
            memset(strIndex, 0, MAXSIZE * sizeof(char*));
            if (selectConfig == 0)
                snprintf(strIndex, MAXSIZE, "Number of Mods: %d", (int)modCount);
            else
                snprintf(strIndex, MAXSIZE, "Select a config file.");
            
            pp2d_draw_text(0, 0, 0.5f, 0.5f, WHITE, strIndex);
            
            // draw all the side buttons
            if (selectConfig == 0)
                for (int i = 0; i < NUMBTNS; i++) {
                    drawButton(buttonList[i], (i == buttonIndex && cursorPos == 1) || isButtonTouched(buttonList[i], &touch));
                }
        pp2d_end_draw();
    }
    
    listing = NULL;
    free(strIndex);
    return GO_BACK;
}

void uiExit() {
    free(configListing);
    free(currentTid);
    free(modListing);  
    free(buttonList);
    free(modSelected);
}