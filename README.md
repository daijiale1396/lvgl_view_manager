# LVGL-Lightweight-View-Manager

轻量级 LVGL 页面管理框架，专为资源受限嵌入式设备设计，核心目标是 **低内存占用 + 简单易用**，解决 LVGL 页面切换、历史导航、资源泄漏等痛点。

## 核心特点

- **极致轻量**：仅保留一个活动页面对象，历史栈仅存储页面类型信息（无冗余对象），内存开销可忽略
- **自动清理**：通过 LVGL 事件驱动自动调用 `deinit`，杜绝内存泄漏，无需手动管理页面生命周期
- **简单API**：3个核心函数完成页面跳转、返回、初始化，无需复杂配置
- **动画支持**：内置页面切换动画，可通过宏定义灵活修改动画类型和时长
- **防重复加载**：自动判断目标页面是否为当前页，避免无效切换

## 快速开始

### 1. 环境依赖

- LVGL 版本：v8+（推荐 v8.3）
- 适用场景：资源受限嵌入式设备（MCU/RTOS 环境）

### 2. 编译集成

1. 将以下文件添加到你的 LVGL 项目中：
   - `lvgl_view_manager.c`（核心实现）
   - `ui_screen_manager.h`（API 头文件）
2. 在项目中启用 LVGL 事件系统（默认启用，无需额外配置）
3. 确保编译器支持 C99 及以上标准

### 3. 运行 Demo

直接使用提供的 Demo 代码，编译后执行 `test_ui_entry()` 即可启动：

```c
// 初始化页面管理器 + 进入初始页面A
test_ui_entry();
```

- 点击页面 A（蓝色）→ 跳转到页面 B（红色）
- 点击页面 B（红色）→ 返回页面 A（蓝色）

## 使用示例

### 1. 页面定义（核心步骤）

```
// 1. 定义全局页面对象指针（可选，用于跨函数访问）
lv_obj_t* g_page_a;
lv_obj_t* g_page_b;

// 2. 声明页面创建/清理函数
static lv_obj_t* create_page_a(void);
static void deinit_page_a(lv_obj_t* page);
static lv_obj_t* create_page_b(void);
static void deinit_page_b(lv_obj_t* page);

// 3. 注册页面信息（绑定 create/deinit 函数）
const PageInfo_t g_page_a_info = {
    .create = create_page_a,
    .deinit = deinit_page_a
};
const PageInfo_t g_page_b_info = {
    .create = create_page_b,
    .deinit = deinit_page_b
};
```

### 2. 页面实现

```
// 页面A：蓝色背景，点击跳转页面B
static lv_obj_t* create_page_a(void) {
    g_page_a = lv_obj_create(NULL); // 父对象为NULL（屏幕级页面）
    lv_obj_set_style_bg_color(g_page_a, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_add_event_cb(g_page_a, on_page_a_pressed, LV_EVENT_CLICKED, NULL);
    printf("Page A Created.\n");
    return g_page_a;
}

// 页面A清理：置空全局指针（关键！避免野指针）
static void deinit_page_a(lv_obj_t* page) {
    printf("Deinit Page A.\n");
    g_page_a = NULL;
}

// 页面B：红色背景，点击返回页面A
static lv_obj_t* create_page_b(void) {
    g_page_b = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(g_page_b, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_add_event_cb(g_page_b, on_page_b_pressed, LV_EVENT_CLICKED, NULL);
    printf("Page B Created.\n");
    return g_page_b;
}

static void deinit_page_b(lv_obj_t* page) {
    printf("Deinit Page B.\n");
    g_page_b = NULL;
}
```

### 3. 事件绑定与初始化

```
// 页面A点击事件：跳转到页面B
static void on_page_a_pressed(lv_event_t* e) {
    screen_manager_switch_page(&g_page_b_info);
}

// 页面B点击事件：返回上一页
static void on_page_b_pressed(lv_event_t* e) {
    screen_manager_back();
}

// 应用入口：初始化并进入首页
void test_ui_entry(void) {
    screen_manager_init(); // 初始化页面管理器（启动时调用一次）
    screen_manager_switch_page(&g_page_a_info); // 进入初始页面A
}
```

## API 说明

| 函数名                                                    | 功能                                 | 参数                                |
| --------------------------------------------------------- | ------------------------------------ | ----------------------------------- |
| `screen_manager_init()`                                   | 初始化页面管理器                     | 无                                  |
| `screen_manager_switch_page(const PageInfo_t* page_info)` | 跳转到指定页面（销毁当前页）         | `page_info`：目标页面信息结构体指针 |
| `screen_manager_back()`                                   | 返回上一个页面（历史栈有记录时生效） | 无                                  |

## 配置自定义

在 `ui_screen_manager.h` 中修改以下宏定义，适配你的需求：

```
#define MAX_HISTORY_DEEP 5          // 最大历史记录深度（可返回次数）
#define LV_LOAD_ANIM LV_SCR_LOAD_ANIM_FADE_ON  // 切换动画类型（如滑动、淡入）
#define LV_ANIMATION_TIME 150       // 动画时长（单位：ms）
```

- 动画类型可选：`LV_SCR_LOAD_ANIM_NONE`（无动画）、`LV_SCR_LOAD_ANIM_SLIDE_LEFT`（左滑）、`LV_SCR_LOAD_ANIM_SLIDE_RIGHT`（右滑）、`LV_SCR_LOAD_ANIM_FADE_ON`（淡入）、`LV_SCR_LOAD_ANIM_ZOOM_IN`（放大）

## 注意事项

1. 页面 `create` 函数必须返回 `lv_obj_create(NULL)` 创建的屏幕级对象（父对象为 NULL），否则动画和销毁逻辑异常
2. 全局页面对象指针（如 `g_page_a`）必须在 `deinit` 中置空，避免野指针访问
3. 页面 `deinit` 函数需清理自定义资源（如 `lv_timer_t` 定时器、动态内存、子对象等）
4. 历史栈深度 `MAX_HISTORY_DEEP` 建议根据设备内存调整（默认 5，最小 2，最大不超过 10）
5. 避免在页面事件回调中直接调用 `lv_obj_del` 或 `lv_obj_del_async` 删除当前页面，框架会自动处理旧页面删除
6. 若需新增页面，按「定义指针 → 声明 create/deinit → 注册 PageInfo」三步即可快速集成

## 许可证

MIT License

Permission is hereby granted, free of charge, to any person obtaining a copyof this software and associated documentation files (the "Software"), to dealin the Software without restriction, including without limitation the rightsto use, copy, modify, merge, publish, distribute, sublicense, and/or sellcopies of the Software, and to permit persons to whom the Software isfurnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in allcopies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS ORIMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THEAUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHERLIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THESOFTWARE.

## 贡献者

- Author: daijiale1396
- 欢迎提交 Issue 反馈 Bug 或需求，也可通过 PR 参与功能优化！