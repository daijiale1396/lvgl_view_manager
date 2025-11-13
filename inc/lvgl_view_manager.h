/**
 * @file lvgl_view_manager.h
 * @author daijiale1396
 * @brief LVGL View Manager —— 为资源受限设备设计的极致轻量级页面管理框架（API 定义）
 * @version 2.0
 * @date 2025-11-13
 *
 * @copyright Copyright (c) 2025
 *
 * ===================================================================================
 * Design Philosophy:
 * ===================================================================================
 *
 * **核心理念：仅保留一个活动页面对象**
 *
 * 页面切换始终遵循 “销毁当前 → 创建目标” 的顺序。
 * 不保留冗余屏幕指针，从根本上杜绝了堆积与泄漏风险。
 *
 * 同时以轻量化历史栈保存页面类型信息（非对象实例），
 * 在保证“返回”功能的前提下，将内存开销降至最低。
 *
 * ===================================================================================
 */

#ifndef _LVGL_VIEW_MANAGER_H
#define _LVGL_VIEW_MANAGER_H

#include "lvgl/lvgl.h"  // 确保项目中 LVGL 头文件路径正确

// 公共配置宏
#define MAX_HISTORY_DEEP 5          // 历史记录最大深度（可调整）
#define LV_LOAD_ANIM LV_SCR_LOAD_ANIM_FADE_ON  // 页面切换默认动画
#define LV_ANIMATION_TIME 150       // 动画持续时间（ms）

// 公共类型定义
/**
 * @brief 页面信息结构体（创建/销毁函数指针集合）
 */
typedef struct {
    lv_obj_t* (*create)(void);  // 创建页面，返回页面对象指针
    void (*deinit)(lv_obj_t* page);  // 清理页面资源，入参为待销毁页面对象
} PageInfo_t;

// 公共API函数声明
void screen_manager_init(void);  // 初始化页面管理器（启动时调用）
void screen_manager_switch_page(const PageInfo_t* page_info);  // 跳转指定页面
void screen_manager_back(void);  // 返回上一个页面

#endif /* _UI_SCREEN_MANAGER_H */
