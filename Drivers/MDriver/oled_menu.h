/* oled_menu.h - 添加数据定义 */
#ifndef OLED_MENU_H
#define OLED_MENU_H

#include "OLED.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "stm32f1xx_hal.h"

#define MAX_DISPLAY_CHARS 12     // 96/8=12个字符(8x16字体)
#define MENU_PREFIX_LEN 2        // 菜单前缀长度("> "占用2个字符)
#define MAX_PLATFORM_NAME 32     // 平台名称最大长度
#define MAX_ACCOUNT_NAME 64      // 账号最大长度
#define SHA256_LEN 64            // SHA256长度（64字符）

// 前向声明
struct Menu;

// 菜单类型枚举
typedef enum {
    MENU_TYPE_MAIN,     // 主菜单
    MENU_TYPE_PLATFORM, // 平台菜单
    MENU_TYPE_ACCOUNT,  // 账号菜单
    MENU_TYPE_PASSWORD  // 密码菜单
} MenuType;

// 平台数据结构体（存储在FLASH中）
typedef struct {
    char platform[MAX_PLATFORM_NAME];  // 平台名称
    char account[MAX_ACCOUNT_NAME];    // 账号
    char password[SHA256_LEN + 1];     // SHA256密码（字符串形式）
    uint8_t isValid;                   // 数据是否有效
} PlatformData;

// 菜单项结构体
typedef struct {
    char* name;           // 菜单项名称
    void (*func)(void);   // 菜单项功能函数指针
    MenuType type;        // 菜单类型
    void* subMenu;        // 子菜单指针
    uint8_t subMenuCount; // 子菜单项数量
    uint32_t dataIndex;   // 数据索引
} MenuItem;

// 滚动状态结构体
typedef struct {
    bool isScrolling;     // 是否正在滚动
    uint32_t scrollTimer; // 滚动计时器
    uint8_t scrollPos;    // 滚动位置
    uint8_t currentItem;  // 当前滚动的菜单项索引
} ScrollState;

// 菜单结构体
typedef struct Menu {
    MenuItem* items;      // 菜单项数组
    uint8_t itemCount;    // 菜单项数量
    uint8_t currentIndex; // 当前选中项索引
    MenuType type;        // 菜单类型
    ScrollState scrollState; // 滚动状态
    struct Menu* parent;  // 父菜单指针
    uint32_t platformIndex; // 当前选中的平台索引
} Menu;

// 函数声明
void Menu_Init(Menu* menu, MenuItem* items, uint8_t count, MenuType type, Menu* parent);
void Menu_Up(Menu* menu);
void Menu_Down(Menu* menu);
void Menu_Enter(Menu* menu);
void Menu_Display(Menu* menu);
void Menu_UpdateScroll(Menu* menu);
void Menu_Back(void);
bool NeedScroll(const char* str);
void Menu_StartScroll(Menu* menu, uint8_t itemIndex);

// 数据菜单相关函数
void PlatformMenu_Init(void);
void PlatformMenu_Enter(void);

// 功能函数声明
void Func_PlatformMenu(void);    // 进入平台菜单
void Func_ShowAccount(void);     // 显示账号
void Func_ShowPassword(void);    // 显示密码
void Func_SetPassword(void);
void Func_SystemVersion(void);
void Func_SerialNumber(void);
void Func_Reboot(void);
void Func_TransferData(void);

// 全局菜单指针
extern Menu* currentMenu;
extern uint8_t KeyState[5];      // 按键状态数组

#endif