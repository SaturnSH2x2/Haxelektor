#ifndef UI_H
#define UI_H

typedef enum {
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
    int side;
    int index;
} UICursor;

void uiInit();
LOOP_RETURN uiModSelectLoop();
void uiExit();

#endif