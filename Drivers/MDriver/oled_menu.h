#ifndef __OLED_MENU_H__
#define __OLED_MENU_H__

#include "stm32f1xx_hal.h"

/*========================Variable Type define=========================*/
typedef struct _Menu{
    char *Title;
    struct Menu *preMenu;
    struct Menu *nextMenu;
    void (*pFunc)(void);
}Menu;

/*========================Extern Variable=========================*/
extern Menu SecurityMenu[];
extern Menu MainMenu[];


/*========================Menu API=========================*/
void MenuInit(Menu *menu);
void ConfigNextMenu(Menu *nowMenu, Menu *nextMenu);
void ConfigPreMenu(Menu *nowMenu, Menu *preMenu);
void MenuDisplay(void);
void Menu_Up(void);
void Menu_Down(void);
void Menu_Back(void);
void Menu_Enter(void);

/*========================Function Define=========================*/
void Function_Reboot(void);
void Function_SetPassword(void);
void Function_SystemVersion(void);
void Function_SystemSerialNumber(void);
void Function_SystemTransformDataForPC(void);

#endif