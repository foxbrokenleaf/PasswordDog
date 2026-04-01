#include "oled_menu.h"
#include "OLED.h"
#include <string.h>
#include "main.h"

uint8_t ContentIndex = 0;
uint8_t ContentLenght = 0;
uint32_t lastcurri = 0;
uint8_t runFlag = 0;

Menu *DisplayMenu = NULL;
Menu *DisplayMenuLimit = NULL;

Menu SecurityMenu[] = {
    {.Title = "Set Password", .preMenu = NULL, .nextMenu = NULL, .pFunc = Function_SetPassword},
    {.Title = "Back", .preMenu = NULL, .nextMenu = NULL, .pFunc = NULL},   
};

Menu AboutSystemMenu[] = {
    {.Title = "System Version", .preMenu = NULL, .nextMenu = NULL, .pFunc = Function_SystemVersion},
    {.Title = "System Number Serial", .preMenu = NULL, .nextMenu = NULL, .pFunc = Function_SystemSerialNumber},
    {.Title = "Back", .preMenu = NULL, .nextMenu = NULL, .pFunc = NULL},      
};

Menu SystemMenu[] = {
    {.Title = "Reboot", .preMenu = NULL, .nextMenu = NULL, .pFunc = Function_Reboot},
    {.Title = "Transform Data From PC", .preMenu = NULL, .nextMenu = NULL, .pFunc = Function_SystemTransformDataForPC},
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
    DisplayMenuLimit = menu;
    MainMenu[1].nextMenu = (struct Menu*)SecurityMenu;
    MainMenu[2].nextMenu = (struct Menu*)AboutSystemMenu;
    MainMenu[3].nextMenu = (struct Menu*)SystemMenu;
    for(uint8_t i = 0;i < sizeof(SecurityMenu) / sizeof(Menu);i++){
        SecurityMenu[i].preMenu = (struct Menu*)(MainMenu + 1);
        if(i == ((sizeof(SecurityMenu) / sizeof(Menu)) - 1)) SecurityMenu[i].nextMenu = (struct Menu*)(MainMenu + 1);
    }
    for(uint8_t i = 0;i < sizeof(AboutSystemMenu) / sizeof(Menu);i++){
        AboutSystemMenu[i].preMenu = (struct Menu*)(MainMenu + 2);
        if(i == ((sizeof(AboutSystemMenu) / sizeof(Menu)) - 1)) AboutSystemMenu[i].nextMenu = (struct Menu*)(MainMenu + 2);
    }
    for(uint8_t i = 0;i < sizeof(SystemMenu) / sizeof(Menu);i++){
        SystemMenu[i].preMenu = (struct Menu*)(MainMenu + 3);
        if(i == ((sizeof(SystemMenu) / sizeof(Menu)) - 1)) SystemMenu[i].nextMenu = (struct Menu*)(MainMenu + 3);
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

    if(ContentIndex == 0 || ContentLenght == 0) ContentLenght = ContentIndex = strlen((DisplayMenu)->Title);
    OLED_ShowString(0, 0, ((DisplayMenu)->Title + ContentLenght - ContentIndex), OLED_8X16);
    // OLED_ShowNum(0, 0, (sizeof(DisplayMenu)), 2, OLED_8X16);
    // OLED_Printf(0, 0, OLED_6X8, "MM:%X", MainMenu);
    // OLED_Printf(0, 0, OLED_6X8, "DML:%X", DisplayMenuLimit);
    // OLED_Printf(0, 8, OLED_6X8, "DM:%X", DisplayMenu);

    OLED_Update();
    OLED_Clear();
}

void Menu_Up(void){
    ContentIndex = 0;
    if(DisplayMenu >= MainMenu && DisplayMenu <= (MainMenu + (sizeof(MainMenu) / sizeof(Menu)) - 1)) if(DisplayMenu > MainMenu) DisplayMenu--;
    if(DisplayMenu >= SecurityMenu && DisplayMenu <= (SecurityMenu + (sizeof(SecurityMenu) / sizeof(Menu)) - 1)) if(DisplayMenu > SecurityMenu) DisplayMenu--;
    if(DisplayMenu >= AboutSystemMenu && DisplayMenu <= (AboutSystemMenu + (sizeof(AboutSystemMenu) / sizeof(Menu)) - 1)) if(DisplayMenu > AboutSystemMenu) DisplayMenu--;
    if(DisplayMenu >= SystemMenu && DisplayMenu <= (SystemMenu + (sizeof(SystemMenu) / sizeof(Menu)) - 1)) if(DisplayMenu > SystemMenu) DisplayMenu--;
}

void Menu_Down(void){
    ContentIndex = 0;
    if(DisplayMenu >= MainMenu && DisplayMenu <= (MainMenu + (sizeof(MainMenu) / sizeof(Menu)) - 1)) if(DisplayMenu < (MainMenu + (sizeof(MainMenu) / sizeof(Menu)) - 1)) DisplayMenu++;
    if(DisplayMenu >= SecurityMenu && DisplayMenu <= (SecurityMenu + (sizeof(SecurityMenu) / sizeof(Menu)) - 1)) if(DisplayMenu < (SecurityMenu + (sizeof(SecurityMenu) / sizeof(Menu)) - 1)) DisplayMenu++;
    if(DisplayMenu >= AboutSystemMenu && DisplayMenu <= (AboutSystemMenu + (sizeof(AboutSystemMenu) / sizeof(Menu)) - 1)) if(DisplayMenu < (AboutSystemMenu + (sizeof(AboutSystemMenu) / sizeof(Menu)) - 1)) DisplayMenu++;
    if(DisplayMenu >= SystemMenu && DisplayMenu <= (SystemMenu + (sizeof(SystemMenu) / sizeof(Menu)) - 1)) if(DisplayMenu < (SystemMenu + (sizeof(SystemMenu) / sizeof(Menu)) - 1)) DisplayMenu++;
}

void Menu_Back(void){
    ContentIndex = 0;
    if(DisplayMenu->preMenu != NULL){
        DisplayMenu = (Menu*)DisplayMenu->preMenu;
        DisplayMenuLimit = (Menu*)DisplayMenu->preMenu;
    }
}

void Menu_Enter(void){
    ContentIndex = 0;
    if(DisplayMenu->nextMenu != NULL){
        DisplayMenu = (Menu*)DisplayMenu->nextMenu;
        DisplayMenuLimit = (Menu*)DisplayMenu->nextMenu;
    }
    else if(DisplayMenu->pFunc != NULL){
        DisplayMenu->pFunc();
    }
}

void Function_Reboot(void){
    __set_FAULTMASK(1);
    NVIC_SystemReset();    
}

void Function_SetPassword(void){

    ResetPassword = 1;
    DriverLock = 1;
    
}

void Function_SystemVersion(void){
    ContentIndex = ContentLenght = 0;
    showMenu_Flag = 0;
    showSystemVersion_Flag = 1;
}

void Function_SystemSerialNumber(void){
    ContentIndex = ContentLenght = 0;
    showMenu_Flag = 0;
    showSystemSerialNumber_Flag = 1;
}

void Function_SystemTransformDataForPC(void){
    ContentIndex = ContentLenght = 0;
    showMenu_Flag = 0;
    GetDataForPC_Flag = 1;
}