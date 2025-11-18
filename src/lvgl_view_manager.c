/**
 * @file lvgl_view_manager.c
 * @author daijiale1396
 * @brief LVGL View Manager —— 面向资源受限设备的极致轻量级页面管理框架。
 * @version 2.0
 * @date 2025-11-13
 *
 * @copyright Copyright (c) 2025
 *
 * ===================================================================================
 * Design Philosophy:
 * ===================================================================================
 * 1. **内存至上 (Memory First)**
 *    - 任何时刻仅保留一个活动页面对象 (`lv_obj_t*`)。
 *    - 页面切换始终采用“销毁旧页面 → 创建新页面”的策略，杜绝内存堆积。
 *
 * 2. **轻量历史栈 (Lightweight History)**
 *    - 仅保存页面的类型信息 (`PageInfo_t*`)，而非完整对象实例。
 *    - 在支持返回功能的同时，内存开销几乎为零。
 *
 * 3. **事件驱动清理 (Event-driven Cleanup)**
 *    - 页面销毁逻辑通过 LVGL 的 `LV_EVENT_DELETE` 自动触发。
 *    - 清理函数在正确的生命周期点调用，避免时序错误和野指针风险。
 *
 * 4. **极简 API (Minimal API)**
 *    - `screen_manager_switch_page()`：跳转页面  
 *    - `screen_manager_back()`：返回上一页  
 *    - 无需理解复杂的页面堆栈与状态管理。
 * ===================================================================================
 *
 */

#include "lvgl_view_manager.h" // 请确保头文件名正确

// 模块级静态变量
static const PageInfo_t* g_current_page_info = NULL;  // 当前活动页面信息
static const PageInfo_t* g_history_stack[MAX_HISTORY_DEEP];  // 页面历史栈（仅存类型信息）
static uint8_t g_history_top = 0;  // 历史栈栈顶指针

// 内部辅助函数
/**
 * @brief 页面删除事件回调
 * @param e 事件对象，用户数据为关联的PageInfo_t
 * 触发时机：页面对象被LVGL删除时，自动调用页面deinit清理函数
 */
static void generic_delete_event_cb(lv_event_t* e) 
{
    const PageInfo_t* page_info = (const PageInfo_t*)lv_event_get_user_data(e);
    if (page_info && page_info->deinit) {
        page_info->deinit(lv_event_get_target(e));
    }
}

/**
 * @brief 页面切换核心函数
 * @param page_info 目标页面信息结构体
 * @param is_back 是否为返回操作（true时不添加历史记录）
 */
static void do_switch_page(const PageInfo_t* page_info, bool is_back) 
{
    if (page_info == NULL || page_info->create == NULL) return;
    if (!is_back && page_info == g_current_page_info) return;  // 避免重复加载

    lv_obj_t* new_page = page_info->create();
    if (new_page == NULL) return;

    lv_obj_add_event_cb(new_page, generic_delete_event_cb, LV_EVENT_DELETE, (void*)page_info);
    lv_scr_load_anim(new_page, LV_LOAD_ANIM, LV_ANIMATION_TIME, 0, true);  // 加载新页+自动删旧页

    g_current_page_info = page_info;

    // 非返回操作则更新历史栈
    if (!is_back) 
    {
        if (g_history_top < MAX_HISTORY_DEEP) 
        {
            g_history_stack[g_history_top++] = page_info;
        } 
        else 
        {
            // 栈满时丢弃最旧记录，新记录入栈
            for (int i = 0; i < MAX_HISTORY_DEEP - 1; i++) 
            {
                g_history_stack[i] = g_history_stack[i + 1];
            }
            g_history_stack[MAX_HISTORY_DEEP - 1] = page_info;
        }
    }
}

// 公共API函数
/**
 * @brief 初始化页面管理器
 * 调用时机：应用启动时执行一次，重置历史记录
 */
void screen_manager_init(void) 
{
    g_current_page_info = NULL;
    g_history_top = 0;
}

/**
 * @brief 跳转到指定页面
 * @param page_info 目标页面信息结构体
 * 功能：销毁当前页→创建新页→记录历史
 */
void screen_manager_switch_page(const PageInfo_t* page_info) 
{
    do_switch_page(page_info, false);
}

/**
 * @brief 返回上一个页面
 * 功能：历史栈有记录时，销毁当前页→重建上一页
 */
void screen_manager_back(void) 
{
    if (g_history_top > 1)
    {   g_history_top--;
        const PageInfo_t* previous_page_info = g_history_stack[g_history_top - 1];
        do_switch_page(previous_page_info, true);
    }
}
