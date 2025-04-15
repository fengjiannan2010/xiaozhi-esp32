#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include "display.h"
#include "esp_timer.h"
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <font_emoji.h>
#include <lvgl.h>
#include <atomic>

class LcdDisplay : public Display {
protected:
    esp_lcd_panel_io_handle_t panel_io_ = nullptr;
    esp_lcd_panel_handle_t panel_ = nullptr;
    
    lv_draw_buf_t draw_buf_;
    lv_obj_t* status_bar_ = nullptr;
    lv_obj_t* content_ = nullptr;
    lv_obj_t* container_ = nullptr;
    lv_obj_t* side_bar_ = nullptr;

    DisplayFonts fonts_;

#if CONFIG_USE_FRAME_ANIMATION_STYLE
    int current_frame_ = 0;
    TaskHandle_t emotion_task_handle_ = nullptr;
    std::atomic<bool> emotion_task_running_{false};
#endif

    void SetupUI();
    virtual bool Lock(int timeout_ms = 0) override;
    virtual void Unlock() override;
    virtual void CreateLowBatteryPopup(lv_obj_t * parent);

#if CONFIG_USE_WECHAT_MESSAGE_STYLE
    virtual lv_coord_t CalculateBubbleWidth(const char* content);
    virtual void UpdateChatBubbleStyles();
    virtual void UpdateMessageBubbleStyle(lv_obj_t* bubble, const char* bubble_type);
    virtual void UpdateMessageTextStyle(lv_obj_t* msg_text, const char* role);
    virtual void SetTransparentContainerStyle(lv_obj_t* container);
    virtual void CreateAndAlignContainer(lv_obj_t* parent, lv_obj_t* child, const char* role);
#endif  

protected:
    // 添加protected构造函数
    LcdDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel, DisplayFonts fonts)
        : panel_io_(panel_io), panel_(panel), fonts_(fonts) {}
    
public:
    ~LcdDisplay();
    virtual void SetEmotion(const char* emotion) override;
    virtual void SetIcon(const char* icon) override;
#if CONFIG_USE_WECHAT_MESSAGE_STYLE || CONFIG_USE_FRAME_ANIMATION_STYLE
    virtual void SetChatMessage(const char* role, const char* content) override; 
#endif  

#if CONFIG_USE_FRAME_ANIMATION_STYLE
    struct EmotionAnimation {
        std::string name;
        int frameCount;
    };
    EmotionAnimation current_animation_;
    virtual void UpdateEmotionFrame();
    uint8_t* LoadRGB565Frame(const char* frame_path);
#endif  
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
