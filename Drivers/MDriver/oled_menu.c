#include "oled_menu.h"
#include "OLED.h"
#include <string.h>

uint8_t ContentIndex = 0;
uint8_t ContentLenght = 0;
uint32_t lastcurri = 0;
uint8_t runFlag = 0;

Menu *DisplayMenu = NULL;

Menu SecurityMenu[] = {
    {.Title = "Set Password", .preMenu = NULL, .nextMenu = NULL, .pFunc = NULL},
    {.Title = "Back", .preMenu = NULL, .nextMenu = NULL, .pFunc = NULL},   
};

Menu MainMenu[] = {
    {.Title = "Data List", .preMenu = NULL, .nextMenu = NULL, .pFunc = NULL},
    {.Title = "Security", .preMenu = NULL, .nextMenu = NULL, .pFunc = NULL},
    {.Title = "About System", .preMenu = NULL, .nextMenu = NULL, .pFunc = NULL},
    {.Title = "System", .preMenu = NULL, .nextMenu = NULL, .pFunc = NULL},
};

void MenuInit(Menu *menu){
    DisplayMenu = menu;
    MainMenu[1].nextMenu = (struct Menu*)SecurityMenu;
    for(uint8_t i = 0;i < sizeof(SecurityMenu) / 16;i++){
        SecurityMenu[i].preMenu = (struct Menu*)(MainMenu + 1);
        if(i == ((sizeof(SecurityMenu) / 16) - 1)) SecurityMenu[i].nextMenu = (struct Menu*)(MainMenu + 1);
    }
}

void ConfigNextMenu(Menu *nowMenu, Menu *nextMenu){

}

void ConfigPreMenu(Menu *nowMenu, Menu *preMenu){
    
}

void MenuDisplay(void){
    uint32_t nowcurri = HAL_GetTick();
    if(nowcurri > lastcurri + 1000){
        lastcurri = nowcurri;
        ContentIndex--;
    }

    if(ContentIndex == 0) ContentLenght = ContentIndex = strlen((DisplayMenu)->Title);
    OLED_ShowString(0, 0, ((DisplayMenu)->Title + ContentLenght - ContentIndex), OLED_8X16);
    // OLED_ShowNum(0, 0, sizeof(MainMenu) / 16, 2, OLED_8X16);
    // OLED_Printf(0, 0, OLED_6X8, "MM:%X", MainMenu);
    // OLED_Printf(0, 8, OLED_6X8, "DM:%X", DisplayMenu);

    OLED_Update();
    OLED_Clear();
}

void Menu_Up(void){
    ContentIndex = 0;
    if(DisplayMenu > MainMenu) DisplayMenu--;
    
}

void Menu_Down(void){
    ContentIndex = 0;
    if(DisplayMenu < (MainMenu + (sizeof(MainMenu) / 16) - 1)) DisplayMenu++;
}

void Menu_Back(void){
    ContentIndex = 0;
    if(DisplayMenu->preMenu != NULL) DisplayMenu = (Menu*)DisplayMenu->preMenu;
}

void Menu_Enter(void){
    ContentIndex = 0;
    if(DisplayMenu->nextMenu != NULL) DisplayMenu = (Menu*)DisplayMenu->nextMenu;
}