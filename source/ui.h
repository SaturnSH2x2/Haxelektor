#ifndef UI_H
#define UI_H

typedef enum {
    LAUNCH_GAME,
    GO_BACK,
    REBOOT
} LOOP_RETURN;

typedef struct {
    int x;
    int y;
    int width;
    int height;
    char* text;
} UIButton;

typedef struct {
    char* name;
    char* description;
    int imgID;
    int isSelected;
} GameMod;

typedef struct {
    char** modListing;
    int modCount;
    int** imageList;
    int** modSelected;
} GameModList;

typedef struct {
    int side;
    int index;
} UICursor;

void uiInit();
void uiError(const char* error);
void uiLoading();
int uiPrompt(const char* prompt);
LOOP_RETURN uiModSelectLoop();
void uiExit();

#endif