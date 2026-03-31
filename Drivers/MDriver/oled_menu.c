/* oled_menu.c - 修正编译错误版 */
#include "oled_menu.h"
#include <stdio.h>
#include <stdarg.h>
#include "Key.h"
#include "main.h"

#define SCROLL_DELAY_MS 300
#define SCROLL_PAUSE_MS 1500

Menu* currentMenu = NULL;
static Menu* platformMenuPtr = NULL;

// 数据项定义（实际使用时从FLASH读取）
static PlatformData platformData[] = {
    {"GitHub", "user@example.com", "5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8", 1},
    {"GitHub", "user@example.com", "5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8", 1},
    {"Google", "user@gmail.com", "5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8", 1},
    {"Microsoft", "user@outlook.com", "5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8", 1},
    {"AWS", "admin@aws.com", "5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8", 1},
    {"Tencent", "user@tencent.com", "5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8", 1},
    {"Alibaba", "user@aliyun.com", "5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8", 1},
    {"mosterhunter:rise", "user@aliyun.com", "5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8", 1}
};

static uint8_t platformCount = sizeof(platformData) / sizeof(PlatformData);
static MenuItem platformMenuItems[50];
static uint32_t currentPlatformIndex = 0;

// 获取字符串显示长度
static uint8_t GetStringLength(const char* str) {
    uint8_t len = 0;
    while (*str) {
        if (*str & 0x80) {
            len += 2;
            str += 2;
        } else {
            len++;
            str++;
        }
    }
    return len;
}

// 获取字符串指定位置开始的显示内容
static void GetScrollString(const char* src, char* dest, uint8_t startPos, uint8_t maxLen) {
    uint8_t currentPos = 0;
    uint8_t destIndex = 0;
    const char* p = src;
    const char* start = src;
    uint8_t totalLen = 0;
    
    // 先计算总长度
    while (*start) {
        if (*start & 0x80) {
            totalLen += 2;
            start += 2;
        } else {
            totalLen++;
            start++;
        }
    }
    
    // 如果起始位置超过总长度，返回空字符串
    if (startPos >= totalLen) {
        *dest = '\0';
        return;
    }
    
    // 移动到起始位置
    while (*p && currentPos < startPos) {
        if (*p & 0x80) {
            currentPos += 2;
            p += 2;
        } else {
            currentPos++;
            p++;
        }
    }
    
    // 复制指定长度的字符
    while (*p && destIndex < maxLen) {
        if (*p & 0x80) {
            // 中文字符需要2个字节，但只占1个显示位置
            if (destIndex + 1 < maxLen) {
                *dest++ = *p++;
                *dest++ = *p++;
                destIndex += 2;
            } else if (destIndex + 1 == maxLen) {
                // 只能放一个中文字符的一半，用省略号替代
                if (destIndex >= 1) {
                    dest[-1] = '.';
                }
                *dest++ = '.';
                destIndex++;
                break;
            } else {
                break;
            }
        } else {
            // 英文字符
            *dest++ = *p++;
            destIndex++;
        }
    }
    
    *dest = '\0';
    
    // 如果还没到末尾，且显示已满，添加省略号
    if (*p != '\0' && destIndex == maxLen) {
        if (destIndex >= 2) {
            dest[-1] = '.';
            dest[-2] = '.';
        } else if (destIndex == 1) {
            dest[-1] = '.';
            *dest = '.';
            *(dest+1) = '\0';
        }
    }
}


// 检查是否需要滚动
bool NeedScroll(const char* str) {
    return GetStringLength(str) > (MAX_DISPLAY_CHARS - MENU_PREFIX_LEN);
}

// 显示菜单
void Menu_Display(Menu* menu) {
    if (menu == NULL || menu->itemCount == 0) return;
    
    const char* itemName = menu->items[menu->currentIndex].name;
    uint8_t nameLen = GetStringLength(itemName);
    uint8_t availableLen = MAX_DISPLAY_CHARS - MENU_PREFIX_LEN;
    char displayStr[MAX_DISPLAY_CHARS + 1];
    char finalStr[MAX_DISPLAY_CHARS + 3];
    
    OLED_Clear();
    
    if (menu->scrollState.isScrolling && menu->scrollState.currentItem == menu->currentIndex) {
        GetScrollString(itemName, displayStr, menu->scrollState.scrollPos, availableLen);
        snprintf(finalStr, sizeof(finalStr), ">%s", displayStr);
    } else {
        if (nameLen > availableLen) {
            GetScrollString(itemName, displayStr, 0, availableLen - 1);
            snprintf(finalStr, sizeof(finalStr), ">%s.", displayStr);
        } else {
            snprintf(finalStr, sizeof(finalStr), ">%s", itemName);
        }
    }
    
    finalStr[MAX_DISPLAY_CHARS] = '\0';
    OLED_ShowString(0, 0, finalStr, OLED_8X16);
    OLED_Update();
}

