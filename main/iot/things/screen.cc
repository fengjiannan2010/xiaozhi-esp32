#include "iot/thing.h"
#include "board.h"
#include "display/lcd_display.h"
#include "settings.h"

#include <esp_log.h>
#include <string>

#define TAG "Screen"

namespace iot {

// 这里仅定义 Screen 的属性和方法，不包含具体的实现
class Screen : public Thing {
public:
    Screen() : Thing("Screen", "A screen that can set theme and brightness and style") {
        // 定义设备的属性
        properties_.AddStringProperty("theme", "Current theme", [this]() -> std::string {
            auto theme = Board::GetInstance().GetDisplay()->GetTheme();
            return theme;
        });

        // 定义设备的样式属性
        properties_.AddStringProperty("style", "Current style", [this]() -> std::string {
            auto style = Board::GetInstance().GetDisplay()->GetStyle();
            return style;
        });

        // 定义设备的亮度属性
        properties_.AddNumberProperty("brightness", "Current brightness percentage", [this]() -> int {
            // 这里可以添加获取当前亮度的逻辑
            auto backlight = Board::GetInstance().GetBacklight();
            return backlight ? backlight->brightness() : 100;
        });

        // 定义设备可以被远程执行的指令
        methods_.AddMethod("set_theme", "Set the screen theme", ParameterList({
            Parameter("theme_name", "Valid string values are 'light' and 'dark'", kValueTypeString, true)
        }), [this](const ParameterList& parameters) {
            std::string theme_name = static_cast<std::string>(parameters["theme_name"].string());
            auto display = Board::GetInstance().GetDisplay();
            if (display) {
                display->SetTheme(theme_name);
            }
        });

        methods_.AddMethod("set_style", "Set the screen style", ParameterList({
            Parameter("theme_style", "Valid string values are 'normal' and 'wechat' and 'animation'", kValueTypeString, true)
        }), [this](const ParameterList& parameters) {
            std::string theme_style = static_cast<std::string>(parameters["theme_style"].string());
            auto display = Board::GetInstance().GetDisplay();
            if (display) {
                display->SetStyle(theme_style);
            }
        });

        methods_.AddMethod("set_brightness", "Set the brightness", ParameterList({
            Parameter("brightness", "An integer between 0 and 100", kValueTypeNumber, true)
        }), [this](const ParameterList& parameters) {
            uint8_t brightness = static_cast<uint8_t>(parameters["brightness"].number());
            auto backlight = Board::GetInstance().GetBacklight();
            if (backlight) {
                backlight->SetBrightness(brightness, true);
            }
        });
    }
};

} // namespace iot

DECLARE_THING(Screen);