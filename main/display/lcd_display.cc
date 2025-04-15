#include "lcd_display.h"

#include <vector>
#include <font_awesome_symbols.h>
#include <esp_log.h>
#include <esp_err.h>
#include <esp_lvgl_port.h>
#include "assets/lang_config.h"
#include <cstring>
#include "settings.h"
#include <unordered_map>
#include <lvgl.h>
#include "board.h"

#define TAG "LcdDisplay"

// Color definitions for dark theme
#define DARK_BACKGROUND_COLOR       lv_color_hex(0x121212)     // Dark background
#define DARK_TEXT_COLOR             lv_color_white()           // White text
#define DARK_CHAT_BACKGROUND_COLOR  lv_color_hex(0x1E1E1E)     // Slightly lighter than background
#define DARK_USER_BUBBLE_COLOR      lv_color_hex(0x1A6C37)     // Dark green
#define DARK_ASSISTANT_BUBBLE_COLOR lv_color_hex(0x333333)     // Dark gray
#define DARK_SYSTEM_BUBBLE_COLOR    lv_color_hex(0x2A2A2A)     // Medium gray
#define DARK_SYSTEM_TEXT_COLOR      lv_color_hex(0xAAAAAA)     // Light gray text
#define DARK_BORDER_COLOR           lv_color_hex(0x333333)     // Dark gray border
#define DARK_LOW_BATTERY_COLOR      lv_color_hex(0xFF0000)     // Red for dark mode

// Color definitions for light theme
#define LIGHT_BACKGROUND_COLOR       lv_color_white()           // White background
#define LIGHT_TEXT_COLOR             lv_color_black()           // Black text
#define LIGHT_CHAT_BACKGROUND_COLOR  lv_color_hex(0xE0E0E0)     // Light gray background
#define LIGHT_USER_BUBBLE_COLOR      lv_color_hex(0x95EC69)     // WeChat green
#define LIGHT_ASSISTANT_BUBBLE_COLOR lv_color_white()           // White
#define LIGHT_SYSTEM_BUBBLE_COLOR    lv_color_hex(0xE0E0E0)     // Light gray
#define LIGHT_SYSTEM_TEXT_COLOR      lv_color_hex(0x666666)     // Dark gray text
#define LIGHT_BORDER_COLOR           lv_color_hex(0xE0E0E0)     // Light gray border
#define LIGHT_LOW_BATTERY_COLOR      lv_color_black()           // Black for light mode



// Theme color structure
struct ThemeColors {
    lv_color_t background;
    lv_color_t text;
    lv_color_t chat_background;
    lv_color_t user_bubble;
    lv_color_t assistant_bubble;
    lv_color_t system_bubble;
    lv_color_t system_text;
    lv_color_t border;
    lv_color_t low_battery;
};

// Define dark theme colors
static const ThemeColors DARK_THEME = {
    .background = DARK_BACKGROUND_COLOR,
    .text = DARK_TEXT_COLOR,
    .chat_background = DARK_CHAT_BACKGROUND_COLOR,
    .user_bubble = DARK_USER_BUBBLE_COLOR,
    .assistant_bubble = DARK_ASSISTANT_BUBBLE_COLOR,
    .system_bubble = DARK_SYSTEM_BUBBLE_COLOR,
    .system_text = DARK_SYSTEM_TEXT_COLOR,
    .border = DARK_BORDER_COLOR,
    .low_battery = DARK_LOW_BATTERY_COLOR
};

// Define light theme colors
static const ThemeColors LIGHT_THEME = {
    .background = LIGHT_BACKGROUND_COLOR,
    .text = LIGHT_TEXT_COLOR,
    .chat_background = LIGHT_CHAT_BACKGROUND_COLOR,
    .user_bubble = LIGHT_USER_BUBBLE_COLOR,
    .assistant_bubble = LIGHT_ASSISTANT_BUBBLE_COLOR,
    .system_bubble = LIGHT_SYSTEM_BUBBLE_COLOR,
    .system_text = LIGHT_SYSTEM_TEXT_COLOR,
    .border = LIGHT_BORDER_COLOR,
    .low_battery = LIGHT_LOW_BATTERY_COLOR
};

// Define frame animation theme colors
static const ThemeColors FRAME_ANIMATION_THEME = {
    .background = lv_color_hex(0x0c0c1e),
    .text = lv_color_white(),
    .chat_background = lv_color_hex(0x0c0c1e)  ,
    .user_bubble = lv_color_hex(0x0c0c1e)  ,
    .assistant_bubble = lv_color_hex(0x0c0c1e)  ,
    .system_bubble = lv_color_hex(0x0c0c1e),
    .system_text = lv_color_hex(0x0c0c1e),
    .border = lv_color_hex(0x0c0c1e),
    .low_battery = lv_color_hex(0x5c1cee)  
};

// Current theme - initialize based on default config
static ThemeColors current_theme = FRAME_ANIMATION_THEME;


LV_FONT_DECLARE(font_awesome_30_4);

SpiLcdDisplay::SpiLcdDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel,
                           int width, int height, int offset_x, int offset_y, bool mirror_x, bool mirror_y, bool swap_xy,
                           DisplayFonts fonts)
    : LcdDisplay(panel_io, panel, fonts) {
    width_ = width;
    height_ = height;

    // draw white
    std::vector<uint16_t> buffer(width_, 0xFFFF);
    for (int y = 0; y < height_; y++) {
        esp_lcd_panel_draw_bitmap(panel_, 0, y, width_, y + 1, buffer.data());
    }

    // Set the display to on
    ESP_LOGI(TAG, "Turning display on");
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_, true));

    ESP_LOGI(TAG, "Initialize LVGL library");
    lv_init();

    ESP_LOGI(TAG, "Initialize LVGL port");
    lvgl_port_cfg_t port_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    port_cfg.task_priority = 1;
    port_cfg.timer_period_ms = 50;
    lvgl_port_init(&port_cfg);

    ESP_LOGI(TAG, "Adding LCD screen");
    const lvgl_port_display_cfg_t display_cfg = {
        .io_handle = panel_io_,
        .panel_handle = panel_,
        .control_handle = nullptr,
        .buffer_size = static_cast<uint32_t>(width_ * 10),
        .double_buffer = false,
        .trans_size = 0,
        .hres = static_cast<uint32_t>(width_),
        .vres = static_cast<uint32_t>(height_),
        .monochrome = false,
        .rotation = {
            .swap_xy = swap_xy,
            .mirror_x = mirror_x,
            .mirror_y = mirror_y,
        },
        .color_format = LV_COLOR_FORMAT_RGB565,
        .flags = {
            .buff_dma = 1,
            .buff_spiram = 0,
            .sw_rotate = 0,
            .swap_bytes = 1,
            .full_refresh = 0,
            .direct_mode = 0,
        },
    };

    display_ = lvgl_port_add_disp(&display_cfg);
    if (display_ == nullptr) {
        ESP_LOGE(TAG, "Failed to add display");
        return;
    }

    if (offset_x != 0 || offset_y != 0) {
        lv_display_set_offset(display_, offset_x, offset_y);
    }

    // Update the theme
    if (current_theme_name_ == "dark") {
        current_theme = DARK_THEME;
    } else if (current_theme_name_ == "light") {
        current_theme = LIGHT_THEME;
    }

    SetupUI();
}