// 菜单初始化
void Menu_Init(Menu* menu, MenuItem* items, uint8_t count, MenuType type, Menu* parent) {
    menu->items = items;
    menu->itemCount = count;
    menu->currentIndex = 0;
    menu->type = type;
    menu->parent = parent;
    menu->platformIndex = 0;
    menu->scrollState.isScrolling = false;
    menu->scrollState.scrollTimer = 0;
    menu->scrollState.scrollPos = 0;
    menu->scrollState.currentItem = 0;
}

// 向上移动
void Menu_Up(Menu* menu) {
    if (menu->currentIndex > 0) {
        menu->currentIndex--;
    } else {
        menu->currentIndex = menu->itemCount - 1;
    }
    
    menu->scrollState.isScrolling = false;
    menu->scrollState.scrollPos = 0;
    Menu_Display(menu);
}

// 向下移动
void Menu_Down(Menu* menu) {
    if (menu->currentIndex < menu->itemCount - 1) {
        menu->currentIndex++;
    } else {
        menu->currentIndex = 0;
    }
    
    menu->scrollState.isScrolling = false;
    menu->scrollState.scrollPos = 0;
    Menu_Display(menu);
}

// 执行当前选中菜单项
void Menu_Enter(Menu* menu) {
    if (menu->items[menu->currentIndex].func != NULL) {
        menu->scrollState.isScrolling = false;
        menu->items[menu->currentIndex].func();
    }
}

// 更新滚动显示
void Menu_UpdateScroll(Menu* menu) {
    if (!menu->scrollState.isScrolling) return;
    
    uint32_t currentTime = HAL_GetTick();
    
    if (menu->scrollState.scrollTimer == 0) {
        menu->scrollState.scrollTimer = currentTime;
        return;
    }
    
    if (currentTime - menu->scrollState.scrollTimer >= SCROLL_DELAY_MS) {
        const char* itemName = menu->items[menu->scrollState.currentItem].name;
        uint8_t totalLen = GetStringLength(itemName);
        uint8_t availableLen = MAX_DISPLAY_CHARS - MENU_PREFIX_LEN;
        int8_t maxScrollPos = totalLen - availableLen;
        
        menu->scrollState.scrollPos++;
        
        if (menu->scrollState.scrollPos > maxScrollPos) {
            menu->scrollState.scrollPos = 0;
            menu->scrollState.isScrolling = false;
            menu->scrollState.scrollTimer = 0;
        } else {
            menu->scrollState.scrollTimer = currentTime;
        }
        
        Menu_Display(menu);
    }
}

// 启动滚动
void Menu_StartScroll(Menu* menu, uint8_t itemIndex) {
    const char* itemName = menu->items[itemIndex].name;
    uint8_t nameLen = GetStringLength(itemName);
    uint8_t availableLen = MAX_DISPLAY_CHARS - MENU_PREFIX_LEN;
    
    if (nameLen > availableLen && !menu->scrollState.isScrolling) {
        menu->scrollState.isScrolling = true;
        menu->scrollState.scrollPos = 0;
        menu->scrollState.currentItem = itemIndex;
        menu->scrollState.scrollTimer = 0;
    }
}

// 返回上一级菜单
void Menu_Back(void) {
    if (currentMenu != NULL && currentMenu->parent != NULL) {
        currentMenu = currentMenu->parent;
        currentMenu->scrollState.isScrolling = false;
        Menu_Display(currentMenu);
    }
}

/* oled_menu.c - 使用与主菜单相同的滚动显示逻辑 */

