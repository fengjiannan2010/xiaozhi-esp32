#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include "display.h"
#include "esp_timer.h"
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <font_emoji.h>
#include <lvgl.h>
#include <atomic>

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


class LcdDisplay : public Display {
protected:
    esp_lcd_panel_io_handle_t panel_io_ = nullptr;
    esp_lcd_panel_handle_t panel_ = nullptr;
    
    lv_draw_buf_t draw_buf_;
    lv_obj_t* status_bar_ = nullptr;
    lv_obj_t* content_ = nullptr;
    lv_obj_t* container_ = nullptr;
    lv_obj_t* side_bar_ = nullptr;
    lv_obj_t* preview_image_ = nullptr;

    DisplayFonts fonts_;
    ThemeColors current_theme_;

    int current_frame_ = 0;
    EventGroupHandle_t emotion_event_group_;
    TaskHandle_t emotion_task_handle_ = nullptr;
    std::atomic<bool> emotion_task_running_{false};

    void SetupUI();
    void SetupNormalUI();
    void SetupWeChatUI();
    void SetupAnimationUI();
    virtual bool Lock(int timeout_ms = 0) override;
    virtual void Unlock() override;
    void CreateLowBatteryPopup(lv_obj_t * parent);
    lv_coord_t CalculateBubbleWidth(const char* content);
    void UpdateChatBubbleStyles();
    void UpdateMessageBubbleStyle(lv_obj_t* bubble, const char* bubble_type);
    void UpdateMessageTextStyle(lv_obj_t* msg_text, const char* role);
    void SetTransparentContainerStyle(lv_obj_t* container);
    void CreateAndAlignContainer(lv_obj_t* parent, lv_obj_t* child, const char* role);
    void SetWeChatMessage(const char* role, const char* content);  
    void SetAnimationChatMessage(const char* role, const char* content);  

    void SetNormalEmotion(const char* emotion);
    void SetAnimationEmotion(const char* emotion);
protected:
    // 添加protected构造函数
    LcdDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel, DisplayFonts fonts, int width, int height);
    
public:
    ~LcdDisplay();
    virtual void SetEmotion(const char* emotion) override;
    virtual void SetIcon(const char* icon) override;
    virtual void SetChatMessage(const char* role, const char* content) override;  
    
    struct EmotionAnimation {
        std::string name;
        int frameCount;
    };
    EmotionAnimation current_animation_;
    virtual void UpdateEmotionFrame();
    uint8_t* LoadRGB565Frame(const char* frame_path);
    virtual void SetPreviewImage(const lv_img_dsc_t* img_dsc) override;
    // Add theme switching function
    virtual void SetTheme(const std::string& theme_name) override; 
};

// RGB LCD显示器
class RgbLcdDisplay : public LcdDisplay {
public:
    RgbLcdDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel,
                  int width, int height, int offset_x, int offset_y,
                  bool mirror_x, bool mirror_y, bool swap_xy,
                  DisplayFonts fonts);
};

// MIPI LCD显示器
class MipiLcdDisplay : public LcdDisplay {
public:
    MipiLcdDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel,
                   int width, int height, int offset_x, int offset_y,
                   bool mirror_x, bool mirror_y, bool swap_xy,
                   DisplayFonts fonts);
};

// SPI LCD显示器
class SpiLcdDisplay : public LcdDisplay {
public:
    SpiLcdDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel,
                  int width, int height, int offset_x, int offset_y,
                  bool mirror_x, bool mirror_y, bool swap_xy,
                  DisplayFonts fonts);
};

// QSPI LCD显示器
class QspiLcdDisplay : public LcdDisplay {
public:
    QspiLcdDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel,
                   int width, int height, int offset_x, int offset_y,
                   bool mirror_x, bool mirror_y, bool swap_xy,
                   DisplayFonts fonts);
};

// MCU8080 LCD显示器
class Mcu8080LcdDisplay : public LcdDisplay {
public:
    Mcu8080LcdDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel,
                      int width, int height, int offset_x, int offset_y,
                      bool mirror_x, bool mirror_y, bool swap_xy,
                      DisplayFonts fonts);
};
#endif // LCD_DISPLAY_H