// RGB LCD实现
RgbLcdDisplay::RgbLcdDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel,
                           int width, int height, int offset_x, int offset_y,
                           bool mirror_x, bool mirror_y, bool swap_xy,
                           DisplayFonts fonts)
    : LcdDisplay(panel_io, panel, fonts) {
    width_ = width;
    height_ = height;
    
    // draw white
    std::vector<uint16_t> buffer(width_, 0xFFFF);
    for (int y = 0; y < height_; y++) {
        esp_lcd_panel_draw_bitmap(panel_, 0, y, width_, y + 1, buffer.data());
    }

    ESP_LOGI(TAG, "Initialize LVGL library");
    lv_init();

    ESP_LOGI(TAG, "Initialize LVGL port");
    lvgl_port_cfg_t port_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    port_cfg.task_priority = 1;
    lvgl_port_init(&port_cfg);

    ESP_LOGI(TAG, "Adding LCD screen");
    const lvgl_port_display_cfg_t display_cfg = {
        .io_handle = panel_io_,
        .panel_handle = panel_,
        .buffer_size = static_cast<uint32_t>(width_ * 10),
        .double_buffer = true,
        .hres = static_cast<uint32_t>(width_),
        .vres = static_cast<uint32_t>(height_),
        .rotation = {
            .swap_xy = swap_xy,
            .mirror_x = mirror_x,
            .mirror_y = mirror_y,
        },
        .flags = {
            .buff_dma = 1,
            .swap_bytes = 0,
            .full_refresh = 1,
            .direct_mode = 1,
        },
    };

    const lvgl_port_display_rgb_cfg_t rgb_cfg = {
        .flags = {
            .bb_mode = true,
            .avoid_tearing = true,
        }
    };
    
    display_ = lvgl_port_add_disp_rgb(&display_cfg, &rgb_cfg);
    if (display_ == nullptr) {
        ESP_LOGE(TAG, "Failed to add RGB display");
        return;
    }
    
    if (offset_x != 0 || offset_y != 0) {
        lv_display_set_offset(display_, offset_x, offset_y);
    }

    // Update the theme
    if (current_theme_name_ == "dark") {
        current_theme = DARK_THEME;
    } else if (current_theme_name_ == "light") {
        current_theme = LIGHT_THEME;
    }

    SetupUI();
}

LcdDisplay::~LcdDisplay() {
    // 添加递归删除逻辑
    if (content_ != nullptr) {
        while (lv_obj_get_child_cnt(content_) > 0) {
            lv_obj_del(lv_obj_get_child(content_, 0));
        }
        lv_obj_del(content_);
    }
    if (status_bar_ != nullptr) {
        lv_obj_del(status_bar_);
    }
    if (side_bar_ != nullptr) {
        lv_obj_del(side_bar_);
    }
    if (container_ != nullptr) {
        lv_obj_del(container_);
    }
    if (display_ != nullptr) {
        lv_display_delete(display_);
    }

    if (panel_ != nullptr) {
        esp_lcd_panel_del(panel_);
    }
    if (panel_io_ != nullptr) {
        esp_lcd_panel_io_del(panel_io_);
    }

#if CONFIG_USE_FRAME_ANIMATION_STYLE
    if (emotion_task_handle_) {
        emotion_task_running_ = false;
        vTaskDelete(emotion_task_handle_);
        emotion_task_handle_ = nullptr;
    }
#endif
}

bool LcdDisplay::Lock(int timeout_ms) {
    return lvgl_port_lock(timeout_ms);
}

void LcdDisplay::Unlock() {
    lvgl_port_unlock();
}