// 通用滚动显示函数（使用与主菜单相同的滚动逻辑）
static void ShowScrollingText(const char* title, const char* content) {
    uint8_t totalLen = GetStringLength(content);
    uint8_t availableLen = MAX_DISPLAY_CHARS;  // 12个字符
    uint8_t scrollPos = 0;
    uint32_t lastScrollTime = 0;
    bool exitFlag = false;
    bool needScroll = (totalLen > availableLen);
    
    if (needScroll) {
        char displayStr[MAX_DISPLAY_CHARS + 3];
        uint8_t maxScrollPos = totalLen - availableLen;
        
        // 初始显示
        OLED_Clear();
        OLED_ShowString(0, 0, (char *)title, OLED_8X16);
        GetScrollString(content, displayStr, scrollPos, availableLen);
        OLED_ShowString(0, 0, displayStr, OLED_8X16);
        OLED_Update();
        
        lastScrollTime = HAL_GetTick();
        
        // 等待用户按返回键，同时处理滚动
        while(!exitFlag) {
            uint32_t currentTime = HAL_GetTick();
            softwd = 0;
            
            // 检测左键或OK键退出
            if (KeyInputBuff[3].KeyState != KEY_UP) {
                KeyInputBuff[3].KeyState = KEY_UP;
                exitFlag = true;
                HAL_Delay(50);
                break;
            }
            
            // 滚动更新
            if (currentTime - lastScrollTime >= SCROLL_DELAY_MS) {
                scrollPos++;
                
                // 滚动到末尾后重置
                if (scrollPos > maxScrollPos) {
                    scrollPos = 0;
                }
                
                // 全屏刷新显示
                OLED_Clear();
                OLED_ShowString(0, 0, (char*)title, OLED_8X16);
                GetScrollString(content, displayStr, scrollPos, availableLen);
                OLED_ShowString(0, 0, displayStr, OLED_8X16);
                OLED_Update();
                
                lastScrollTime = currentTime;
            }
            
            HAL_Delay(10);
        }
    } else {
        // 不需要滚动，直接显示完整内容
        char displayStr[MAX_DISPLAY_CHARS + 1];
        OLED_Clear();
        OLED_ShowString(0, 0, (char*)title, OLED_8X16);
        snprintf(displayStr, sizeof(displayStr), "%s", content);
        OLED_ShowString(0, 0, displayStr, OLED_8X16);
        OLED_Update();
        
        // 等待3秒或按键退出
        uint32_t startTime = HAL_GetTick();
        while(HAL_GetTick() - startTime < 3000) {
            softwd = 0;
            if (KeyInputBuff[3].KeyState != KEY_UP) {
                KeyInputBuff[3].KeyState = KEY_UP;
                break;
            }
            HAL_Delay(10);
        }
    }
}


// 显示账号详情
static void ShowAccountDetail(uint32_t platformIndex) {
    ShowScrollingText(">", platformData[platformIndex].account);
}

// 显示密码详情
static void ShowPasswordDetail(uint32_t platformIndex) {
    ShowScrollingText(">", platformData[platformIndex].password);
}

// 账号详情包装函数 - 不自动调用Menu_Back
static void ShowAccountDetailWrapper(void) {
    ShowAccountDetail(currentPlatformIndex);
    // 显示完成后手动返回菜单
    // Menu_Back();
}

// 密码详情包装函数 - 不自动调用Menu_Back
static void ShowPasswordDetailWrapper(void) {
    ShowPasswordDetail(currentPlatformIndex);
    // 显示完成后手动返回菜单
    // Menu_Back();
}

// 进入账号子菜单
static void EnterAccountMenu(uint32_t platformIndex) {
    static Menu accountMenu;
    static MenuItem accountItems[3];
    static bool initialized = false;
    
    if (!initialized) {
        accountItems[0].name = "Show Account";
        accountItems[0].func = ShowAccountDetailWrapper;
        accountItems[0].type = MENU_TYPE_ACCOUNT;
        accountItems[0].subMenu = NULL;
        accountItems[0].subMenuCount = 0;
        accountItems[0].dataIndex = 0;
        
        accountItems[1].name = "Show Password";
        accountItems[1].func = ShowPasswordDetailWrapper;
        accountItems[1].type = MENU_TYPE_PASSWORD;
        accountItems[1].subMenu = NULL;
        accountItems[1].subMenuCount = 0;
        accountItems[1].dataIndex = 0;
        
        accountItems[2].name = "< Back";
        accountItems[2].func = Menu_Back;
        accountItems[2].type = MENU_TYPE_MAIN;
        accountItems[2].subMenu = NULL;
        accountItems[2].subMenuCount = 0;
        accountItems[2].dataIndex = 0;
        initialized = true;
    }
    
    currentPlatformIndex = platformIndex;
    Menu_Init(&accountMenu, accountItems, 3, MENU_TYPE_ACCOUNT, currentMenu);
    currentMenu = &accountMenu;
    currentMenu->scrollState.isScrolling = false;
    Menu_Display(currentMenu);
}

// 选择平台
static void SelectPlatform(uint32_t platformIndex) {
    EnterAccountMenu(platformIndex);
}

