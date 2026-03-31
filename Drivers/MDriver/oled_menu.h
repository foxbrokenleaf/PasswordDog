#ifndef __OLED_MENU_H__
#define __OLED_MENU_H__

#include "stm32f1xx_hal.h"

typedef struct _Menu{
    char *Title;
    struct Menu *preMenu;
    struct Menu *nextMenu;
    void (*pFunc)(void);
}Menu;

extern Menu SecurityMenu[];
extern Menu MainMenu[];

void MenuInit(Menu *menu);
void ConfigNextMenu(Menu *nowMenu, Menu *nextMenu);
void ConfigPreMenu(Menu *nowMenu, Menu *preMenu);
void MenuDisplay(void);
void Menu_Up(void);
void Menu_Down(void);
void Menu_Back(void);
void Menu_Enter(void);

#endif