#if CONFIG_USE_WECHAT_MESSAGE_STYLE
void LcdDisplay::SetupUI() {
    DisplayLockGuard lock(this);

    auto screen = lv_screen_active();
    lv_obj_set_style_text_font(screen, fonts_.text_font, 0);
    lv_obj_set_style_text_color(screen, current_theme.text, 0);
    lv_obj_set_style_bg_color(screen, current_theme.background, 0);

    /* Container */
    container_ = lv_obj_create(screen);
    lv_obj_set_size(container_, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(container_, 0, 0);
    lv_obj_set_style_border_width(container_, 0, 0);
    lv_obj_set_style_pad_row(container_, 0, 0);
    lv_obj_set_style_bg_color(container_, current_theme.background, 0);
    lv_obj_set_style_border_color(container_, current_theme.border, 0);

    /* Status bar */
    status_bar_ = lv_obj_create(container_);
    lv_obj_set_size(status_bar_, LV_HOR_RES, LV_SIZE_CONTENT);
    lv_obj_set_style_radius(status_bar_, 0, 0);
    lv_obj_set_style_bg_color(status_bar_, current_theme.background, 0);
    lv_obj_set_style_text_color(status_bar_, current_theme.text, 0);
    
    /* Content - Chat area */
    content_ = lv_obj_create(container_);
    lv_obj_set_style_radius(content_, 0, 0);
    lv_obj_set_width(content_, LV_HOR_RES);
    lv_obj_set_flex_grow(content_, 1);
    lv_obj_set_style_pad_all(content_, 10, 0);
    lv_obj_set_style_bg_color(content_, current_theme.chat_background, 0); // Background for chat area
    lv_obj_set_style_border_color(content_, current_theme.border, 0); // Border color for chat area

    // Enable scrolling for chat content
    lv_obj_set_scrollbar_mode(content_, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(content_, LV_DIR_VER);
    
    // Create a flex container for chat messages
    lv_obj_set_flex_flow(content_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(content_, 10, 0); // Space between messages

    // We'll create chat messages dynamically in SetChatMessage
    chat_message_label_ = nullptr;

    /* Status bar */
    lv_obj_set_flex_flow(status_bar_, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(status_bar_, 0, 0);
    lv_obj_set_style_border_width(status_bar_, 0, 0);
    lv_obj_set_style_pad_column(status_bar_, 0, 0);
    lv_obj_set_style_pad_left(status_bar_, 10, 0);
    lv_obj_set_style_pad_right(status_bar_, 10, 0);
    lv_obj_set_style_pad_top(status_bar_, 2, 0);
    lv_obj_set_style_pad_bottom(status_bar_, 2, 0);
    lv_obj_set_scrollbar_mode(status_bar_, LV_SCROLLBAR_MODE_OFF);
    // 设置状态栏的内容垂直居中
    lv_obj_set_flex_align(status_bar_, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // 创建emotion_label_在状态栏最左侧
    emotion_label_ = lv_label_create(status_bar_);
    lv_obj_set_style_text_font(emotion_label_, &font_awesome_30_4, 0);
    lv_obj_set_style_text_color(emotion_label_, current_theme.text, 0);
    lv_label_set_text(emotion_label_, FONT_AWESOME_AI_CHIP);
    lv_obj_set_style_margin_right(emotion_label_, 5, 0); // 添加右边距，与后面的元素分隔

    notification_label_ = lv_label_create(status_bar_);
    lv_obj_set_flex_grow(notification_label_, 1);
    lv_obj_set_style_text_align(notification_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(notification_label_, current_theme.text, 0);
    lv_label_set_text(notification_label_, "");
    lv_obj_add_flag(notification_label_, LV_OBJ_FLAG_HIDDEN);

    status_label_ = lv_label_create(status_bar_);
    lv_obj_set_flex_grow(status_label_, 1);
    lv_label_set_long_mode(status_label_, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_align(status_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(status_label_, current_theme.text, 0);
    lv_label_set_text(status_label_, Lang::Strings::INITIALIZING);
    
    mute_label_ = lv_label_create(status_bar_);
    lv_label_set_text(mute_label_, "");
    lv_obj_set_style_text_font(mute_label_, fonts_.icon_font, 0);
    lv_obj_set_style_text_color(mute_label_, current_theme.text, 0);

    network_label_ = lv_label_create(status_bar_);
    lv_label_set_text(network_label_, "");
    lv_obj_set_style_text_font(network_label_, fonts_.icon_font, 0);
    lv_obj_set_style_text_color(network_label_, current_theme.text, 0);
    lv_obj_set_style_margin_left(network_label_, 5, 0); // 添加左边距，与前面的元素分隔

    battery_label_ = lv_label_create(status_bar_);
    lv_label_set_text(battery_label_, "");
    lv_obj_set_style_text_font(battery_label_, fonts_.icon_font, 0);
    lv_obj_set_style_text_color(battery_label_, current_theme.text, 0);
    lv_obj_set_style_margin_left(battery_label_, 5, 0); // 添加左边距，与前面的元素分隔

    low_battery_popup_ = lv_obj_create(screen);
    lv_obj_set_scrollbar_mode(low_battery_popup_, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_size(low_battery_popup_, LV_HOR_RES * 0.9, fonts_.text_font->line_height * 2);
    lv_obj_align(low_battery_popup_, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(low_battery_popup_, current_theme.low_battery, 0);
    lv_obj_set_style_radius(low_battery_popup_, 10, 0);
    lv_obj_t* low_battery_label = lv_label_create(low_battery_popup_);
    lv_label_set_text(low_battery_label, Lang::Strings::BATTERY_NEED_CHARGE);
    lv_obj_set_style_text_color(low_battery_label, lv_color_white(), 0);
    lv_obj_center(low_battery_label);
    lv_obj_add_flag(low_battery_popup_, LV_OBJ_FLAG_HIDDEN);
    //创建电量弹窗
    CreateLowBatteryPopup(screen);

    UpdateChatBubbleStyles();
}

// 计算聊天气泡宽度
lv_coord_t LcdDisplay::CalculateBubbleWidth(const char* content) {
    if (content == nullptr) return 20; // 默认最小宽度

    lv_coord_t text_width = lv_txt_get_width(content, strlen(content), fonts_.text_font, 0);
    lv_coord_t max_width = LV_HOR_RES * 85 / 100 - 16;  // 屏幕宽度的85%
    lv_coord_t min_width = 20;

    // 确保宽度在最小和最大范围内
    return std::max(min_width, std::min(text_width, max_width));
}

void LcdDisplay::SetTransparentContainerStyle(lv_obj_t* container) {
    if (container == nullptr) return;

    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, 0, 0);
}

// 更新聊天气泡样式
void LcdDisplay::UpdateMessageBubbleStyle(lv_obj_t* msg_bubble, const char* role) {
    if (msg_bubble == nullptr || role == nullptr) return;

    // 根据气泡类型应用样式
    if (strcmp(role, "user") == 0) {
        lv_obj_set_style_bg_color(msg_bubble, current_theme.user_bubble, 0);
        lv_obj_set_user_data(msg_bubble, (void*)role);
        lv_obj_set_style_flex_grow(msg_bubble, 0, 0);
    } else if (strcmp(role, "assistant") == 0) {
        lv_obj_set_style_bg_color(msg_bubble, current_theme.assistant_bubble, 0);
        lv_obj_set_user_data(msg_bubble, (void*)role);
        lv_obj_set_style_flex_grow(msg_bubble, 0, 0);
    } else if (strcmp(role, "system") == 0) {
        lv_obj_set_style_bg_color(msg_bubble, current_theme.system_bubble, 0);
        lv_obj_set_style_text_color(msg_bubble, current_theme.system_text, 0);
        lv_obj_set_user_data(msg_bubble, (void*)role);
        lv_obj_set_style_flex_grow(msg_bubble, 0, 0);
    }

    lv_obj_set_style_border_width(msg_bubble, 1, 0);
    lv_obj_set_style_border_color(msg_bubble, current_theme.border, 0);
}

void LcdDisplay::UpdateMessageTextStyle(lv_obj_t* msg_text, const char* role) {
    if (msg_text == nullptr || role == nullptr) return;

   // Set alignment and style based on message role
    if (strcmp(role, "user") == 0) {
        // Set text color for contrast
        lv_obj_set_style_text_color(msg_text, current_theme.text, 0);
    } else if (strcmp(role, "assistant") == 0) {
        // Set text color for contrast
        lv_obj_set_style_text_color(msg_text, current_theme.text, 0);
    } else if (strcmp(role, "system") == 0) {
        // Set text color for contrast
        lv_obj_set_style_text_color(msg_text, current_theme.system_text, 0);
    }
}

void LcdDisplay::CreateAndAlignContainer(lv_obj_t* parent, lv_obj_t* child, const char* role) {
    lv_obj_t* container = lv_obj_create(parent);
    lv_obj_set_width(container, LV_HOR_RES);
    lv_obj_set_height(container, LV_SIZE_CONTENT);
    SetTransparentContainerStyle(container);
    lv_obj_set_parent(child, container);

    if (strcmp(role, "user") == 0) {
        lv_obj_align(child, LV_ALIGN_RIGHT_MID, -25, 0);
    } else if (strcmp(role, "system") == 0) {
        lv_obj_align(child, LV_ALIGN_CENTER, 0, 0);
    }
    lv_obj_scroll_to_view_recursive(container, LV_ANIM_ON);
}

#define  MAX_MESSAGES 20
void LcdDisplay::SetChatMessage(const char* role, const char* content) {
    DisplayLockGuard lock(this);
    if (content_ == nullptr) {
        ESP_LOGW(TAG, "Content area is null, cannot set chat message.");
        return;
    }
    //避免出现空的消息框
    if (strlen(content) == 0) return;
    
    // 检查消息数量是否超过限制
    uint32_t child_count = lv_obj_get_child_cnt(content_);
    if (child_count >= MAX_MESSAGES) {
        lv_obj_t* first_child = lv_obj_get_child(content_, 0);
        if (first_child != nullptr) {
            lv_obj_del(first_child); 
            ESP_LOGD(TAG, "Deleted oldest message (container:%d)", lv_obj_get_child_cnt(first_child) > 0);
        }
    }
    
    // 创建消息气泡
    lv_obj_t* msg_bubble = lv_obj_create(content_);
    lv_obj_set_style_radius(msg_bubble, 8, 0);
    lv_obj_set_scrollbar_mode(msg_bubble, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_all(msg_bubble, 8, 0);

    // 设置气泡宽度
    lv_coord_t bubble_width = CalculateBubbleWidth(content);
    lv_obj_set_width(msg_bubble, bubble_width);
    lv_obj_set_height(msg_bubble, LV_SIZE_CONTENT);
    // 更新气泡样式
    UpdateMessageBubbleStyle(msg_bubble, role);

    // 创建消息文本
    lv_obj_t* msg_text = lv_label_create(msg_bubble);
    lv_label_set_text(msg_text, content);
    lv_obj_set_width(msg_text, bubble_width);
    lv_label_set_long_mode(msg_text, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_font(msg_text, fonts_.text_font, 0);
    // 更新消息样式
    UpdateMessageTextStyle(msg_text, role);

    // 创建容器并设置对齐方式
    if (strcmp(role, "user") == 0 || strcmp(role, "system") == 0) {
        CreateAndAlignContainer(content_, msg_bubble, role);
    } else {
        lv_obj_align(msg_bubble, LV_ALIGN_LEFT_MID, 0, 0);
        lv_obj_scroll_to_view_recursive(msg_bubble, LV_ANIM_ON);
    }

    // 存储最新消息标签的引用
    chat_message_label_ = msg_text;
}

void LcdDisplay::UpdateChatBubbleStyles() {
    if (content_ == nullptr) return;

    uint32_t child_count = lv_obj_get_child_cnt(content_);
    for (uint32_t i = 0; i < child_count; i++) {
        lv_obj_t* bubble = lv_obj_get_child(content_, i);
        if (bubble == nullptr) continue;

        void* bubble_type_ptr = lv_obj_get_user_data(bubble);
        if (bubble_type_ptr != nullptr) {
            const char* bubble_type = static_cast<const char*>(bubble_type_ptr);
            UpdateMessageBubbleStyle(bubble, bubble_type);
        }
    }
}

#elif CONFIG_USE_FRAME_ANIMATION_STYLE
#define SD_DRIVE "/sdcard"
#define FPS 10  // 设置帧率为 30 帧每秒
void LcdDisplay::SetupUI() {
    // 上锁，防止显示操作冲突
    DisplayLockGuard lock(this);

    // 获取当前活动屏幕
    auto screen = lv_screen_active();
    // 设置默认字体和颜色样式
    lv_obj_set_style_text_font(screen, fonts_.text_font, 0);
    lv_obj_set_style_text_color(screen, current_theme.text, 0);
    lv_obj_set_style_bg_color(screen, current_theme.background, 0);

    // 创建容器
    container_ = lv_obj_create(screen);
    lv_obj_set_size(container_, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(container_, 0, 0);
    lv_obj_set_style_border_width(container_, 0, 0);
    lv_obj_set_style_pad_row(container_, 0, 0);
    lv_obj_set_style_bg_color(container_, current_theme.background, 0);
    lv_obj_set_style_border_color(container_, current_theme.border, 0);

    // 状态栏
    status_bar_ = lv_obj_create(container_);
    lv_obj_set_size(status_bar_, LV_HOR_RES, fonts_.text_font->line_height);
    lv_obj_set_style_radius(status_bar_, 0, 0);
    lv_obj_set_style_bg_color(status_bar_, current_theme.background, 0);
    lv_obj_set_style_text_color(status_bar_, current_theme.text, 0);
    lv_obj_set_flex_flow(status_bar_, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(status_bar_, 0, 0);
    lv_obj_set_style_border_width(status_bar_, 0, 0);
    lv_obj_set_style_pad_column(status_bar_, 0, 0);
    lv_obj_set_style_pad_left(status_bar_, 2, 0);
    lv_obj_set_style_pad_right(status_bar_, 2, 0);

    // 网络状态图标标签
    network_label_ = lv_label_create(status_bar_);
    lv_label_set_text(network_label_, "");
    lv_obj_set_style_text_font(network_label_, fonts_.icon_font, 0);
    lv_obj_set_style_text_color(network_label_, current_theme.text, 0);

    // 通知图标标签
    notification_label_ = lv_label_create(status_bar_);
    lv_obj_set_flex_grow(notification_label_, 1);
    lv_obj_set_style_text_align(notification_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(notification_label_, current_theme.text, 0);
    lv_label_set_text(notification_label_, "");
    lv_obj_add_flag(notification_label_, LV_OBJ_FLAG_HIDDEN);

    // 状态文字标签（滚动）
    status_label_ = lv_label_create(status_bar_);
    lv_obj_set_flex_grow(status_label_, 1);
    lv_label_set_long_mode(status_label_, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_align(status_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(status_label_, current_theme.text, 0);
    lv_label_set_text(status_label_, Lang::Strings::INITIALIZING);

    // 静音图标
    mute_label_ = lv_label_create(status_bar_);
    lv_label_set_text(mute_label_, "");
    lv_obj_set_style_text_font(mute_label_, fonts_.icon_font, 0);
    lv_obj_set_style_text_color(mute_label_, current_theme.text, 0);

    // 电池图标
    battery_label_ = lv_label_create(status_bar_);
    lv_label_set_text(battery_label_, "");
    lv_obj_set_style_text_font(battery_label_, fonts_.icon_font, 0);
    lv_obj_set_style_text_color(battery_label_, current_theme.text, 0);

    // 内容区域：使用 FLEX 布局
    content_ = lv_obj_create(container_);
    lv_obj_set_scrollbar_mode(content_, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(content_, 0, 0);
    lv_obj_set_width(content_, LV_HOR_RES);
    lv_obj_set_flex_grow(content_, 1);
    lv_obj_set_style_pad_all(content_, 0, 0);
    lv_obj_set_style_bg_color(content_, current_theme.chat_background, 0);
    lv_obj_set_style_border_color(content_, current_theme.border, 0); 

    //创建一个图像容器，使图片不受 flex 挤压，保持居中
    lv_obj_t* img_container = lv_obj_create(content_);
    lv_obj_set_scrollbar_mode(img_container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_size(img_container, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_pad_all(img_container, 0, 0);
    lv_obj_set_style_border_width(img_container, 0, 0);
    lv_obj_set_style_pad_column(img_container, 0, 0);
    lv_obj_set_style_pad_left(img_container, 0, 0);
    lv_obj_set_style_pad_right(img_container, 0, 0);
    lv_obj_set_style_bg_color(img_container, current_theme.background, 0);
    lv_obj_set_style_bg_opa(img_container, LV_OPA_TRANSP, 0);
    lv_obj_set_layout(img_container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_align(img_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);


    //表情图（居中显示）
    emotion_label_ = lv_img_create(img_container);

    // 聊天消息标签，单独添加到 content_ 中，并强制定位
    chat_message_label_ = lv_label_create(content_);
    lv_label_set_text(chat_message_label_, "Chat message");
    lv_obj_set_pos(chat_message_label_, 8, 180);
    lv_obj_set_width(chat_message_label_, LV_HOR_RES * 0.9);
    lv_label_set_long_mode(chat_message_label_, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_align(chat_message_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(chat_message_label_, current_theme.text, 0);
    lv_obj_move_foreground(chat_message_label_);

    // 延迟滚动字幕动画
    static lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_delay(&a, 1000);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_obj_set_style_anim(chat_message_label_, &a, LV_PART_MAIN);
    lv_obj_set_style_anim_duration(chat_message_label_, lv_anim_speed_clamped(60, 300, 60000), LV_PART_MAIN);

    // 创建低电量弹窗（保留原逻辑）
    CreateLowBatteryPopup(screen);

}

// static const std::unordered_map<std::string_view, LcdDisplay::EmotionAnimation> emotion_animations_r = {
//     {"angry_r",     {"angry_r",     12}},
//     {"awkwardness_r", {"awkwardness_r", 12}},
//     {"cute_r",      {"cute_r",      12}},
//     {"grievance_r", {"grievance_r", 12}},
//     {"guffaw_r",    {"guffaw_r",    12}},
//     {"hate_r",      {"hate_r",      12}},
//     {"love_r",      {"love_r",      12}},
//     {"naughty_r",   {"naughty_r",   12}},
//     {"query_r",     {"query_r",     12}},
//     {"sad_r",       {"sad_r",       12}},
//     {"shame_r",     {"shame_r",     12}},
//     {"sleepy_r",    {"sleepy_r",    12}},
//     {"Stun_r",      {"Stun_r",      12}},
//     {"surprise_r",  {"surprise_r",  12}}
// };

// static const std::unordered_map<std::string_view, LcdDisplay::EmotionAnimation> rider_animations = {
//     {"like",        {"like",        83}},
//     {"sad",         {"sad",         65}},
//     {"Angry",       {"Angry",       55}},
//     {"cute",        {"cute",        76}},
//     {"doubt",       {"doubt",       64}},
//     {"embarrassed", {"embarrassed", 66}},
//     {"grievance",   {"grievance",   72}},
//     {"hate",        {"hate",        81}},
//     {"laugh",       {"laugh",       51}},
//     {"shy",         {"shy",         60}},
//     {"sleep",       {"sleep",       94}},
//     {"surprised",   {"surprised",   74}},
//     {"vertigo",     {"vertigo",     59}}
// };

std::vector<uint8_t*> preloaded_frames_; // 预加载的帧缓冲区

static const std::unordered_map<std::string_view, LcdDisplay::EmotionAnimation> emotion_animations = {
    {"neutral",     {"happy",     12}},
    {"happy",       {"happy",     12}},
    {"laughing",    {"guffaw",    8}},
    {"funny",       {"funny",    12}},    
    {"sad",         {"sad",      14}},
    {"angry",       {"naughty",  14}},    
    {"crying",      {"wronged",  12}},    // crying → wronged
    {"loving",      {"love",     12}},    // loving → love
    {"embarrassed", {"awkwardness", 12}}, // embarrassed → awkwardness
    {"surprised",   {"surprise", 15}},
    {"shocked",     {"Stun",     12}},    // shocked → Stun
    {"thinking",    {"query",    12}},    // thinking → query
    {"winking",     {"eyes",     12}},    // winking → eyes
    {"cool",        {"lookaround", 12}},
    {"relaxed",     {"pray",     12}},    // relaxed → pray
    {"delicious",   {"drool",    12}},    // delicious → drool
    {"kissy",       {"kissy",    12}},
    {"confident",   {"confident", 12}},
    {"sleepy",      {"sleepy",   19}},
    {"silly",       {"naughty",  12}},    // silly → naughty
    {"confused",    {"boring",   15}},    // confused → boring

    // 新表情补充
    {"awkwardness", {"awkwardness", 11}}, 
    {"boring",      {"boring",    14}},   
    {"drool",       {"drool",     8}},
    {"eyes",        {"eyes",      15}},   
    {"guffaw",      {"guffaw",    8}},    
    {"hate",        {"hate",      10}},    
    {"lookaround",  {"lookaround", 12}},   
    {"love",         {"love",     13}},   
    {"naughty",      {"naughty",  14}},   
    {"pray",         {"pray",     8}},    
    {"query",        {"query",    7}},    
    {"seek",         {"seek",     12}},   
    {"Shakehead",    {"Shakehead", 7}},   
    {"shame",        {"shame",    11}},   
    {"Stun",         {"Stun",     8}},    
    {"surprise",     {"surprise", 15}},    
    {"wronged",      {"wronged",  14}}     
};

void LcdDisplay::SetEmotion(const char* emotion) {
    DisplayLockGuard lock(this);
    if (!emotion || !emotion_label_) return;
    // 查找动画配置
    std::string_view emotion_view(emotion);
    auto it = emotion_animations.find(emotion_view);
    if (it == emotion_animations.end()) {
        emotion_view = "neutral";
        it = emotion_animations.find(emotion_view);
    }
    current_animation_ = it->second;
    // 停止旧任务并释放资源
    if (emotion_task_handle_) {
        emotion_task_running_ = false;
        vTaskDelay(pdMS_TO_TICKS(50)); // 给任务结束的时间
        if (emotion_task_handle_) {
            vTaskDelete(emotion_task_handle_);
            emotion_task_handle_ = nullptr;
        }
    }
    // 预加载所有帧到内存
    for (auto buf : preloaded_frames_) {
        delete[] buf;
    }
    preloaded_frames_.clear();
    
    for (int i = 0; i < current_animation_.frameCount; i++) {
        char frame_path[128];
        snprintf(frame_path, sizeof(frame_path), "%s/emoji_bin/%s/%d.bin",
                SD_DRIVE, current_animation_.name.c_str(), i);
        if (uint8_t* buffer = LoadRGB565Frame(frame_path)) {
            preloaded_frames_.push_back(buffer);
        } else {
            ESP_LOGE(TAG, "预加载失败，已加载 %d/%d 帧",
                   (int)preloaded_frames_.size(), current_animation_.frameCount);
            break;
        }
    }
    // 启动动画任务
    current_frame_ = 0;
    emotion_task_running_ = !preloaded_frames_.empty();
    if (emotion_task_running_) {
        xTaskCreate([](void* arg) {
            LcdDisplay* self = static_cast<LcdDisplay*>(arg);
            self->UpdateEmotionFrame();
        }, "EmotionTask", 4096, this, 5, &emotion_task_handle_);
    }
}

void LcdDisplay::UpdateEmotionFrame() {
    ESP_LOGI(TAG, "启动动画：%s (%d帧)",
           current_animation_.name.c_str(),
           (int)preloaded_frames_.size());
    const uint32_t frame_delay = 1000 / FPS;
    
    while (emotion_task_running_ && !preloaded_frames_.empty()) {
        // 使用预加载的帧数据
        uint8_t* current_buffer = preloaded_frames_[current_frame_];
        
        // 更新LVGL图像描述符
        static lv_img_dsc_t frame_desc;
        frame_desc.header.w = 240;
        frame_desc.header.h = 180;
        frame_desc.header.cf = LV_COLOR_FORMAT_RGB565;
        frame_desc.data_size = 240 * 180 * 2;
        frame_desc.data = current_buffer;
        
        lv_img_set_src(emotion_label_, &frame_desc);
        // 推进帧序号
        current_frame_ = (current_frame_ + 1) % preloaded_frames_.size();
        
        // 精确延时（考虑lv_tick_get()计时）
        static uint32_t last_tick = 0;
        uint32_t elapsed = lv_tick_elaps(last_tick);
        if (elapsed < frame_delay) {
            vTaskDelay(pdMS_TO_TICKS(frame_delay - elapsed));
        }
        last_tick = lv_tick_get();
    }
    // 退出时清理
    emotion_task_running_ = false;
    emotion_task_handle_ = nullptr;
    vTaskDelete(nullptr);
}

uint8_t* LcdDisplay::LoadRGB565Frame(const char* frame_path) {
    FILE* file = fopen(frame_path, "rb");
    if (!file) {
        ESP_LOGE(TAG, "无法打开文件：%s (错误码: %d)", frame_path, errno);
        return nullptr;
    }
    // 验证文件大小
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (file_size != 240*180*2) {
        ESP_LOGE(TAG, "文件大小错误：%s (期望 %d 字节，实际 %d 字节)",
               frame_path, 240*180*2, file_size);
        fclose(file);
        return nullptr;
    }
    // 分配内存并加载
    uint8_t* buffer = new uint8_t[file_size];
    if (fread(buffer, 1, file_size, file) != file_size) {
        ESP_LOGE(TAG, "读取文件失败：%s", frame_path);
        fclose(file);
        delete[] buffer;
        return nullptr;
    }
    fclose(file);
    return buffer;
}

void LcdDisplay::SetChatMessage(const char* role, const char* content) {
    DisplayLockGuard lock(this);
    if (chat_message_label_ == nullptr) {
        return;
    }
    lv_label_set_text(chat_message_label_, content);
    lv_obj_move_foreground(chat_message_label_);
}

#else
void LcdDisplay::SetupUI() {
    DisplayLockGuard lock(this);

    auto screen = lv_screen_active();
    lv_obj_set_style_text_font(screen, fonts_.text_font, 0);
    lv_obj_set_style_text_color(screen, current_theme.text, 0);
    lv_obj_set_style_bg_color(screen, current_theme.background, 0);

    /* Container */
    container_ = lv_obj_create(screen);
    lv_obj_set_size(container_, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(container_, 0, 0);
    lv_obj_set_style_border_width(container_, 0, 0);
    lv_obj_set_style_pad_row(container_, 0, 0);
    lv_obj_set_style_bg_color(container_, current_theme.background, 0);
    lv_obj_set_style_border_color(container_, current_theme.border, 0);

    /* Status bar */
    status_bar_ = lv_obj_create(container_);
    lv_obj_set_size(status_bar_, LV_HOR_RES, fonts_.text_font->line_height);
    lv_obj_set_style_radius(status_bar_, 0, 0);
    lv_obj_set_style_bg_color(status_bar_, current_theme.background, 0);
    lv_obj_set_style_text_color(status_bar_, current_theme.text, 0);
    
    /* Content */
    content_ = lv_obj_create(container_);
    lv_obj_set_scrollbar_mode(content_, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(content_, 0, 0);
    lv_obj_set_width(content_, LV_HOR_RES);
    lv_obj_set_flex_grow(content_, 1);
    lv_obj_set_style_pad_all(content_, 5, 0);
    lv_obj_set_style_bg_color(content_, current_theme.chat_background, 0);
    lv_obj_set_style_border_color(content_, current_theme.border, 0); // Border color for content

    lv_obj_set_flex_flow(content_, LV_FLEX_FLOW_COLUMN); // 垂直布局（从上到下）
    lv_obj_set_flex_align(content_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_EVENLY); // 子对象居中对齐，等距分布

    emotion_label_ = lv_label_create(content_);
    lv_obj_set_style_text_font(emotion_label_, &font_awesome_30_4, 0);
    lv_obj_set_style_text_color(emotion_label_, current_theme.text, 0);
    lv_label_set_text(emotion_label_, FONT_AWESOME_AI_CHIP);

    chat_message_label_ = lv_label_create(content_);
    lv_label_set_text(chat_message_label_, "");
    lv_obj_set_width(chat_message_label_, LV_HOR_RES * 0.9); // 限制宽度为屏幕宽度的 90%
    lv_label_set_long_mode(chat_message_label_, LV_LABEL_LONG_WRAP); // 设置为自动换行模式
    lv_obj_set_style_text_align(chat_message_label_, LV_TEXT_ALIGN_CENTER, 0); // 设置文本居中对齐
    lv_obj_set_style_text_color(chat_message_label_, current_theme.text, 0);

    /* Status bar */
    lv_obj_set_flex_flow(status_bar_, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(status_bar_, 0, 0);
    lv_obj_set_style_border_width(status_bar_, 0, 0);
    lv_obj_set_style_pad_column(status_bar_, 0, 0);
    lv_obj_set_style_pad_left(status_bar_, 2, 0);
    lv_obj_set_style_pad_right(status_bar_, 2, 0);

    network_label_ = lv_label_create(status_bar_);
    lv_label_set_text(network_label_, "");
    lv_obj_set_style_text_font(network_label_, fonts_.icon_font, 0);
    lv_obj_set_style_text_color(network_label_, current_theme.text, 0);

    notification_label_ = lv_label_create(status_bar_);
    lv_obj_set_flex_grow(notification_label_, 1);
    lv_obj_set_style_text_align(notification_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(notification_label_, current_theme.text, 0);
    lv_label_set_text(notification_label_, "");
    lv_obj_add_flag(notification_label_, LV_OBJ_FLAG_HIDDEN);

    status_label_ = lv_label_create(status_bar_);
    lv_obj_set_flex_grow(status_label_, 1);
    lv_label_set_long_mode(status_label_, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_align(status_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(status_label_, current_theme.text, 0);
    lv_label_set_text(status_label_, Lang::Strings::INITIALIZING);
    mute_label_ = lv_label_create(status_bar_);
    lv_label_set_text(mute_label_, "");
    lv_obj_set_style_text_font(mute_label_, fonts_.icon_font, 0);
    lv_obj_set_style_text_color(mute_label_, current_theme.text, 0);

    battery_label_ = lv_label_create(status_bar_);
    lv_label_set_text(battery_label_, "");
    lv_obj_set_style_text_font(battery_label_, fonts_.icon_font, 0);
    lv_obj_set_style_text_color(battery_label_, current_theme.text, 0);
    //创建电量弹窗
    CreateLowBatteryPopup(screen);
}
#endif

#if !CONFIG_USE_FRAME_ANIMATION_STYLE
    
static const std::unordered_map<std::string_view, std::string_view> emotion_map = {
    {"neutral",   "😶"},
    {"happy",     "🙂"},
    {"laughing",  "😆"},
    {"funny",     "😂"},
    {"sad",       "😔"},
    {"angry",     "😠"},
    {"crying",    "😭"},
    {"loving",    "😍"},
    {"embarrassed", "😳"},
    {"surprised", "😯"},
    {"shocked",   "😱"},
    {"thinking",  "🤔"},
    {"winking",   "😉"},
    {"cool",      "😎"},
    {"relaxed",   "😌"},
    {"delicious", "🤤"},
    {"kissy",     "😘"},
    {"confident", "😏"},
    {"sleepy",    "😴"},
    {"silly",     "😜"},
    {"confused",  "🙄"}
};

void LcdDisplay::SetEmotion(const char* emotion) {
    DisplayLockGuard lock(this);
    if (emotion_label_ == nullptr) {
        return;
    }

    if (emotion == nullptr) {
        lv_label_set_text(emotion_label_, "😶");
        return;
    }
    
    // 使用 std::string_view 作为查找键
    std::string_view emotion_view(emotion);
    lv_obj_set_style_text_font(emotion_label_, fonts_.emoji_font, 0);
    auto it = emotion_map.find(emotion_view); // 查找 std::string_view 类型的键
    if (it != emotion_map.end()) {
        lv_label_set_text(emotion_label_, it->second.data()); // 使用 data() 获取字符串
    } else {
        lv_label_set_text(emotion_label_, "😶");
    }
}
#endif


void LcdDisplay::SetIcon(const char* icon) {
    DisplayLockGuard lock(this);
    if (emotion_label_ == nullptr) {
        return;
    }
    lv_obj_set_style_text_font(emotion_label_, &font_awesome_30_4, 0);
    lv_label_set_text(emotion_label_, icon);
}

void LcdDisplay::SetTheme(const std::string& theme_name) {
    DisplayLockGuard lock(this);
    
    if (theme_name == "dark" || theme_name == "DARK") {
        current_theme = DARK_THEME;
    } else if (theme_name == "light" || theme_name == "LIGHT") {
        current_theme = LIGHT_THEME;
    }  else if (theme_name == "animation" || theme_name == "ANIMATION") {
        current_theme = FRAME_ANIMATION_THEME;
    } else {
        // Invalid theme name, return false
        ESP_LOGE(TAG, "Invalid theme name: %s", theme_name.c_str());
        return;
    }
    
    // Get the active screen
    lv_obj_t* screen = lv_screen_active();
    
    // Update the screen colors
    lv_obj_set_style_bg_color(screen, current_theme.background, 0);
    lv_obj_set_style_text_color(screen, current_theme.text, 0);
    
    // Update container colors
    if (container_ != nullptr) {
        lv_obj_set_style_bg_color(container_, current_theme.background, 0);
        lv_obj_set_style_border_color(container_, current_theme.border, 0);
    }
    
    // Update status bar colors
    if (status_bar_ != nullptr) {
        lv_obj_set_style_bg_color(status_bar_, current_theme.background, 0);
        lv_obj_set_style_text_color(status_bar_, current_theme.text, 0);
        
        // Update status bar elements
        if (network_label_ != nullptr) {
            lv_obj_set_style_text_color(network_label_, current_theme.text, 0);
        }
        if (status_label_ != nullptr) {
            lv_obj_set_style_text_color(status_label_, current_theme.text, 0);
        }
        if (notification_label_ != nullptr) {
            lv_obj_set_style_text_color(notification_label_, current_theme.text, 0);
        }
        if (mute_label_ != nullptr) {
            lv_obj_set_style_text_color(mute_label_, current_theme.text, 0);
        }
        if (battery_label_ != nullptr) {
            lv_obj_set_style_text_color(battery_label_, current_theme.text, 0);
        }
        if (emotion_label_ != nullptr) {
            lv_obj_set_style_text_color(emotion_label_, current_theme.text, 0);
        }
    }
    
    // Update content area colors
    if (content_ != nullptr) {
        lv_obj_set_style_bg_color(content_, current_theme.chat_background, 0);
        lv_obj_set_style_border_color(content_, current_theme.border, 0);
        
        // If we have the chat message style, update all message bubbles
#if CONFIG_USE_WECHAT_MESSAGE_STYLE
        // Iterate through all children of content (message containers or bubbles)
        uint32_t child_count = lv_obj_get_child_cnt(content_);
        for (uint32_t i = 0; i < child_count; i++) {
            lv_obj_t* obj = lv_obj_get_child(content_, i);
            if (obj == nullptr) continue;
            
            lv_obj_t* bubble = nullptr;
            
            // 检查这个对象是容器还是气泡
            // 如果是容器（用户或系统消息），则获取其子对象作为气泡
            // 如果是气泡（助手消息），则直接使用
            if (lv_obj_get_child_cnt(obj) > 0) {
                // 可能是容器，检查它是否为用户或系统消息容器
                // 用户和系统消息容器是透明的
                lv_opa_t bg_opa = lv_obj_get_style_bg_opa(obj, 0);
                if (bg_opa == LV_OPA_TRANSP) {
                    // 这是用户或系统消息的容器
                    bubble = lv_obj_get_child(obj, 0);
                } else {
                    // 这可能是助手消息的气泡自身
                    bubble = obj;
                }
            } else {
                // 没有子元素，可能是其他UI元素，跳过
                continue;
            }
            
            if (bubble == nullptr) continue;
            
            // 使用保存的用户数据来识别气泡类型
            void* bubble_type_ptr = lv_obj_get_user_data(bubble);
            if (bubble_type_ptr != nullptr) {
                const char* bubble_type = static_cast<const char*>(bubble_type_ptr);
                
                // 根据气泡类型应用正确的颜色
                if (strcmp(bubble_type, "user") == 0) {
                    lv_obj_set_style_bg_color(bubble, current_theme.user_bubble, 0);
                } else if (strcmp(bubble_type, "assistant") == 0) {
                    lv_obj_set_style_bg_color(bubble, current_theme.assistant_bubble, 0); 
                } else if (strcmp(bubble_type, "system") == 0) {
                    lv_obj_set_style_bg_color(bubble, current_theme.system_bubble, 0);
                }
                
                // Update border color
                lv_obj_set_style_border_color(bubble, current_theme.border, 0);
                
                // Update text color for the message
                if (lv_obj_get_child_cnt(bubble) > 0) {
                    lv_obj_t* text = lv_obj_get_child(bubble, 0);
                    if (text != nullptr) {
                        // 根据气泡类型设置文本颜色
                        if (strcmp(bubble_type, "system") == 0) {
                            lv_obj_set_style_text_color(text, current_theme.system_text, 0);
                        } else {
                            lv_obj_set_style_text_color(text, current_theme.text, 0);
                        }
                    }
                }
            } else {
                // 如果没有标记，回退到之前的逻辑（颜色比较）
                // ...保留原有的回退逻辑...
                lv_color_t bg_color = lv_obj_get_style_bg_color(bubble, 0);
            
                // 改进bubble类型检测逻辑，不仅使用颜色比较
                bool is_user_bubble = false;
                bool is_assistant_bubble = false;
                bool is_system_bubble = false;
            
                // 检查用户bubble
                if (lv_color_eq(bg_color, DARK_USER_BUBBLE_COLOR) || 
                    lv_color_eq(bg_color, LIGHT_USER_BUBBLE_COLOR) ||
                    lv_color_eq(bg_color, current_theme.user_bubble)) {
                    is_user_bubble = true;
                }
                // 检查系统bubble
                else if (lv_color_eq(bg_color, DARK_SYSTEM_BUBBLE_COLOR) || 
                         lv_color_eq(bg_color, LIGHT_SYSTEM_BUBBLE_COLOR) ||
                         lv_color_eq(bg_color, current_theme.system_bubble)) {
                    is_system_bubble = true;
                }
                // 剩余的都当作助手bubble处理
                else {
                    is_assistant_bubble = true;
                }
            
                // 根据bubble类型应用正确的颜色
                if (is_user_bubble) {
                    lv_obj_set_style_bg_color(bubble, current_theme.user_bubble, 0);
                } else if (is_assistant_bubble) {
                    lv_obj_set_style_bg_color(bubble, current_theme.assistant_bubble, 0);
                } else if (is_system_bubble) {
                    lv_obj_set_style_bg_color(bubble, current_theme.system_bubble, 0);
                }
                
                // Update border color
                lv_obj_set_style_border_color(bubble, current_theme.border, 0);
                
                // Update text color for the message
                if (lv_obj_get_child_cnt(bubble) > 0) {
                    lv_obj_t* text = lv_obj_get_child(bubble, 0);
                    if (text != nullptr) {
                        // 回退到颜色检测逻辑
                        if (lv_color_eq(bg_color, current_theme.system_bubble) ||
                            lv_color_eq(bg_color, DARK_SYSTEM_BUBBLE_COLOR) || 
                            lv_color_eq(bg_color, LIGHT_SYSTEM_BUBBLE_COLOR)) {
                            lv_obj_set_style_text_color(text, current_theme.system_text, 0);
                        } else {
                            lv_obj_set_style_text_color(text, current_theme.text, 0);
                        }
                    }
                }
            }
        }
#else
        // Simple UI mode - just update the main chat message
        if (chat_message_label_ != nullptr) {
            lv_obj_set_style_text_color(chat_message_label_, current_theme.text, 0);
        }
        
        // if (emotion_label_ != nullptr) {
        //     lv_obj_set_style_text_color(emotion_label_, current_theme.text, 0);
        // }
#endif
    }
    
    // Update low battery popup
    if (low_battery_popup_ != nullptr) {
        lv_obj_set_style_bg_color(low_battery_popup_, current_theme.low_battery, 0);
    }

    // No errors occurred. Save theme to settings
    Display::SetTheme(theme_name);
}
//创建电量弹窗
void LcdDisplay::CreateLowBatteryPopup(lv_obj_t * parent) {
    low_battery_popup_ = lv_obj_create(parent);
    lv_obj_set_scrollbar_mode(low_battery_popup_, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_size(low_battery_popup_, LV_HOR_RES * 0.9, fonts_.text_font->line_height * 2);
    lv_obj_align(low_battery_popup_, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(low_battery_popup_, current_theme.low_battery, 0);
    lv_obj_set_style_radius(low_battery_popup_, 10, 0);
    lv_obj_t* low_battery_label = lv_label_create(low_battery_popup_);
    lv_label_set_text(low_battery_label, Lang::Strings::BATTERY_NEED_CHARGE);
    lv_obj_set_style_text_color(low_battery_label, lv_color_white(), 0);
    lv_obj_center(low_battery_label);
    lv_obj_add_flag(low_battery_popup_, LV_OBJ_FLAG_HIDDEN);
}