// 平台选择包装函数
static void SelectPlatform0(void) { SelectPlatform(0); }
static void SelectPlatform1(void) { SelectPlatform(1); }
static void SelectPlatform2(void) { SelectPlatform(2); }
static void SelectPlatform3(void) { SelectPlatform(3); }
static void SelectPlatform4(void) { SelectPlatform(4); }
static void SelectPlatform5(void) { SelectPlatform(5); }
static void SelectPlatform6(void) { SelectPlatform(6); }
static void SelectPlatform7(void) { SelectPlatform(7); }

static void (*selectPlatformFuncs[])(void) = {
    SelectPlatform0, SelectPlatform1, SelectPlatform2, SelectPlatform3,
    SelectPlatform4, SelectPlatform5, SelectPlatform6, SelectPlatform7
};

// 初始化平台菜单
void PlatformMenu_Init(void) {
    uint8_t validCount = 0;
    
    // 只添加有效的平台数据
    for (int i = 0; i < platformCount && i < 8; i++) {
        if (platformData[i].isValid) {
            platformMenuItems[validCount].name = platformData[i].platform;
            platformMenuItems[validCount].func = selectPlatformFuncs[i];
            platformMenuItems[validCount].type = MENU_TYPE_PLATFORM;
            platformMenuItems[validCount].subMenu = NULL;
            platformMenuItems[validCount].subMenuCount = 0;
            platformMenuItems[validCount].dataIndex = i;
            validCount++;
        }
    }
    
    // 添加返回选项
    platformMenuItems[validCount].name = "< Back";
    platformMenuItems[validCount].func = Menu_Back;
    platformMenuItems[validCount].type = MENU_TYPE_MAIN;
    platformMenuItems[validCount].subMenu = NULL;
    platformMenuItems[validCount].subMenuCount = 0;
    platformMenuItems[validCount].dataIndex = 0;
    validCount++;
    
    // 初始化平台菜单
    static Menu platformMenu;
    platformMenuPtr = &platformMenu;
    Menu_Init(platformMenuPtr, platformMenuItems, validCount, MENU_TYPE_PLATFORM, currentMenu);
}

// 进入平台菜单
void PlatformMenu_Enter(void) {
    if (platformMenuPtr == NULL) {
        PlatformMenu_Init();
    }
    
    platformMenuPtr->parent = currentMenu;
    currentMenu = platformMenuPtr;
    currentMenu->scrollState.isScrolling = false;
    Menu_Display(currentMenu);
}

// ========== 主菜单功能函数 ==========

void Func_PlatformMenu(void) {
    PlatformMenu_Enter();
}

void Func_SetPassword(void) {
    OLED_Clear();
    OLED_ShowString(0, 0, "Set Password:", OLED_8X16);
    OLED_Update();
    HAL_Delay(2000);
    Menu_Back();
}

void Func_SystemVersion(void) {
    OLED_Clear();
    OLED_ShowString(0, 0, "Ver:V1.0", OLED_8X16);
    OLED_Update();
    HAL_Delay(2000);
    Menu_Back();
}

void Func_SerialNumber(void) {
    uint32_t serial = HAL_GetUIDw0();
    char displayStr[MAX_DISPLAY_CHARS + 1];
    
    OLED_Clear();
    snprintf(displayStr, sizeof(displayStr), "SN:%08X", serial);
    OLED_ShowString(0, 0, displayStr, OLED_8X16);
    OLED_Update();
    HAL_Delay(3000);
    Menu_Back();
}

void Func_Reboot(void) {
    OLED_Clear();
    OLED_ShowString(0, 0, "Rebooting...", OLED_8X16);
    OLED_Update();
    // HAL_Delay(1000);
    HAL_NVIC_SystemReset();
}

void Func_TransferData(void) {
    OLED_Clear();
    OLED_ShowString(0, 0, "Transfer...", OLED_8X16);
    OLED_Update();
    
    uint8_t progress = 0;
    char displayStr[MAX_DISPLAY_CHARS + 1];
    
    while(progress <= 100) {
        snprintf(displayStr, sizeof(displayStr), "%d%%", progress);
        OLED_ShowString(0, 0, displayStr, OLED_8X16);
        OLED_Update();
        HAL_Delay(100);
        progress += 10;
    }
    
    OLED_Clear();
    OLED_ShowString(0, 0, "Complete!", OLED_8X16);
    OLED_Update();
    HAL_Delay(2000);
    Menu_Back();
}